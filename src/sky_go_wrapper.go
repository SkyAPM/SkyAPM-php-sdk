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
	"fmt"
	"github.com/SkyAPM/SkyAPM-php-sdk/src/golang/protocol"
	"github.com/google/uuid"
	"math/rand"
	"os"
	"strings"
	"time"
)

var std *protocol.Protocol

//export skywalking_connect
func skywalking_connect(address, server, instance *C.char) {
	std = protocol.NewProtocol(C.GoString(address), C.GoString(server), C.GoString(instance))
	return
}

//export skywalking_get_instance
func skywalking_get_instance() *C.char {
	return C.CString(std.Instance)
}

//export skywalking_write_segment
func skywalking_write_segment(sky, json *C.char) {
	std.WriteSegment(C.GoString(json))
}

//export skywalking_trace_id_new
func skywalking_trace_id_new() *C.char {
	rand.Seed(time.Now().UnixNano())
	traceId := fmt.Sprintf("%s.%d.%d", uuid.New().String(), os.Getpid(), rand.Intn(999999)+5000)
	return C.CString(strings.ReplaceAll(traceId, "-", ""))
}

func main() {
}
