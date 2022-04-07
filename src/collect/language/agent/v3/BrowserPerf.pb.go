//
// Licensed to the Apache Software Foundation (ASF) under one or more
// contributor license agreements.  See the NOTICE file distributed with
// this work for additional information regarding copyright ownership.
// The ASF licenses this file to You under the Apache License, Version 2.0
// (the "License"); you may not use this file except in compliance with
// the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.28.0
// 	protoc        v3.19.4
// source: browser/BrowserPerf.proto

package v3

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	v3 "github.com/SkyAPM/SkyAPM-php-sdk/src/collect/common/v3"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type ErrorCategory int32

const (
	ErrorCategory_ajax     ErrorCategory = 0
	ErrorCategory_resource ErrorCategory = 1
	ErrorCategory_vue      ErrorCategory = 2
	ErrorCategory_promise  ErrorCategory = 3
	ErrorCategory_js       ErrorCategory = 4
	ErrorCategory_unknown  ErrorCategory = 5
)

// Enum value maps for ErrorCategory.
var (
	ErrorCategory_name = map[int32]string{
		0: "ajax",
		1: "resource",
		2: "vue",
		3: "promise",
		4: "js",
		5: "unknown",
	}
	ErrorCategory_value = map[string]int32{
		"ajax":     0,
		"resource": 1,
		"vue":      2,
		"promise":  3,
		"js":       4,
		"unknown":  5,
	}
)

func (x ErrorCategory) Enum() *ErrorCategory {
	p := new(ErrorCategory)
	*p = x
	return p
}

func (x ErrorCategory) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (ErrorCategory) Descriptor() protoreflect.EnumDescriptor {
	return file_browser_BrowserPerf_proto_enumTypes[0].Descriptor()
}

func (ErrorCategory) Type() protoreflect.EnumType {
	return &file_browser_BrowserPerf_proto_enumTypes[0]
}

func (x ErrorCategory) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use ErrorCategory.Descriptor instead.
func (ErrorCategory) EnumDescriptor() ([]byte, []int) {
	return file_browser_BrowserPerf_proto_rawDescGZIP(), []int{0}
}

type BrowserPerfData struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Service string `protobuf:"bytes,1,opt,name=service,proto3" json:"service,omitempty"`
	// Service version in browser is the Instance concept in the backend.
	ServiceVersion string `protobuf:"bytes,2,opt,name=serviceVersion,proto3" json:"serviceVersion,omitempty"`
	// Perf data time, set by the backend side.
	Time int64 `protobuf:"varint,3,opt,name=time,proto3" json:"time,omitempty"`
	// Page path in browser is the endpoint concept in the backend
	// Page path in the browser, mostly it is URI, without parameter
	PagePath string `protobuf:"bytes,4,opt,name=pagePath,proto3" json:"pagePath,omitempty"`
	// Unit of all time related field should be `ms`.
	RedirectTime int32 `protobuf:"varint,5,opt,name=redirectTime,proto3" json:"redirectTime,omitempty"`
	// DNS query time
	DnsTime int32 `protobuf:"varint,6,opt,name=dnsTime,proto3" json:"dnsTime,omitempty"`
	// Time to first Byte
	TtfbTime int32 `protobuf:"varint,7,opt,name=ttfbTime,proto3" json:"ttfbTime,omitempty"`
	//  TCP connection time
	TcpTime int32 `protobuf:"varint,8,opt,name=tcpTime,proto3" json:"tcpTime,omitempty"`
	// Content transfer time
	TransTime int32 `protobuf:"varint,9,opt,name=transTime,proto3" json:"transTime,omitempty"`
	// Dom parsing time
	DomAnalysisTime int32 `protobuf:"varint,10,opt,name=domAnalysisTime,proto3" json:"domAnalysisTime,omitempty"`
	// First paint time or blank screen time
	FptTime int32 `protobuf:"varint,11,opt,name=fptTime,proto3" json:"fptTime,omitempty"`
	// Dom ready time
	DomReadyTime int32 `protobuf:"varint,12,opt,name=domReadyTime,proto3" json:"domReadyTime,omitempty"`
	// Page full load time
	LoadPageTime int32 `protobuf:"varint,13,opt,name=loadPageTime,proto3" json:"loadPageTime,omitempty"`
	// Synchronous load resources in the page
	ResTime int32 `protobuf:"varint,14,opt,name=resTime,proto3" json:"resTime,omitempty"`
	// Only valid for HTTPS
	SslTime int32 `protobuf:"varint,15,opt,name=sslTime,proto3" json:"sslTime,omitempty"`
	// Time to interact
	TtlTime int32 `protobuf:"varint,16,opt,name=ttlTime,proto3" json:"ttlTime,omitempty"`
	// First pack time
	FirstPackTime int32 `protobuf:"varint,17,opt,name=firstPackTime,proto3" json:"firstPackTime,omitempty"`
	// First Meaningful Paint
	FmpTime int32 `protobuf:"varint,18,opt,name=fmpTime,proto3" json:"fmpTime,omitempty"`
}

func (x *BrowserPerfData) Reset() {
	*x = BrowserPerfData{}
	if protoimpl.UnsafeEnabled {
		mi := &file_browser_BrowserPerf_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *BrowserPerfData) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*BrowserPerfData) ProtoMessage() {}

func (x *BrowserPerfData) ProtoReflect() protoreflect.Message {
	mi := &file_browser_BrowserPerf_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use BrowserPerfData.ProtoReflect.Descriptor instead.
func (*BrowserPerfData) Descriptor() ([]byte, []int) {
	return file_browser_BrowserPerf_proto_rawDescGZIP(), []int{0}
}

func (x *BrowserPerfData) GetService() string {
	if x != nil {
		return x.Service
	}
	return ""
}

func (x *BrowserPerfData) GetServiceVersion() string {
	if x != nil {
		return x.ServiceVersion
	}
	return ""
}

func (x *BrowserPerfData) GetTime() int64 {
	if x != nil {
		return x.Time
	}
	return 0
}

func (x *BrowserPerfData) GetPagePath() string {
	if x != nil {
		return x.PagePath
	}
	return ""
}

func (x *BrowserPerfData) GetRedirectTime() int32 {
	if x != nil {
		return x.RedirectTime
	}
	return 0
}

func (x *BrowserPerfData) GetDnsTime() int32 {
	if x != nil {
		return x.DnsTime
	}
	return 0
}

func (x *BrowserPerfData) GetTtfbTime() int32 {
	if x != nil {
		return x.TtfbTime
	}
	return 0
}

func (x *BrowserPerfData) GetTcpTime() int32 {
	if x != nil {
		return x.TcpTime
	}
	return 0
}

func (x *BrowserPerfData) GetTransTime() int32 {
	if x != nil {
		return x.TransTime
	}
	return 0
}

func (x *BrowserPerfData) GetDomAnalysisTime() int32 {
	if x != nil {
		return x.DomAnalysisTime
	}
	return 0
}

func (x *BrowserPerfData) GetFptTime() int32 {
	if x != nil {
		return x.FptTime
	}
	return 0
}

func (x *BrowserPerfData) GetDomReadyTime() int32 {
	if x != nil {
		return x.DomReadyTime
	}
	return 0
}

func (x *BrowserPerfData) GetLoadPageTime() int32 {
	if x != nil {
		return x.LoadPageTime
	}
	return 0
}

func (x *BrowserPerfData) GetResTime() int32 {
	if x != nil {
		return x.ResTime
	}
	return 0
}

func (x *BrowserPerfData) GetSslTime() int32 {
	if x != nil {
		return x.SslTime
	}
	return 0
}

func (x *BrowserPerfData) GetTtlTime() int32 {
	if x != nil {
		return x.TtlTime
	}
	return 0
}

func (x *BrowserPerfData) GetFirstPackTime() int32 {
	if x != nil {
		return x.FirstPackTime
	}
	return 0
}

func (x *BrowserPerfData) GetFmpTime() int32 {
	if x != nil {
		return x.FmpTime
	}
	return 0
}

type BrowserErrorLog struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// UUID
	UniqueId string `protobuf:"bytes,1,opt,name=uniqueId,proto3" json:"uniqueId,omitempty"`
	Service  string `protobuf:"bytes,2,opt,name=service,proto3" json:"service,omitempty"`
	// Service version in browser is the Instance concept in the backend.
	ServiceVersion string `protobuf:"bytes,3,opt,name=serviceVersion,proto3" json:"serviceVersion,omitempty"`
	// Error log time, set by the backend side.
	Time int64 `protobuf:"varint,4,opt,name=time,proto3" json:"time,omitempty"`
	// Page path in browser is the endpoint concept in the backend
	// Page path in the browser, mostly it is URI, without parameter
	PagePath string        `protobuf:"bytes,5,opt,name=pagePath,proto3" json:"pagePath,omitempty"`
	Category ErrorCategory `protobuf:"varint,6,opt,name=category,proto3,enum=skywalking.v3.ErrorCategory" json:"category,omitempty"`
	Grade    string        `protobuf:"bytes,7,opt,name=grade,proto3" json:"grade,omitempty"`
	Message  string        `protobuf:"bytes,8,opt,name=message,proto3" json:"message,omitempty"`
	Line     int32         `protobuf:"varint,9,opt,name=line,proto3" json:"line,omitempty"`
	Col      int32         `protobuf:"varint,10,opt,name=col,proto3" json:"col,omitempty"`
	Stack    string        `protobuf:"bytes,11,opt,name=stack,proto3" json:"stack,omitempty"`
	ErrorUrl string        `protobuf:"bytes,12,opt,name=errorUrl,proto3" json:"errorUrl,omitempty"`
	// Then the PV with error is only calculated when firstReportedError is true.
	FirstReportedError bool `protobuf:"varint,13,opt,name=firstReportedError,proto3" json:"firstReportedError,omitempty"`
}

func (x *BrowserErrorLog) Reset() {
	*x = BrowserErrorLog{}
	if protoimpl.UnsafeEnabled {
		mi := &file_browser_BrowserPerf_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *BrowserErrorLog) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*BrowserErrorLog) ProtoMessage() {}

func (x *BrowserErrorLog) ProtoReflect() protoreflect.Message {
	mi := &file_browser_BrowserPerf_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use BrowserErrorLog.ProtoReflect.Descriptor instead.
func (*BrowserErrorLog) Descriptor() ([]byte, []int) {
	return file_browser_BrowserPerf_proto_rawDescGZIP(), []int{1}
}

func (x *BrowserErrorLog) GetUniqueId() string {
	if x != nil {
		return x.UniqueId
	}
	return ""
}

func (x *BrowserErrorLog) GetService() string {
	if x != nil {
		return x.Service
	}
	return ""
}

func (x *BrowserErrorLog) GetServiceVersion() string {
	if x != nil {
		return x.ServiceVersion
	}
	return ""
}

func (x *BrowserErrorLog) GetTime() int64 {
	if x != nil {
		return x.Time
	}
	return 0
}

func (x *BrowserErrorLog) GetPagePath() string {
	if x != nil {
		return x.PagePath
	}
	return ""
}

func (x *BrowserErrorLog) GetCategory() ErrorCategory {
	if x != nil {
		return x.Category
	}
	return ErrorCategory_ajax
}

func (x *BrowserErrorLog) GetGrade() string {
	if x != nil {
		return x.Grade
	}
	return ""
}

func (x *BrowserErrorLog) GetMessage() string {
	if x != nil {
		return x.Message
	}
	return ""
}

func (x *BrowserErrorLog) GetLine() int32 {
	if x != nil {
		return x.Line
	}
	return 0
}

func (x *BrowserErrorLog) GetCol() int32 {
	if x != nil {
		return x.Col
	}
	return 0
}

func (x *BrowserErrorLog) GetStack() string {
	if x != nil {
		return x.Stack
	}
	return ""
}

func (x *BrowserErrorLog) GetErrorUrl() string {
	if x != nil {
		return x.ErrorUrl
	}
	return ""
}

func (x *BrowserErrorLog) GetFirstReportedError() bool {
	if x != nil {
		return x.FirstReportedError
	}
	return false
}

var File_browser_BrowserPerf_proto protoreflect.FileDescriptor

var file_browser_BrowserPerf_proto_rawDesc = []byte{
	0x0a, 0x19, 0x62, 0x72, 0x6f, 0x77, 0x73, 0x65, 0x72, 0x2f, 0x42, 0x72, 0x6f, 0x77, 0x73, 0x65,
	0x72, 0x50, 0x65, 0x72, 0x66, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x0d, 0x73, 0x6b, 0x79,
	0x77, 0x61, 0x6c, 0x6b, 0x69, 0x6e, 0x67, 0x2e, 0x76, 0x33, 0x1a, 0x13, 0x63, 0x6f, 0x6d, 0x6d,
	0x6f, 0x6e, 0x2f, 0x43, 0x6f, 0x6d, 0x6d, 0x6f, 0x6e, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x22,
	0xaf, 0x04, 0x0a, 0x0f, 0x42, 0x72, 0x6f, 0x77, 0x73, 0x65, 0x72, 0x50, 0x65, 0x72, 0x66, 0x44,
	0x61, 0x74, 0x61, 0x12, 0x18, 0x0a, 0x07, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x07, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x12, 0x26, 0x0a,
	0x0e, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x18,
	0x02, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0e, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x56, 0x65,
	0x72, 0x73, 0x69, 0x6f, 0x6e, 0x12, 0x12, 0x0a, 0x04, 0x74, 0x69, 0x6d, 0x65, 0x18, 0x03, 0x20,
	0x01, 0x28, 0x03, 0x52, 0x04, 0x74, 0x69, 0x6d, 0x65, 0x12, 0x1a, 0x0a, 0x08, 0x70, 0x61, 0x67,
	0x65, 0x50, 0x61, 0x74, 0x68, 0x18, 0x04, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x70, 0x61, 0x67,
	0x65, 0x50, 0x61, 0x74, 0x68, 0x12, 0x22, 0x0a, 0x0c, 0x72, 0x65, 0x64, 0x69, 0x72, 0x65, 0x63,
	0x74, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x05, 0x20, 0x01, 0x28, 0x05, 0x52, 0x0c, 0x72, 0x65, 0x64,
	0x69, 0x72, 0x65, 0x63, 0x74, 0x54, 0x69, 0x6d, 0x65, 0x12, 0x18, 0x0a, 0x07, 0x64, 0x6e, 0x73,
	0x54, 0x69, 0x6d, 0x65, 0x18, 0x06, 0x20, 0x01, 0x28, 0x05, 0x52, 0x07, 0x64, 0x6e, 0x73, 0x54,
	0x69, 0x6d, 0x65, 0x12, 0x1a, 0x0a, 0x08, 0x74, 0x74, 0x66, 0x62, 0x54, 0x69, 0x6d, 0x65, 0x18,
	0x07, 0x20, 0x01, 0x28, 0x05, 0x52, 0x08, 0x74, 0x74, 0x66, 0x62, 0x54, 0x69, 0x6d, 0x65, 0x12,
	0x18, 0x0a, 0x07, 0x74, 0x63, 0x70, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x08, 0x20, 0x01, 0x28, 0x05,
	0x52, 0x07, 0x74, 0x63, 0x70, 0x54, 0x69, 0x6d, 0x65, 0x12, 0x1c, 0x0a, 0x09, 0x74, 0x72, 0x61,
	0x6e, 0x73, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x09, 0x20, 0x01, 0x28, 0x05, 0x52, 0x09, 0x74, 0x72,
	0x61, 0x6e, 0x73, 0x54, 0x69, 0x6d, 0x65, 0x12, 0x28, 0x0a, 0x0f, 0x64, 0x6f, 0x6d, 0x41, 0x6e,
	0x61, 0x6c, 0x79, 0x73, 0x69, 0x73, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x0a, 0x20, 0x01, 0x28, 0x05,
	0x52, 0x0f, 0x64, 0x6f, 0x6d, 0x41, 0x6e, 0x61, 0x6c, 0x79, 0x73, 0x69, 0x73, 0x54, 0x69, 0x6d,
	0x65, 0x12, 0x18, 0x0a, 0x07, 0x66, 0x70, 0x74, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x0b, 0x20, 0x01,
	0x28, 0x05, 0x52, 0x07, 0x66, 0x70, 0x74, 0x54, 0x69, 0x6d, 0x65, 0x12, 0x22, 0x0a, 0x0c, 0x64,
	0x6f, 0x6d, 0x52, 0x65, 0x61, 0x64, 0x79, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x0c, 0x20, 0x01, 0x28,
	0x05, 0x52, 0x0c, 0x64, 0x6f, 0x6d, 0x52, 0x65, 0x61, 0x64, 0x79, 0x54, 0x69, 0x6d, 0x65, 0x12,
	0x22, 0x0a, 0x0c, 0x6c, 0x6f, 0x61, 0x64, 0x50, 0x61, 0x67, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x18,
	0x0d, 0x20, 0x01, 0x28, 0x05, 0x52, 0x0c, 0x6c, 0x6f, 0x61, 0x64, 0x50, 0x61, 0x67, 0x65, 0x54,
	0x69, 0x6d, 0x65, 0x12, 0x18, 0x0a, 0x07, 0x72, 0x65, 0x73, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x0e,
	0x20, 0x01, 0x28, 0x05, 0x52, 0x07, 0x72, 0x65, 0x73, 0x54, 0x69, 0x6d, 0x65, 0x12, 0x18, 0x0a,
	0x07, 0x73, 0x73, 0x6c, 0x54, 0x69, 0x6d, 0x65, 0x18, 0x0f, 0x20, 0x01, 0x28, 0x05, 0x52, 0x07,
	0x73, 0x73, 0x6c, 0x54, 0x69, 0x6d, 0x65, 0x12, 0x18, 0x0a, 0x07, 0x74, 0x74, 0x6c, 0x54, 0x69,
	0x6d, 0x65, 0x18, 0x10, 0x20, 0x01, 0x28, 0x05, 0x52, 0x07, 0x74, 0x74, 0x6c, 0x54, 0x69, 0x6d,
	0x65, 0x12, 0x24, 0x0a, 0x0d, 0x66, 0x69, 0x72, 0x73, 0x74, 0x50, 0x61, 0x63, 0x6b, 0x54, 0x69,
	0x6d, 0x65, 0x18, 0x11, 0x20, 0x01, 0x28, 0x05, 0x52, 0x0d, 0x66, 0x69, 0x72, 0x73, 0x74, 0x50,
	0x61, 0x63, 0x6b, 0x54, 0x69, 0x6d, 0x65, 0x12, 0x18, 0x0a, 0x07, 0x66, 0x6d, 0x70, 0x54, 0x69,
	0x6d, 0x65, 0x18, 0x12, 0x20, 0x01, 0x28, 0x05, 0x52, 0x07, 0x66, 0x6d, 0x70, 0x54, 0x69, 0x6d,
	0x65, 0x22, 0x91, 0x03, 0x0a, 0x0f, 0x42, 0x72, 0x6f, 0x77, 0x73, 0x65, 0x72, 0x45, 0x72, 0x72,
	0x6f, 0x72, 0x4c, 0x6f, 0x67, 0x12, 0x1a, 0x0a, 0x08, 0x75, 0x6e, 0x69, 0x71, 0x75, 0x65, 0x49,
	0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x75, 0x6e, 0x69, 0x71, 0x75, 0x65, 0x49,
	0x64, 0x12, 0x18, 0x0a, 0x07, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x18, 0x02, 0x20, 0x01,
	0x28, 0x09, 0x52, 0x07, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x12, 0x26, 0x0a, 0x0e, 0x73,
	0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x18, 0x03, 0x20,
	0x01, 0x28, 0x09, 0x52, 0x0e, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x56, 0x65, 0x72, 0x73,
	0x69, 0x6f, 0x6e, 0x12, 0x12, 0x0a, 0x04, 0x74, 0x69, 0x6d, 0x65, 0x18, 0x04, 0x20, 0x01, 0x28,
	0x03, 0x52, 0x04, 0x74, 0x69, 0x6d, 0x65, 0x12, 0x1a, 0x0a, 0x08, 0x70, 0x61, 0x67, 0x65, 0x50,
	0x61, 0x74, 0x68, 0x18, 0x05, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x70, 0x61, 0x67, 0x65, 0x50,
	0x61, 0x74, 0x68, 0x12, 0x38, 0x0a, 0x08, 0x63, 0x61, 0x74, 0x65, 0x67, 0x6f, 0x72, 0x79, 0x18,
	0x06, 0x20, 0x01, 0x28, 0x0e, 0x32, 0x1c, 0x2e, 0x73, 0x6b, 0x79, 0x77, 0x61, 0x6c, 0x6b, 0x69,
	0x6e, 0x67, 0x2e, 0x76, 0x33, 0x2e, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x43, 0x61, 0x74, 0x65, 0x67,
	0x6f, 0x72, 0x79, 0x52, 0x08, 0x63, 0x61, 0x74, 0x65, 0x67, 0x6f, 0x72, 0x79, 0x12, 0x14, 0x0a,
	0x05, 0x67, 0x72, 0x61, 0x64, 0x65, 0x18, 0x07, 0x20, 0x01, 0x28, 0x09, 0x52, 0x05, 0x67, 0x72,
	0x61, 0x64, 0x65, 0x12, 0x18, 0x0a, 0x07, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x18, 0x08,
	0x20, 0x01, 0x28, 0x09, 0x52, 0x07, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x12, 0x12, 0x0a,
	0x04, 0x6c, 0x69, 0x6e, 0x65, 0x18, 0x09, 0x20, 0x01, 0x28, 0x05, 0x52, 0x04, 0x6c, 0x69, 0x6e,
	0x65, 0x12, 0x10, 0x0a, 0x03, 0x63, 0x6f, 0x6c, 0x18, 0x0a, 0x20, 0x01, 0x28, 0x05, 0x52, 0x03,
	0x63, 0x6f, 0x6c, 0x12, 0x14, 0x0a, 0x05, 0x73, 0x74, 0x61, 0x63, 0x6b, 0x18, 0x0b, 0x20, 0x01,
	0x28, 0x09, 0x52, 0x05, 0x73, 0x74, 0x61, 0x63, 0x6b, 0x12, 0x1a, 0x0a, 0x08, 0x65, 0x72, 0x72,
	0x6f, 0x72, 0x55, 0x72, 0x6c, 0x18, 0x0c, 0x20, 0x01, 0x28, 0x09, 0x52, 0x08, 0x65, 0x72, 0x72,
	0x6f, 0x72, 0x55, 0x72, 0x6c, 0x12, 0x2e, 0x0a, 0x12, 0x66, 0x69, 0x72, 0x73, 0x74, 0x52, 0x65,
	0x70, 0x6f, 0x72, 0x74, 0x65, 0x64, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x18, 0x0d, 0x20, 0x01, 0x28,
	0x08, 0x52, 0x12, 0x66, 0x69, 0x72, 0x73, 0x74, 0x52, 0x65, 0x70, 0x6f, 0x72, 0x74, 0x65, 0x64,
	0x45, 0x72, 0x72, 0x6f, 0x72, 0x2a, 0x52, 0x0a, 0x0d, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x43, 0x61,
	0x74, 0x65, 0x67, 0x6f, 0x72, 0x79, 0x12, 0x08, 0x0a, 0x04, 0x61, 0x6a, 0x61, 0x78, 0x10, 0x00,
	0x12, 0x0c, 0x0a, 0x08, 0x72, 0x65, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x10, 0x01, 0x12, 0x07,
	0x0a, 0x03, 0x76, 0x75, 0x65, 0x10, 0x02, 0x12, 0x0b, 0x0a, 0x07, 0x70, 0x72, 0x6f, 0x6d, 0x69,
	0x73, 0x65, 0x10, 0x03, 0x12, 0x06, 0x0a, 0x02, 0x6a, 0x73, 0x10, 0x04, 0x12, 0x0b, 0x0a, 0x07,
	0x75, 0x6e, 0x6b, 0x6e, 0x6f, 0x77, 0x6e, 0x10, 0x05, 0x32, 0xb3, 0x01, 0x0a, 0x12, 0x42, 0x72,
	0x6f, 0x77, 0x73, 0x65, 0x72, 0x50, 0x65, 0x72, 0x66, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65,
	0x12, 0x4c, 0x0a, 0x0f, 0x63, 0x6f, 0x6c, 0x6c, 0x65, 0x63, 0x74, 0x50, 0x65, 0x72, 0x66, 0x44,
	0x61, 0x74, 0x61, 0x12, 0x1e, 0x2e, 0x73, 0x6b, 0x79, 0x77, 0x61, 0x6c, 0x6b, 0x69, 0x6e, 0x67,
	0x2e, 0x76, 0x33, 0x2e, 0x42, 0x72, 0x6f, 0x77, 0x73, 0x65, 0x72, 0x50, 0x65, 0x72, 0x66, 0x44,
	0x61, 0x74, 0x61, 0x1a, 0x17, 0x2e, 0x73, 0x6b, 0x79, 0x77, 0x61, 0x6c, 0x6b, 0x69, 0x6e, 0x67,
	0x2e, 0x76, 0x33, 0x2e, 0x43, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64, 0x73, 0x22, 0x00, 0x12, 0x4f,
	0x0a, 0x10, 0x63, 0x6f, 0x6c, 0x6c, 0x65, 0x63, 0x74, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x4c, 0x6f,
	0x67, 0x73, 0x12, 0x1e, 0x2e, 0x73, 0x6b, 0x79, 0x77, 0x61, 0x6c, 0x6b, 0x69, 0x6e, 0x67, 0x2e,
	0x76, 0x33, 0x2e, 0x42, 0x72, 0x6f, 0x77, 0x73, 0x65, 0x72, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x4c,
	0x6f, 0x67, 0x1a, 0x17, 0x2e, 0x73, 0x6b, 0x79, 0x77, 0x61, 0x6c, 0x6b, 0x69, 0x6e, 0x67, 0x2e,
	0x76, 0x33, 0x2e, 0x43, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64, 0x73, 0x22, 0x00, 0x28, 0x01, 0x42,
	0x93, 0x01, 0x0a, 0x33, 0x6f, 0x72, 0x67, 0x2e, 0x61, 0x70, 0x61, 0x63, 0x68, 0x65, 0x2e, 0x73,
	0x6b, 0x79, 0x77, 0x61, 0x6c, 0x6b, 0x69, 0x6e, 0x67, 0x2e, 0x61, 0x70, 0x6d, 0x2e, 0x6e, 0x65,
	0x74, 0x77, 0x6f, 0x72, 0x6b, 0x2e, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x2e, 0x61,
	0x67, 0x65, 0x6e, 0x74, 0x2e, 0x76, 0x33, 0x50, 0x01, 0x5a, 0x3a, 0x73, 0x6b, 0x79, 0x77, 0x61,
	0x6c, 0x6b, 0x69, 0x6e, 0x67, 0x2e, 0x61, 0x70, 0x61, 0x63, 0x68, 0x65, 0x2e, 0x6f, 0x72, 0x67,
	0x2f, 0x72, 0x65, 0x70, 0x6f, 0x2f, 0x67, 0x6f, 0x61, 0x70, 0x69, 0x2f, 0x63, 0x6f, 0x6c, 0x6c,
	0x65, 0x63, 0x74, 0x2f, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x2f, 0x61, 0x67, 0x65,
	0x6e, 0x74, 0x2f, 0x76, 0x33, 0xaa, 0x02, 0x1d, 0x53, 0x6b, 0x79, 0x57, 0x61, 0x6c, 0x6b, 0x69,
	0x6e, 0x67, 0x2e, 0x4e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x50, 0x72, 0x6f, 0x74, 0x6f, 0x63,
	0x6f, 0x6c, 0x2e, 0x56, 0x33, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_browser_BrowserPerf_proto_rawDescOnce sync.Once
	file_browser_BrowserPerf_proto_rawDescData = file_browser_BrowserPerf_proto_rawDesc
)

func file_browser_BrowserPerf_proto_rawDescGZIP() []byte {
	file_browser_BrowserPerf_proto_rawDescOnce.Do(func() {
		file_browser_BrowserPerf_proto_rawDescData = protoimpl.X.CompressGZIP(file_browser_BrowserPerf_proto_rawDescData)
	})
	return file_browser_BrowserPerf_proto_rawDescData
}

var file_browser_BrowserPerf_proto_enumTypes = make([]protoimpl.EnumInfo, 1)
var file_browser_BrowserPerf_proto_msgTypes = make([]protoimpl.MessageInfo, 2)
var file_browser_BrowserPerf_proto_goTypes = []interface{}{
	(ErrorCategory)(0),      // 0: skywalking.v3.ErrorCategory
	(*BrowserPerfData)(nil), // 1: skywalking.v3.BrowserPerfData
	(*BrowserErrorLog)(nil), // 2: skywalking.v3.BrowserErrorLog
	(*v3.Commands)(nil),     // 3: skywalking.v3.Commands
}
var file_browser_BrowserPerf_proto_depIdxs = []int32{
	0, // 0: skywalking.v3.BrowserErrorLog.category:type_name -> skywalking.v3.ErrorCategory
	1, // 1: skywalking.v3.BrowserPerfService.collectPerfData:input_type -> skywalking.v3.BrowserPerfData
	2, // 2: skywalking.v3.BrowserPerfService.collectErrorLogs:input_type -> skywalking.v3.BrowserErrorLog
	3, // 3: skywalking.v3.BrowserPerfService.collectPerfData:output_type -> skywalking.v3.Commands
	3, // 4: skywalking.v3.BrowserPerfService.collectErrorLogs:output_type -> skywalking.v3.Commands
	3, // [3:5] is the sub-list for method output_type
	1, // [1:3] is the sub-list for method input_type
	1, // [1:1] is the sub-list for extension type_name
	1, // [1:1] is the sub-list for extension extendee
	0, // [0:1] is the sub-list for field type_name
}

func init() { file_browser_BrowserPerf_proto_init() }
func file_browser_BrowserPerf_proto_init() {
	if File_browser_BrowserPerf_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_browser_BrowserPerf_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*BrowserPerfData); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_browser_BrowserPerf_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*BrowserErrorLog); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_browser_BrowserPerf_proto_rawDesc,
			NumEnums:      1,
			NumMessages:   2,
			NumExtensions: 0,
			NumServices:   1,
		},
		GoTypes:           file_browser_BrowserPerf_proto_goTypes,
		DependencyIndexes: file_browser_BrowserPerf_proto_depIdxs,
		EnumInfos:         file_browser_BrowserPerf_proto_enumTypes,
		MessageInfos:      file_browser_BrowserPerf_proto_msgTypes,
	}.Build()
	File_browser_BrowserPerf_proto = out.File
	file_browser_BrowserPerf_proto_rawDesc = nil
	file_browser_BrowserPerf_proto_goTypes = nil
	file_browser_BrowserPerf_proto_depIdxs = nil
}
