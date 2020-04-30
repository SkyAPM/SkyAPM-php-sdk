package service

import (
	"context"
	"encoding/json"
	nc2 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/common/v2"
	nc3 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/common/v3"
	nla1 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v1"
	nm3 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/management/v3"
	nr2 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/register/v2"
	"github.com/google/uuid"
	"net"
	"os"
	"runtime"
	"strconv"
	"time"
)

type registerCache struct {
	Version         int
	AppId           int32
	InstanceId      int32
	Uuid            string
	Service         string
	ServiceInstance string
}

type registerReq struct {
	AppCode string `json:"app_code"`
	Pid     int    `json:"pid"`
	Version int    `json:"version"`
}

func ip4s() []string {
	ipv4s, addErr := net.InterfaceAddrs()
	var ips []string
	if addErr == nil {
		for _, i := range ipv4s {
			if ipnet, ok := i.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
				if ipnet.IP.To4() != nil {
					ips = append(ips, ipnet.IP.String())
				}
			}
		}
	}
	return ips
}

func (t *Agent) recoverRegister(info trace) {

	t.registerCacheLock.RLock()
	_, ok := t.registerCache[info.Pid]
	t.registerCacheLock.RUnlock()

	if !ok {
		t.registerCacheLock.Lock()
		t.registerCache[info.Pid] = registerCache{
			Version:    info.Version,
			AppId:      info.ApplicationId,
			InstanceId: info.ApplicationInstance,
			Uuid:       info.Uuid,
		}
		t.registerCacheLock.Unlock()
	}
}

func (t *Agent) doRegister(r *register) {

	info := registerReq{}
	err := json.Unmarshal([]byte(r.body), &info)
	if err != nil {
		log.Error("register json decode error", err)
		r.c.Write([]byte(""))
		return
	}

	pid := info.Pid
	t.registerCacheLock.RLock()
	bind, ok := t.registerCache[pid]
	t.registerCacheLock.RUnlock()
	if ok {
		if t.version == 5 || t.version == 6 || t.version == 7 {
			log.Infof("register cache => pid %d, service %s, appId %d insId %d", pid, info.AppCode, bind.AppId, bind.InstanceId)
			r.c.Write([]byte(strconv.FormatInt(int64(bind.AppId), 10) + "," + strconv.FormatInt(int64(bind.InstanceId), 10) + "," + bind.Uuid))
		} else if t.version == 8 {
			log.Infof("register cache => pid %d, service %s, service instance %s", pid, bind.Service, bind.ServiceInstance)
			r.c.Write([]byte(bind.Service + "," + bind.ServiceInstance + "," + bind.Uuid))
		}
		return
	} else {
		r.c.Write([]byte(""))
	}

	t.registerCacheLock.Lock()
	defer t.registerCacheLock.Unlock()
	// if map not found pid.. start register
	if _, ok := t.registerCache[pid]; !ok {
		log.Infof("start register pid %d used SkyWalking v%d", pid, info.Version)
		var regAppStatus = false
		var appId int32 = 0
		var appInsId int32 = 0
		var regErr error
		agentUUID := uuid.New().String()

		if t.version == 5 {
			c := nla1.NewApplicationRegisterServiceClient(t.grpcConn)
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			var regResp *nla1.ApplicationMapping

			// loop register
			for {
				regResp, regErr = c.ApplicationCodeRegister(ctx, &nla1.Application{
					ApplicationCode: info.AppCode,
				})
				if regErr != nil {
					log.Error("register error:", regErr)
					break
				}
				if regResp.GetApplication() != nil {
					regAppStatus = true
					appId = regResp.GetApplication().GetValue()
					break
				}
				time.Sleep(time.Second)
			}
		} else if t.version == 6 || t.version == 7 {
			c := nr2.NewRegisterClient(t.grpcConn)
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			var regResp *nr2.ServiceRegisterMapping
			var services []*nr2.Service
			services = append(services, &nr2.Service{
				ServiceName: info.AppCode,
			})
			// loop register
			for {
				regResp, regErr = c.DoServiceRegister(ctx, &nr2.Services{
					Services: services,
				})
				if regErr != nil {
					log.Error("register error:", regErr)
					break
				}

				if regResp.GetServices() != nil {
					for _, v := range regResp.GetServices() {
						if v.GetKey() == info.AppCode {
							regAppStatus = true
							appId = v.GetValue()
							break
						}
					}
				}

				if regAppStatus {
					break
				}
				time.Sleep(time.Second)
			}
		} else if t.version == 8 {
			c := nm3.NewManagementServiceClient(t.grpcConn)
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			req := &nm3.InstanceProperties{
				Service:         info.AppCode,
				ServiceInstance: uuid.New().String(),
			}

			req.Properties = append(req.Properties, &nc3.KeyStringValuePair{
				Key:   "os_name",
				Value: runtime.GOOS,
			})

			hostName, _ := os.Hostname()
			req.Properties = append(req.Properties, &nc3.KeyStringValuePair{
				Key:   "host_name",
				Value: hostName,
			})

			req.Properties = append(req.Properties, &nc3.KeyStringValuePair{
				Key:   "process_no",
				Value: strconv.Itoa(pid),
			})

			req.Properties = append(req.Properties, &nc3.KeyStringValuePair{
				Key:   "language",
				Value: "php",
			})

			for _, ip := range ip4s() {
				req.Properties = append(req.Properties, &nc3.KeyStringValuePair{
					Key:   "ipv4",
					Value: ip,
				})
			}

			_, err := c.ReportInstanceProperties(ctx, req)

			if err != nil {
				log.Error("report instance properties:", err)
			} else {
				t.registerCache[pid] = registerCache{
					Version:         t.version,
					AppId:           1,
					InstanceId:      1,
					Uuid:            req.ServiceInstance,
					Service:         info.AppCode,
					ServiceInstance: req.ServiceInstance,
				}
				log.Infof("registered pid %d, service %s service instance %s", pid, info.AppCode, req.ServiceInstance)
			}
			return
		}

		if regAppStatus {
			// start reg instance
			if t.version == 5 {
				instanceClient := nla1.NewInstanceDiscoveryServiceClient(t.grpcConn)
				instanceCtx, instanceCancel := context.WithTimeout(context.Background(), time.Second*3)
				defer instanceCancel()

				var instanceErr error
				var instanceResp *nla1.ApplicationInstanceMapping
				hostName, _ := os.Hostname()

				instanceReq := &nla1.ApplicationInstance{
					ApplicationId: appId,
					AgentUUID:     agentUUID,
					RegisterTime:  time.Now().UnixNano() / 1000000,
					Osinfo: &nla1.OSInfo{
						OsName:    runtime.GOOS,
						Hostname:  hostName,
						ProcessNo: int32(pid),
						Ipv4S:     ip4s(),
					},
				}
				for {
					instanceResp, instanceErr = instanceClient.RegisterInstance(instanceCtx, instanceReq)
					if instanceErr != nil {
						log.Error("register error:", instanceErr)
						break
					}
					if instanceResp.GetApplicationInstanceId() != 0 {
						appInsId = instanceResp.GetApplicationInstanceId()
						break
					}
					time.Sleep(time.Second)
				}

			} else if t.version == 6 || t.version == 7 {
				instanceClient := nr2.NewRegisterClient(t.grpcConn)
				instanceCtx, instanceCancel := context.WithTimeout(context.Background(), time.Second*3)
				defer instanceCancel()

				var instanceErr error
				var instanceResp *nr2.ServiceInstanceRegisterMapping
				hostName, _ := os.Hostname()

				var instances []*nr2.ServiceInstance
				var properties []*nc2.KeyStringValuePairV2

				properties = append(properties, &nc2.KeyStringValuePairV2{
					Key:   "os_name",
					Value: runtime.GOOS,
				})

				properties = append(properties, &nc2.KeyStringValuePairV2{
					Key:   "host_name",
					Value: hostName,
				})

				properties = append(properties, &nc2.KeyStringValuePairV2{
					Key:   "process_no",
					Value: strconv.Itoa(pid),
				})

				properties = append(properties, &nc2.KeyStringValuePairV2{
					Key:   "language",
					Value: "php",
				})

				for _, ip := range ip4s() {
					properties = append(properties, &nc2.KeyStringValuePairV2{
						Key:   "ipv4",
						Value: ip,
					})
				}

				instances = append(instances, &nr2.ServiceInstance{
					ServiceId:    appId,
					InstanceUUID: agentUUID,
					Time:         time.Now().UnixNano() / 1000000,
					Properties:   properties,
				})

				instanceReq := &nr2.ServiceInstances{
					Instances: instances,
				}
				for {
					instanceResp, instanceErr = instanceClient.DoServiceInstanceRegister(instanceCtx, instanceReq)
					if instanceErr != nil {
						log.Error(instanceErr)
						break
					}
					if instanceResp.GetServiceInstances() != nil {
						for _, v := range instanceResp.GetServiceInstances() {
							if v.GetKey() == agentUUID {
								appInsId = v.GetValue()
								break
							}
						}
					}
					if appInsId != 0 {
						break
					}
					time.Sleep(time.Second)
				}
			}

			if appInsId != 0 {
				t.registerCache[pid] = registerCache{
					Version:    info.Version,
					AppId:      appId,
					InstanceId: appInsId,
					Uuid:       agentUUID,
				}
				log.Infof("register pid %d appid %d insId %d", pid, appId, appInsId)
			}
		} else {
			log.Error("register error:", err)
		}
	}
}
