package service

import (
	"agent/agent/pb/agent"
	"agent/agent/pb/agent2"
	"container/list"
	"context"
	"fmt"
	"google.golang.org/grpc"
	"io"
	"log"
	"net"
	"os"
	"sync"
	"time"
)

type register struct {
	c    net.Conn
	body string
}

type Agent struct {
	grpc            string
	conn            *grpc.ClientConn
	segmentClientV5 agent.TraceSegmentServiceClient
	segmentClientV6 agent2.TraceSegmentReportServiceClient
	socket          string
	socketListener  net.Listener
	register        chan *register
	registerCache   sync.Map
	trace           chan string
	queue           *list.List
}

func NewAgent() *Agent {
	var agent = &Agent{
		socket:   "/tmp/sky_agent.sock",
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
	//t.connGRPC()
	t.listenSocket()

	defer func() {
		//err := t.conn.Close()
		//if err != nil {
		//	fmt.Println(err)
		//}
		err := t.socketListener.Close()
		if err != nil {
			fmt.Println(err)
		}
	}()
}

func (t *Agent) connGRPC() {
	var err error
	t.conn, err = grpc.Dial(t.grpc, grpc.WithInsecure())
	if err != nil {
		panic(err)
	}
	t.segmentClientV5 = agent.NewTraceSegmentServiceClient(t.conn)
	t.segmentClientV6 = agent2.NewTraceSegmentReportServiceClient(t.conn)
}

func (t *Agent) listenSocket() {
	var err error
	if err = os.RemoveAll(t.socket); err != nil {
		panic(err)
	}
	t.socketListener, err = net.Listen("unix", t.socket)
	if err != nil {
		panic(err)
	}

	err = os.Chmod(t.socket, os.ModeSocket|0666)
	if err != nil {
		fmt.Println(err)
	}

	for {
		c, err := t.socketListener.Accept()
		if err != nil {
			fmt.Println(err)
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
			go t.reg(register)
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

func (t *Agent) reg(r *register) {
	fmt.Println(r)
	fmt.Println(t.segmentClientV5)
}

func (t *Agent) send(segments []*upstreamSegment) {
	var err error
	// process
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel()
	ctx6, cancel6 := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel6()

	var stream5 agent.TraceSegmentService_CollectClient
	var stream6 agent2.TraceSegmentReportService_CollectClient

	for _, segment := range segments {
		log.Println("trace => Start trace...")
		if segment.Version == 5 {
			if stream5 == nil {
				stream5, err = t.segmentClientV5.Collect(ctx)
			}

			if stream5 != nil {
				if err = stream5.Send(segment.segment); err != nil {
					if err == io.EOF {
						break
					}
					fmt.Println(err)
				}
			} else {
				fmt.Println("stream not open")
			}

		} else if segment.Version == 6 {
			if stream6 == nil {
				stream6, err = t.segmentClientV6.Collect(ctx6)
			}
			if stream6 != nil {
				if err = stream6.Send(segment.segment); err != nil {
					if err == io.EOF {
						break
					}
					fmt.Println(err)
				}
			} else {
				fmt.Println("stream not open")
			}
		}
	}

	if stream5 != nil {
		_, err = stream5.CloseAndRecv()
		if err != nil {
			fmt.Println(err)
		}
	}

	if stream6 != nil {
		_, err = stream6.CloseAndRecv()
		if err != nil {
			fmt.Println(err)
		}
	}
}
