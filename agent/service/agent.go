package service

import (
	"agent/agent/logger"
	"agent/agent/pb/agent"
	"agent/agent/pb/agent2"
	"agent/agent/pb/register2"
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
	segmentClientV5 agent.TraceSegmentServiceClient
	segmentClientV6 agent2.TraceSegmentReportServiceClient
	pingClient5     agent.InstanceDiscoveryServiceClient
	pintClient6     register2.ServiceInstancePingClient
}

type Agent struct {
	flag              *cli.Context
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
	grpcAddress := services[r.Intn(len(services))]

	t.grpcConn, err = grpc.Dial(grpcAddress, grpc.WithInsecure())
	if err != nil {
		log.Panic(err)
	}

	log.Infof("connection %s...", grpcAddress)
	t.grpcClient.segmentClientV5 = agent.NewTraceSegmentServiceClient(t.grpcConn)
	t.grpcClient.segmentClientV6 = agent2.NewTraceSegmentReportServiceClient(t.grpcConn)
	t.grpcClient.pingClient5 = agent.NewInstanceDiscoveryServiceClient(t.grpcConn)
	t.grpcClient.pintClient6 = register2.NewServiceInstancePingClient(t.grpcConn)
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
					info, st := format(trace)
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
