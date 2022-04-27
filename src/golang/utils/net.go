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
package utils

import "net"

func GetLocalIP() ([]string, error) {
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
