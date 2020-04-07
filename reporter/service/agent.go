package service

import (
	"github.com/SkyAPM/SkyAPM-php-sdk/reporter/logger"
	nla1 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v1"
	nla2 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v2"
	nla3 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v3"
	nm3 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/management/v3"
	nr2 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/register/v2"
	cli "github.com/urfave/cli/v2"
	"google.golang.org/grpc"
	"math/rand"
	"net"
	"os"
	"strings"
	"sync"
	"time"
)

var log = logger.Log

type register struct {
	c    net.Conn
	body string
}

type grpcClient struct {
	tssc1  nla1.TraceSegmentServiceClient
	arsc1  nla1.ApplicationRegisterServiceClient
	idsc1  nla1.InstanceDiscoveryServiceClient
	tsrsc2 nla2.TraceSegmentReportServiceClient
	tsrsc3 nla3.TraceSegmentReportServiceClient
	sipc2  nr2.ServiceInstancePingClient
	msc3   nm3.ManagementServiceClient
}

type Agent struct {
	flag              *cli.Context
	version           int
	grpcConn          *grpc.ClientConn
	grpcClient        grpcClient
	socket            string
	socketListener    net.Listener
	register          chan *register
	registerCache     map[int]registerCache
	registerCacheLock sync.RWMutex
	trace             chan string
	queue             []string
	queueLock         sync.Mutex
}

func NewAgent(cli *cli.Context) *Agent {
	var agent = &Agent{
		flag:          cli,
		version:       cli.Int("sky-version"),
		socket:        cli.String("socket"),
		register:      make(chan *register),
		trace:         make(chan string),
		registerCache: make(map[int]registerCache),
	}

	go agent.sub()

	return agent
}

func (t *Agent) Run() {
	log.Info("hello skywalking")

	defer func() {
		var err error
		if t.socketListener != nil {
			err = t.socketListener.Close()
			if err != nil {
				log.Errorln(err)
			}
		}

		if t.grpcConn != nil {
			err = t.grpcConn.Close()
			if err != nil {
				log.Errorln(err)
			}
		}
	}()
	t.connGRPC()
	t.listenSocket()
}

func (t *Agent) connGRPC() {
	var err error
	grpcAdd := t.flag.StringSlice("grpc")
	var services []string
	for _, item := range grpcAdd {
		services = append(services, strings.Split(item, ",")...)
	}

	r := rand.New(rand.NewSource(time.Now().UnixNano()))
	var grpcAddress string
	if len(services) > 0 {
		grpcAddress = services[r.Intn(len(services))]
	} else {
		log.Panic("oap server not found")
	}

	t.grpcConn, err = grpc.Dial(grpcAddress, grpc.WithInsecure())
	if err != nil {
		log.Panic(err)
	}

	log.Infof("connection %s...", grpcAddress)

	if t.version == 5 {
		t.grpcClient.tssc1 = nla1.NewTraceSegmentServiceClient(t.grpcConn)
		t.grpcClient.arsc1 = nla1.NewApplicationRegisterServiceClient(t.grpcConn)
		t.grpcClient.idsc1 = nla1.NewInstanceDiscoveryServiceClient(t.grpcConn)
	} else if t.version == 6 || t.version == 7 {
		t.grpcClient.tsrsc2 = nla2.NewTraceSegmentReportServiceClient(t.grpcConn)
		t.grpcClient.sipc2 = nr2.NewServiceInstancePingClient(t.grpcConn)
	} else if t.version == 8 {
		t.grpcClient.tsrsc3 = nla3.NewTraceSegmentReportServiceClient(t.grpcConn)
		t.grpcClient.msc3 = nm3.NewManagementServiceClient(t.grpcConn)
	}
	log.Info("🍺 skywalking php agent started successfully, enjoy yourself")
}

func (t *Agent) listenSocket() {
	var err error

	fi, _ := os.Stat(t.socket)

	if fi != nil && !fi.Mode().IsDir() {
		if err = os.RemoveAll(t.socket); err != nil {
			log.Panic(err)
		}
	}

	t.socketListener, err = net.Listen("unix", t.socket)
	if err != nil {
		log.Panic(err)
	}

	err = os.Chmod(t.socket, os.ModeSocket|0777)
	if err != nil {
		log.Warningln(err)
	}

	for {
		c, err := t.socketListener.Accept()
		if err != nil {
			log.Errorln(err)
			break
		}
		// start a new goroutine to handle
		// the new connection.
		conn := NewConn(t, c)
		go conn.Handle()
	}
}

func (t *Agent) sub() {
	heartbeatTicker := time.NewTicker(time.Second * 40)
	defer heartbeatTicker.Stop()
	traceSendTicker := time.NewTicker(time.Second * time.Duration(t.flag.Int("send-rate")))
	defer traceSendTicker.Stop()

	for {
		select {
		case <-traceSendTicker.C:
			len := len(t.queue)
			if len > 0 {
				var segments []*upstreamSegment

				t.queueLock.Lock()
				list := t.queue[:]
				t.queue = []string{}
				t.queueLock.Unlock()

				for _, trace := range list {
					info, st := format(t.version, trace)
					if st != nil {
						t.recoverRegister(info)
						segments = append(segments, st)
					}
				}
				go t.send(segments)
			}
		case <-heartbeatTicker.C:
			go t.heartbeat()
		case register := <-t.register:
			go t.doRegister(register)
		case trace := <-t.trace:
			t.queueLock.Lock()
			t.queue = append(t.queue, trace)
			t.queueLock.Unlock()
		}
	}
}
