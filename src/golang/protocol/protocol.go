package protocol

import (
	"context"
	"fmt"
	common "github.com/SkyAPM/SkyAPM-php-sdk/src/collect/common/v3"
	agent "github.com/SkyAPM/SkyAPM-php-sdk/src/collect/language/agent/v3"
	management "github.com/SkyAPM/SkyAPM-php-sdk/src/collect/management/v3"
	"github.com/SkyAPM/SkyAPM-php-sdk/src/golang/utils"
	"github.com/google/uuid"
	"google.golang.org/grpc"
	"os"
	"runtime"
	"time"
)

type Protocol struct {
	conn     *grpc.ClientConn
	address  string
	server   string
	instance string
	segments chan *agent.SegmentObject
}

func NewProtocol(address, server, instance string) *Protocol {
	return &Protocol{}
}

func (s *Protocol) ReportInstanceProperties() error {
	err := s.dail()
	if err != nil {
		return err
	}

	var kv []*common.KeyStringValuePair
	hostname, _ := os.Hostname()
	kv = append(kv, []*common.KeyStringValuePair{
		{Key: "os_name", Value: runtime.GOOS},
		{Key: "host_name", Value: hostname},
		{Key: "process_no", Value: fmt.Sprintf("%d", os.Getpid())},
		{Key: "language", Value: "php"},
	}...)

	ips, err := utils.GetLocalIP()
	if err != nil {
		return err
	}
	for _, ip := range ips {
		kv = append(kv, &common.KeyStringValuePair{
			Key:   "ipv4",
			Value: ip,
		})
	}

	if s.instance == "" {
		s.instance = uuid.New().String()
		if len(ips) > 0 {
			s.instance = fmt.Sprintf("%s@%s", s.instance, ips[0])
		}
	}

	in := &management.InstanceProperties{
		Service:         s.server,
		ServiceInstance: s.instance,
		Properties:      kv,
	}

	c := management.NewManagementServiceClient(s.conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	_, err = c.ReportInstanceProperties(ctx, in)
	if err != nil {
		return err
	}

	go s.keepAlive()
	go s.collect()

	return nil
}

func (s *Protocol) keepAlive() error {
	err := s.dail()
	if err != nil {
		return err
	}

	c := management.NewManagementServiceClient(s.conn)

	for {
		in := &management.InstancePingPkg{
			Service:         s.server,
			ServiceInstance: s.instance,
		}

		ctx, cancel := context.WithTimeout(context.Background(), time.Second)
		c.KeepAlive(ctx, in)
		cancel()
		time.Sleep(time.Second)
	}

	return nil
}

func (s *Protocol) WriteSegment() {
	// todo
}

func (s *Protocol) collect() {
	c := agent.NewTraceSegmentReportServiceClient(s.conn)
	sender, _ := c.Collect(context.Background())

	for segment := range s.segments {
		sender.Send(segment)
	}
}

func (s *Protocol) dail() error {
	if s.conn != nil {
		return nil
	}

	// todo authentication, ssl
	var err error
	s.conn, err = grpc.Dial(s.address)
	if err != nil {
		return err
	}
	return nil
}
