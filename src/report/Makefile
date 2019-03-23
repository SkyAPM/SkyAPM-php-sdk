#
# Copyright 2015 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11
ifeq ($(SYSTEM),Darwin)
LDFLAGS += -Ldeps/boost/stage/lib -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -lgrpc++_reflection\
           -ldl\
           -lboost_regex
else
LDFLAGS += -Ldeps/boost/stage/lib -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl\
           -lboost_regex
endif
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`


all: system-check boost report_client

boost:
	cd deps/boost && ./bootstrap.sh && \
	./b2 stage --with-regex link=static runtime-link=shared threading=multi

report_client: ../grpc/.libs/ApplicationRegisterService-grpc.o ../grpc/.libs/ApplicationRegisterService.o ../grpc/.libs/Common-grpc.o ../grpc/.libs/Common.o ../grpc/.libs/DiscoveryService-grpc.o ../grpc/.libs/DiscoveryService.o ../grpc/.libs/Downstream-grpc.o ../grpc/.libs/Downstream.o ../grpc/.libs/JVMMetricsService-grpc.o ../grpc/.libs/JVMMetricsService.o ../grpc/.libs/KeyWithIntegerValue-grpc.o ../grpc/.libs/KeyWithIntegerValue.o ../grpc/.libs/KeyWithStringValue-grpc.o ../grpc/.libs/KeyWithStringValue.o ../grpc/.libs/NetworkAddressRegisterService-grpc.o ../grpc/.libs/NetworkAddressRegisterService.o ../grpc/.libs/TraceSegmentService-grpc.o ../grpc/.libs/TraceSegmentService.o
	$(CXX) report_client.cpp $^ $(LDFLAGS) -Ideps/boost $(CXXFLAGS) -o $@

clean:
	rm -rf *.o *.pb.cc *.pb.h report_client \
	deps/boost/bin.v2 deps/boost/stage deps/boost/b2 deps/boost/bjam \
	deps/boost/project-config.* deps/boost/bootstrap.log


# The following is to test your system and ensure a smoother experience.
# They are by no means necessary to actually compile a grpc-enabled software.

PROTOC_CMD = which $(PROTOC)
PROTOC_CHECK_CMD = $(PROTOC) --version | grep -q libprotoc.3
PLUGIN_CHECK_CMD = which $(GRPC_CPP_PLUGIN)
HAS_PROTOC = $(shell $(PROTOC_CMD) > /dev/null && echo true || echo false)
ifeq ($(HAS_PROTOC),true)
HAS_VALID_PROTOC = $(shell $(PROTOC_CHECK_CMD) 2> /dev/null && echo true || echo false)
endif
HAS_PLUGIN = $(shell $(PLUGIN_CHECK_CMD) > /dev/null && echo true || echo false)

SYSTEM_OK = false
ifeq ($(HAS_VALID_PROTOC),true)
ifeq ($(HAS_PLUGIN),true)
SYSTEM_OK = true
endif
endif

system-check:
ifneq ($(HAS_VALID_PROTOC),true)
	@echo " DEPENDENCY ERROR"
	@echo
	@echo "You don't have protoc 3.0.0 installed in your path."
	@echo "Please install Google protocol buffers 3.0.0 and its compiler."
	@echo "You can find it here:"
	@echo
	@echo "   https://github.com/google/protobuf/releases/tag/v3.0.0"
	@echo
	@echo "Here is what I get when trying to evaluate your version of protoc:"
	@echo
	-$(PROTOC) --version
	@echo
	@echo
endif
ifneq ($(HAS_PLUGIN),true)
	@echo " DEPENDENCY ERROR"
	@echo
	@echo "You don't have the grpc c++ protobuf plugin installed in your path."
	@echo "Please install grpc. You can find it here:"
	@echo
	@echo "   https://github.com/grpc/grpc"
	@echo
	@echo "Here is what I get when trying to detect if you have the plugin:"
	@echo
	-which $(GRPC_CPP_PLUGIN)
	@echo
	@echo
endif
ifneq ($(SYSTEM_OK),true)
	@false
endif