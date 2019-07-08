package main

import (
	"agent/agent/pb5"
	"agent/agent/pb6/common"
	pb6Reg "agent/agent/pb6/register"
	"agent/agent/service"
	"context"
	"encoding/json"
	"fmt"
	"github.com/google/uuid"
	"google.golang.org/grpc"
	"io"
	"net"
	"os"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"time"
)

type PHPSkyBind struct {
	Version    int
	AppId      int32
	InstanceId int32
	Uuid       string
}

type Register struct {
	AppCode string `json:"app_code"`
	Pid     int    `json:"pid"`
	Version int    `json:"version"`
}

var registerMapLock = new(sync.Mutex)
var registerMap sync.Map
var grpcConn *grpc.ClientConn

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

func register(c net.Conn, j string) {
	defer func() {
		err := recover()
		if err != nil {
			fmt.Println("System error[register]:", err)
		}
	}()

	info := Register{}
	err := json.Unmarshal([]byte(j), &info)
	if err != nil {
		fmt.Println("register => ", err)
		c.Write([]byte(""))
		return
	}

	pid := info.Pid
	if value, ok := registerMap.Load(pid); ok {
		bind := value.(PHPSkyBind)
		fmt.Printf("register => pid %d appid %d insId %d\n", pid, bind.AppId, bind.InstanceId)
		c.Write([]byte(strconv.FormatInt(int64(bind.AppId), 10) + "," + strconv.FormatInt(int64(bind.InstanceId), 10)))
		return
	} else {
		c.Write([]byte(""))
	}

	registerMapLock.Lock()
	defer registerMapLock.Unlock()

	// if map not found pid.. start register
	if _, ok := registerMap.Load(pid); !ok {
		fmt.Println("register => Start register...")
		var regAppStatus = false
		var appId int32 = 0
		var appInsId int32 = 0
		var regErr error
		agentUUID := uuid.New().String()

		if info.Version == 5 {
			c := pb5.NewApplicationRegisterServiceClient(grpcConn)
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			var regResp *pb5.ApplicationMapping

			// loop register
			for {
				regResp, regErr = c.ApplicationCodeRegister(ctx, &pb5.Application{
					ApplicationCode: info.AppCode,
				})
				if regErr != nil {
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
			c := pb6Reg.NewRegisterClient(grpcConn)
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			var regResp *pb6Reg.ServiceRegisterMapping
			var services []*pb6Reg.Service
			services = append(services, &pb6Reg.Service{
				ServiceName: info.AppCode,
			})
			// loop register
			for {
				regResp, regErr = c.DoServiceRegister(ctx, &pb6Reg.Services{
					Services: services,
				})
				if regErr != nil {
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
				instanceClient := pb5.NewInstanceDiscoveryServiceClient(grpcConn)
				instanceCtx, instanceCancel := context.WithTimeout(context.Background(), time.Second*3)
				defer instanceCancel()

				var instanceErr error
				var instanceResp *pb5.ApplicationInstanceMapping
				hostName, _ := os.Hostname()

				instanceReq := &pb5.ApplicationInstance{
					ApplicationId: appId,
					AgentUUID:     agentUUID,
					RegisterTime:  time.Now().UnixNano() / 1000000,
					Osinfo: &pb5.OSInfo{
						OsName:    runtime.GOOS,
						Hostname:  hostName,
						ProcessNo: int32(pid),
						Ipv4S:     ip4s(),
					},
				}
				for {
					instanceResp, instanceErr = instanceClient.RegisterInstance(instanceCtx, instanceReq)
					if instanceErr != nil {
						break
					}
					if instanceResp.GetApplicationInstanceId() != 0 {
						appInsId = instanceResp.GetApplicationInstanceId()
						break
					}
					time.Sleep(time.Second)
				}
			} else if info.Version == 6 {
				instanceClient := pb6Reg.NewRegisterClient(grpcConn)
				instanceCtx, instanceCancel := context.WithTimeout(context.Background(), time.Second*3)
				defer instanceCancel()

				var instanceErr error
				var instanceResp *pb6Reg.ServiceInstanceRegisterMapping
				hostName, _ := os.Hostname()

				var instances []*pb6Reg.ServiceInstance
				var properties []*common.KeyStringValuePair

				instances = append(instances, &pb6Reg.ServiceInstance{
					ServiceId:    appId,
					InstanceUUID: agentUUID,
					Time:         time.Now().UnixNano() / 1000000,
					Properties:   properties,
				})

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
					Value: string(pid),
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

				instanceReq := &pb6Reg.ServiceInstances{
					Instances: instances,
				}
				for {
					instanceResp, instanceErr = instanceClient.DoServiceInstanceRegister(instanceCtx, instanceReq)
					if instanceErr != nil {
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
				registerMap.Store(pid, PHPSkyBind{
					Version:    info.Version,
					AppId:      appId,
					InstanceId: appInsId,
					Uuid:       agentUUID,
				})
				fmt.Println("register => Start register end...")
			}
		} else {
			fmt.Println("register => ", err)
			fmt.Println("register => Start register error...")
		}
	}
}

func handleConn(c net.Conn) {
	defer func() {
		err := recover()
		if err != nil {
			fmt.Println("System error[register]:", err)
		}
	}()

	defer func() {
		fmt.Println("Close conn..")
		c.Close()
	}()

	buf := make([]byte, 4096)
	var json string
	var endIndex int
	for {
		n, err := c.Read(buf)
		if err != nil {
			if err != io.EOF {
				fmt.Println("conn read error:", err)
			}
			return
		}
		json += string(buf[0:n])
		for {
			endIndex = strings.IndexAny(json, "\n")
			if endIndex >= 0 {
				body := json[0:endIndex]
				if body[:1] == "0" {
					fmt.Println("Service register protocol")
					go register(c, body[1:])
				} else if body[:1] == "1" {
					fmt.Println("Service send trace protocol")
					go service.SendTrace(grpcConn, body[1:])
				}
				json = json[endIndex+1:]
			} else {
				break
			}
		}
	}
}

func heartbeat() {
	defer func() {
		err := recover()
		if err != nil {
			fmt.Println("System error[heartbeat]:", err)
			go heartbeat()
		}
	}()

	for {
		registerMap.Range(func(key, value interface{}) bool {
			fmt.Println("heartbeat => ...")
			bind := value.(PHPSkyBind)

			if bind.Version == 5 {

				c := pb5.NewInstanceDiscoveryServiceClient(grpcConn)
				ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
				defer cancel()

				_, err := c.Heartbeat(ctx, &pb5.ApplicationInstanceHeartbeat{
					ApplicationInstanceId: bind.InstanceId,
					HeartbeatTime:         time.Now().UnixNano() / 1000000,
				})
				if err != nil {
					fmt.Println("heartbeat =>", err)
				} else {
					fmt.Printf("heartbeat => %d %d\n", bind.AppId, bind.InstanceId)
				}
			} else if bind.Version == 6 {
				c := pb6Reg.NewServiceInstancePingClient(grpcConn)
				ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
				defer cancel()

				_, err := c.DoPing(ctx, &pb6Reg.ServiceInstancePingPkg{
					ServiceInstanceId:   bind.InstanceId,
					Time:                time.Now().UnixNano() / 1000000,
					ServiceInstanceUUID: bind.Uuid,
				})
				if err != nil {
					fmt.Println("heartbeat =>", err)
				} else {
					fmt.Printf("heartbeat => %d %d\n", bind.AppId, bind.InstanceId)
				}
			}
			return true
		})
		time.Sleep(time.Second * 40)
	}
}

func main() {

	args := os.Args

	// connection to sky server
	fmt.Println("hello skywalking")
	var err error
	grpcConn, err = grpc.Dial(args[1], grpc.WithInsecure())

	if err != nil {
		fmt.Println(err)
	}
	defer grpcConn.Close()

	if err := os.RemoveAll("/tmp/sky_agent.sock"); err != nil {
		fmt.Println(err)
	}

	l, err := net.Listen("unix", "/tmp/sky_agent.sock")
	if err != nil {
		fmt.Println("listen error:", err)
		return
	}
	defer l.Close()

	go heartbeat()

	for {
		c, err := l.Accept()
		if err != nil {
			fmt.Println("accept error:", err)
			break
		}
		// start a new goroutine to handle
		// the new connection.
		go handleConn(c)
	}
}
