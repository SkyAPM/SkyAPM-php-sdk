/*
 * Copyright 2021 SkyAPM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package main

import "C"
import (
	"context"
	"fmt"
	common "github.com/SkyAPM/SkyAPM-php-sdk/src/collect/common/v3"
	management "github.com/SkyAPM/SkyAPM-php-sdk/src/collect/management/v3"
	"github.com/google/uuid"
	"google.golang.org/grpc"
	"net"
	"os"
	"runtime"
	"time"
)

//export ReportInstanceProperties
func ReportInstanceProperties(address, server, instance string) (string, string) {
    // todo authentication, ssl
	conn, err := grpc.Dial(address)
	if err != nil {
		return "", err.Error()
	}
	defer conn.Close()
	c := management.NewManagementServiceClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	var kv []*common.KeyStringValuePair
	hostname, _ := os.Hostname()
	kv = append(kv, []*common.KeyStringValuePair{
		{
			Key:   "os_name",
			Value: runtime.GOOS,
		},
		{
			Key:   "host_name",
			Value: hostname,
		},
		{
			Key:   "process_no",
			Value: fmt.Sprintf("%d", os.Getpid()),
		},
		{
			Key:   "language",
			Value: "php",
		},
	}...)

	ips, err := getLocalIP()
	if err != nil {
		return "", err.Error()
	}
	for _, ip := range ips {
		kv = append(kv, &common.KeyStringValuePair{
			Key:   "ipv4",
			Value: ip,
		})
	}

	if instance == "" {
		instance = uuid.New().String()
		if len(ips) > 0 {
			instance = fmt.Sprintf("%s@%s", instance, ips[0])
		}
	}

	in := &management.InstanceProperties{
		Service:         server,
		ServiceInstance: instance,
		Properties:      kv,
	}
	_, err = c.ReportInstanceProperties(ctx, in)
	if err != nil {
		return "", err.Error()
	}
	return instance, ""
}

func getLocalIP() ([]string, error) {
	var ips []string
	adders, err := net.InterfaceAddrs()
	if err != nil {
		return ips, err
	}
	for _, addr := range adders {
		n, ok := addr.(*net.IPNet)
		if !ok {
			continue
		}
		if n.IP.IsLoopback() {
			continue
		}
		if !n.IP.IsGlobalUnicast() {
			continue
		}
		ips = append(ips, n.IP.String())
	}
	return ips, nil
}

func main() {
}
