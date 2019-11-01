package service

import (
	"agent/agent/logger"
	"agent/agent/pb/agent"
	"agent/agent/pb/agent2"
	"agent/agent/pb/register2"
	"container/list"
	"fmt"
	"github.com/urfave/cli"
	"google.golang.org/grpc"
	"net"
	"os"
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
	registerCache     sync.Map
	registerCacheLock sync.Mutex
	trace             chan string
	queue             *list.List
}

func NewAgent(cli *cli.Context) *Agent {
	var agent = &Agent{
		flag:     cli,
		socket:   cli.String("socket"),
		register: make(chan *register),
		trace:    make(chan string),
		queue:    list.New(),
	}

	go agent.sub()

	return agent
}

func (t *Agent) Run() {
	log.Info("hello skywalking")
	t.connGRPC()
	t.listenSocket()

	log.Info("🍺 skywalking php agent started successfully, enjoy yourself")
	defer func() {
		var err error
		err = t.socketListener.Close()
		if err != nil {
			log.Errorln(err)
		}

		err = t.grpcConn.Close()
		if err != nil {
			log.Errorln(err)
		}
	}()
}

func (t *Agent) connGRPC() {
	var err error
	grpcAdd := t.flag.String("grpc")
	t.grpcConn, err = grpc.Dial(grpcAdd, grpc.WithInsecure())
	if err != nil {
		log.Panic(err)
	}

	log.Infof("connection %s...", grpcAdd)
	t.grpcClient.segmentClientV5 = agent.NewTraceSegmentServiceClient(t.grpcConn)
	t.grpcClient.segmentClientV6 = agent2.NewTraceSegmentReportServiceClient(t.grpcConn)
	t.grpcClient.pingClient5 = agent.NewInstanceDiscoveryServiceClient(t.grpcConn)
	t.grpcClient.pintClient6 = register2.NewServiceInstancePingClient(t.grpcConn)
}

func (t *Agent) listenSocket() {
	var err error
	if err = os.RemoveAll(t.socket); err != nil {
		log.Panic(err)
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
			len := t.queue.Len()
			if len > 0 {
				var segments []*upstreamSegment
				for i := 0; i < len; i++ {
					// front top 100
					e := t.queue.Front()
					st := format(fmt.Sprintf("%v", e.Value))
					if st != nil {
						segments = append(segments, st)
					}
					t.queue.Remove(e)
				}
				go t.send(segments)
			}
		case <-heartbeatTicker.C:
			go t.heartbeat()
		case register := <-t.register:
			go t.doRegister(register)
		case trace := <-t.trace:
			t.queue.PushBack(trace)
			go t.recoverRegister(trace)
		}
	}
}
