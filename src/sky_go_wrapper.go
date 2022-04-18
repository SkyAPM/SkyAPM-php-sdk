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
	"strings"
	"os"
	"time"
	"unsafe"
)

var std *protocol.Protocol
var refs struct {
    objs map[int32]*protocol.Protocol
}

func init() {
    refs.objs = make(map[int32]*protocol.Protocol)
}

//export skywalking_connect
func skywalking_connect(address, server, instance *C.char) *C.char {
    var id int32 = 1
    refs.objs[id] = protocol.NewProtocol(C.GoString(address), C.GoString(server), C.GoString(instance))

	return (*C.char)(unsafe.Pointer(uintptr(id)))
}

//export skywalking_get_instance
func skywalking_get_instance(sky *C.char) *C.char {
    id := int32(uintptr(unsafe.Pointer(sky)))
    instance := refs.objs[id].Instance
    return C.CString(instance)
}

//export ReportInstanceProperties
func ReportInstanceProperties() int {
	if err := std.ReportInstanceProperties(); err != nil {
		return -1
	}
	return 0
}

//export skywalking_write_segment
func skywalking_write_segment(sky, json *C.char) {
    id := int32(uintptr(unsafe.Pointer(sky)))
    obj := refs.objs[id]
    fmt.Println(C.GoString(json), obj)
	// std.WriteSegment()
}

//export skywalking_trace_id_new
func skywalking_trace_id_new() *C.char {
	rand.Seed(time.Now().UnixNano())
	traceId := fmt.Sprintf("%s.%d.%d", uuid.New().String(), os.Getpid(), rand.Intn(999999)+5000)
    return C.CString(strings.ReplaceAll(traceId, "-", ""))
}

func main() {
}
