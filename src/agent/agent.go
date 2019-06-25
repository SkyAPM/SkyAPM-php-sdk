package main

import (
	"agent/agent/pb5"
	"context"
	"encoding/json"
	"fmt"
	"github.com/google/uuid"
	"google.golang.org/grpc"
	"io"
	"log"
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
			log.Println("System error[register]:", err)
		}
	}()

	info := Register{}
	err := json.Unmarshal([]byte(j), &info)
	if err != nil {
		log.Println("register => ", err)
		c.Write([]byte(""))
		return
	}

	pid := info.Pid
	if value, ok := registerMap.Load(pid); ok {
		bind := value.(PHPSkyBind)
		log.Printf("register => pid %d appid %d insId %d\n", pid, bind.AppId, bind.InstanceId)
		c.Write([]byte(strconv.FormatInt(int64(bind.AppId), 10) + "," + strconv.FormatInt(int64(bind.InstanceId), 10)))
		return
	} else {
		c.Write([]byte(""))
	}

	registerMapLock.Lock()
	defer registerMapLock.Unlock()

	// if map not found pid.. start register
	if _, ok := registerMap.Load(pid); !ok {
		log.Println("register => Start register...")
		c := pb5.NewApplicationRegisterServiceClient(grpcConn)
		ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
		defer cancel()

		var regErr error
		var regResp *pb5.ApplicationMapping
		var regAppStatus = false

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
				break
			}
			time.Sleep(time.Second)
		}

		if regAppStatus {
			// start reg instance
			instanceClient := pb5.NewInstanceDiscoveryServiceClient(grpcConn)
			instanceCtx, instanceCancel := context.WithTimeout(context.Background(), time.Second*3)
			defer instanceCancel()

			var instanceErr error
			var instanceResp *pb5.ApplicationInstanceMapping
			hostName, _ := os.Hostname()

			agentUUID := uuid.New().String()
			fmt.Println(agentUUID)

			instanceReq := &pb5.ApplicationInstance{
				ApplicationId: regResp.GetApplication().GetValue(),
				AgentUUID:     agentUUID,
				RegisterTime:  time.Now().UnixNano(),
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
					break
				}
				time.Sleep(time.Second)
			}

			if instanceResp != nil && instanceResp.GetApplicationInstanceId() != 0 {
				registerMap.Store(pid, PHPSkyBind{
					Version:    5,
					AppId:      regResp.GetApplication().GetValue(),
					InstanceId: instanceResp.GetApplicationInstanceId(),
					Uuid:       agentUUID,
				})
				log.Println("register => Start register end...")
			}
		} else {
			log.Println("register => ", err)
			log.Println("register => Start register error...")
		}

	}
}

func sendTrace(json string) {
	fmt.Println(json)
}

func handleConn(c net.Conn) {
	defer func() {
		err := recover()
		if err != nil {
			log.Println("System error[register]:", err)
		}
	}()

	defer c.Close()
	defer log.Println("Close conn..")

	buf := make([]byte, 4096)
	var json string
	var endIndex int
	for {
		n, err := c.Read(buf)
		if err != nil {
			if err != io.EOF {
				log.Println("conn read error:", err)
			}
			return
		}
		json += string(buf[0:n])
		for {
			endIndex = strings.IndexAny(json, "\n")
			if endIndex >= 0 {
				body := json[0:endIndex]
				if body[:1] == "0" {
					log.Println("Service register protocol")
					go register(c, body[1:])
				} else if body[:1] == "1" {
					log.Println("Service send trace protocol")
					go sendTrace(body[1:])
				}
				json = json[endIndex+1:]
			} else {
				break
			}
		}
	}
}

func main() {

	// connection to sky server
	var err error
	grpcConn, err = grpc.Dial("172.16.68.37:11800", grpc.WithInsecure())

	if err != nil {
		log.Fatal(err)
	}
	defer grpcConn.Close()

	if err := os.RemoveAll("/tmp/sky_agent.sock"); err != nil {
		log.Fatal(err)
	}

	l, err := net.Listen("unix", "/tmp/sky_agent.sock")
	if err != nil {
		fmt.Println("listen error:", err)
		return
	}
	defer l.Close()

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
