package service

import (
	"agent/agent/logger"
	"agent/agent/pb/agent"
	"agent/agent/pb/agent2"
	"agent/agent/pb/register2"
	"container/list"
	"context"
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
	streamV5        agent.TraceSegmentService_CollectClient
	streamV6        agent2.TraceSegmentReportService_CollectClient
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

	go func() {
		// test
		time.Sleep(time.Second * 3)
		conn, _ := net.Dial("unix", agent.socket)
		for {
			conn.Write([]byte("0{1212}\n"))
			time.Sleep(time.Second)
			fmt.Println("11")
		}

	}()

	go agent.sub()

	return agent
}

func (t *Agent) Run() {
	log.Info("hello skywalking")
	t.connGRPC()
	t.listenSocket()

	defer func() {
		var err error
		err = t.socketListener.Close()
		if err != nil {
			log.Errorln(err)
		}

		if t.grpcClient.streamV5 != nil {
			_, err = t.grpcClient.streamV5.CloseAndRecv()
			if err != nil {
				log.Errorln(err)
			}
		}

		if t.grpcClient.streamV6 != nil {
			_, err = t.grpcClient.streamV6.CloseAndRecv()
			if err != nil {
				log.Errorln(err)
			}
		}
		err = t.grpcConn.Close()
		if err != nil {
			log.Errorln(err)
		}
	}()
}

func (t *Agent) connGRPC() {
	var err error
	t.grpcConn, err = grpc.Dial(t.flag.String("grpc"), grpc.WithInsecure())
	if err != nil {
		log.Panic(err)
	}
	t.grpcClient.segmentClientV5 = agent.NewTraceSegmentServiceClient(t.grpcConn)
	t.grpcClient.segmentClientV6 = agent2.NewTraceSegmentReportServiceClient(t.grpcConn)
	t.grpcClient.pingClient5 = agent.NewInstanceDiscoveryServiceClient(t.grpcConn)
	t.grpcClient.pintClient6 = register2.NewServiceInstancePingClient(t.grpcConn)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel()
	ctx6, cancel6 := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel6()

	t.grpcClient.streamV5, err = t.grpcClient.segmentClientV5.Collect(ctx)
	if err != nil {
		log.Warningln(err)
	}

	t.grpcClient.streamV6, err = t.grpcClient.segmentClientV6.Collect(ctx6)
	if err != nil {
		log.Warningln(err)
	}

	if t.grpcClient.streamV5 == nil && t.grpcClient.streamV6 == nil {
		log.Panic("No stream available")
	}
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

	err = os.Chmod(t.socket, os.ModeSocket|0666)
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
	heartbeatTicker := time.NewTicker(time.Duration(time.Second * 40))
	defer heartbeatTicker.Stop()

	for {
		select {
		case <-heartbeatTicker.C:
			go t.heartbeat()
		case register := <-t.register:
			go t.doRegister(register)
		case trace := <-t.trace:
			t.queue.PushBack(trace)

			if t.queue.Len() > 100 {
				var segments []*upstreamSegment
				for i := 0; i < 100; i++ {
					// front top 100
					first := t.queue.Front().Value
					st := format(fmt.Sprintf("%v", first))
					if st != nil {
						segments = append(segments, st)
					}
				}
				go t.send(segments)
			}
		}
	}
}
