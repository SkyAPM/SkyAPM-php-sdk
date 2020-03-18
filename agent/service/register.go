package service

import (
	"agent/agent/pb/agent"
	"agent/agent/pb/common"
	"agent/agent/pb/register2"
	"context"
	"encoding/json"
	"github.com/google/uuid"
	"net"
	"os"
	"runtime"
	"strconv"
	"time"
)

type registerCache struct {
	Version    int
	AppId      int32
	InstanceId int32
	Uuid       string
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

func (t *Agent) recoverRegister(r string) {
	var info trace
	err := json.Unmarshal([]byte(r), &info)
	if err == nil {
		if _, ok := t.registerCache.Load(info.Pid); !ok {
			t.registerCache.Store(info.Pid, registerCache{
				Version:    info.Version,
				AppId:      info.ApplicationId,
				InstanceId: info.ApplicationInstance,
				Uuid:       info.Uuid,
			})
		}
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
	if value, ok := t.registerCache.Load(pid); ok {
		bind := value.(registerCache)
		log.Infof("register => pid %d appid %d insId %d", pid, bind.AppId, bind.InstanceId)
		r.c.Write([]byte(strconv.FormatInt(int64(bind.AppId), 10) + "," + strconv.FormatInt(int64(bind.InstanceId), 10) + "," + bind.Uuid))
		return
	} else {
		r.c.Write([]byte(""))
	}

	t.registerCacheLock.Lock()
	defer t.registerCacheLock.Unlock()

	// if map not found pid.. start register
	if _, ok := t.registerCache.Load(pid); !ok {
		log.Infof("start register pid %d used SkyWalking v%d", pid, info.Version)
		var regAppStatus = false
		var appId int32 = 0
		var appInsId int32 = 0
		var regErr error
		agentUUID := uuid.New().String()

		if info.Version == 5 {
			c := agent.NewApplicationRegisterServiceClient(t.grpcConn)
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			var regResp *agent.ApplicationMapping

			// loop register
			for {
				regResp, regErr = c.ApplicationCodeRegister(ctx, &agent.Application{
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
		} else if info.Version == 6 {
			c := register2.NewRegisterClient(t.grpcConn)
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			var regResp *register2.ServiceRegisterMapping
			var services []*register2.Service
			services = append(services, &register2.Service{
				ServiceName: info.AppCode,
			})
			// loop register
			for {
				regResp, regErr = c.DoServiceRegister(ctx, &register2.Services{
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
		}

		if regAppStatus {
			// start reg instance
			if info.Version == 5 {
				instanceClient := agent.NewInstanceDiscoveryServiceClient(t.grpcConn)
				instanceCtx, instanceCancel := context.WithTimeout(context.Background(), time.Second*3)
				defer instanceCancel()

				var instanceErr error
				var instanceResp *agent.ApplicationInstanceMapping
				hostName, _ := os.Hostname()

				instanceReq := &agent.ApplicationInstance{
					ApplicationId: appId,
					AgentUUID:     agentUUID,
					RegisterTime:  time.Now().UnixNano() / 1000000,
					Osinfo: &agent.OSInfo{
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
			} else if info.Version == 6 {
				instanceClient := register2.NewRegisterClient(t.grpcConn)
				instanceCtx, instanceCancel := context.WithTimeout(context.Background(), time.Second*3)
				defer instanceCancel()

				var instanceErr error
				var instanceResp *register2.ServiceInstanceRegisterMapping
				hostName, _ := os.Hostname()

				var instances []*register2.ServiceInstance
				var properties []*common.KeyStringValuePair

				properties = append(properties, &common.KeyStringValuePair{
					Key:   "os_name",
					Value: runtime.GOOS,
				})

				properties = append(properties, &common.KeyStringValuePair{
					Key:   "host_name",
					Value: hostName,
				})

				properties = append(properties, &common.KeyStringValuePair{
					Key:   "process_no",
					Value: strconv.Itoa(pid),
				})

				properties = append(properties, &common.KeyStringValuePair{
					Key:   "language",
					Value: "php",
				})

				for _, ip := range ip4s() {
					properties = append(properties, &common.KeyStringValuePair{
						Key:   "ipV4s",
						Value: ip,
					})
				}

				instances = append(instances, &register2.ServiceInstance{
					ServiceId:    appId,
					InstanceUUID: agentUUID,
					Time:         time.Now().UnixNano() / 1000000,
					Properties:   properties,
				})

				instanceReq := &register2.ServiceInstances{
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
				t.registerCache.Store(pid, registerCache{
					Version:    info.Version,
					AppId:      appId,
					InstanceId: appInsId,
					Uuid:       agentUUID,
				})
				log.Infof("register pid %d appid %d insId %d", pid, appId, appInsId)
			}
		} else {
			log.Error("register error:", err)
		}
	}
}
