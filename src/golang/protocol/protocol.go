// Copyright 2022 SkyAPM
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
	"google.golang.org/protobuf/encoding/protojson"
	"os"
	"runtime"
	"time"
)

type Protocol struct {
	conn     *grpc.ClientConn
	address  string
	server   string
	Instance string
	ready    bool
	segments chan []byte
}

func NewProtocol(address, server, instance string) *Protocol {
	if instance == "" {
		ips, _ := utils.GetLocalIP()
		ip := "localhost"
		if len(ips) > 0 {
			ip = ips[0]
		}
		instance = fmt.Sprintf("%s@%s", uuid.New().String(), ip)
	}
	p := &Protocol{
		address:  address,
		server:   server,
		Instance: instance,
		segments: make(chan []byte, 1024),
	}
	go p.ReportInstanceProperties()
	return p
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

	in := &management.InstanceProperties{
		Service:         s.server,
		ServiceInstance: s.Instance,
		Properties:      kv,
	}

	c := management.NewManagementServiceClient(s.conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	_, err = c.ReportInstanceProperties(ctx, in)
	if err != nil {
		return err
	}

	s.ready = true
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
			ServiceInstance: s.Instance,
		}

		ctx, cancel := context.WithTimeout(context.Background(), time.Second)
		c.KeepAlive(ctx, in)
		cancel()
		time.Sleep(time.Second)
	}

	return nil
}

func (s *Protocol) WriteSegment(json string) {
	select {
	case s.segments <- []byte(json):
	default:
	}
}

func (s *Protocol) collect() {
	err := s.dail()
	if err != nil {
		return
	}
	c := agent.NewTraceSegmentReportServiceClient(s.conn)
	sender, _ := c.Collect(context.Background())

	for bts := range s.segments {
		if !s.ready {
			continue
		}
		var segment *agent.SegmentObject
		if protojson.Unmarshal(bts, segment) == nil {
			fmt.Println(segment)
			sender.Send(segment)
		}
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
