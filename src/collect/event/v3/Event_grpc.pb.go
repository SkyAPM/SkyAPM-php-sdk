// Code generated by protoc-gen-go-grpc. DO NOT EDIT.
// versions:
// - protoc-gen-go-grpc v1.2.0
// - protoc             v3.19.4
// source: event/Event.proto

package v3

import (
	context "context"
	grpc "google.golang.org/grpc"
	codes "google.golang.org/grpc/codes"
	status "google.golang.org/grpc/status"
	v3 "github.com/SkyAPM/SkyAPM-php-sdk/src/collect/common/v3"
)

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
// Requires gRPC-Go v1.32.0 or later.
const _ = grpc.SupportPackageIsVersion7

// EventServiceClient is the client API for EventService service.
//
// For semantics around ctx use and closing/ending streaming RPCs, please refer to https://pkg.go.dev/google.golang.org/grpc/?tab=doc#ClientConn.NewStream.
type EventServiceClient interface {
	// When reporting an event, you typically call the collect function twice, one for starting of the event and the other one for ending of the event, with the same UUID.
	// There are also cases where you have both start time and end time already, for example, when exporting events from a 3rd-party system,
	// the start time and end time are already known so that you can call the collect function only once.
	Collect(ctx context.Context, opts ...grpc.CallOption) (EventService_CollectClient, error)
}

type eventServiceClient struct {
	cc grpc.ClientConnInterface
}

func NewEventServiceClient(cc grpc.ClientConnInterface) EventServiceClient {
	return &eventServiceClient{cc}
}

func (c *eventServiceClient) Collect(ctx context.Context, opts ...grpc.CallOption) (EventService_CollectClient, error) {
	stream, err := c.cc.NewStream(ctx, &EventService_ServiceDesc.Streams[0], "/skywalking.v3.EventService/collect", opts...)
	if err != nil {
		return nil, err
	}
	x := &eventServiceCollectClient{stream}
	return x, nil
}

type EventService_CollectClient interface {
	Send(*Event) error
	CloseAndRecv() (*v3.Commands, error)
	grpc.ClientStream
}

type eventServiceCollectClient struct {
	grpc.ClientStream
}

func (x *eventServiceCollectClient) Send(m *Event) error {
	return x.ClientStream.SendMsg(m)
}

func (x *eventServiceCollectClient) CloseAndRecv() (*v3.Commands, error) {
	if err := x.ClientStream.CloseSend(); err != nil {
		return nil, err
	}
	m := new(v3.Commands)
	if err := x.ClientStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

// EventServiceServer is the server API for EventService service.
// All implementations must embed UnimplementedEventServiceServer
// for forward compatibility
type EventServiceServer interface {
	// When reporting an event, you typically call the collect function twice, one for starting of the event and the other one for ending of the event, with the same UUID.
	// There are also cases where you have both start time and end time already, for example, when exporting events from a 3rd-party system,
	// the start time and end time are already known so that you can call the collect function only once.
	Collect(EventService_CollectServer) error
	mustEmbedUnimplementedEventServiceServer()
}

// UnimplementedEventServiceServer must be embedded to have forward compatible implementations.
type UnimplementedEventServiceServer struct {
}

func (UnimplementedEventServiceServer) Collect(EventService_CollectServer) error {
	return status.Errorf(codes.Unimplemented, "method Collect not implemented")
}
func (UnimplementedEventServiceServer) mustEmbedUnimplementedEventServiceServer() {}

// UnsafeEventServiceServer may be embedded to opt out of forward compatibility for this service.
// Use of this interface is not recommended, as added methods to EventServiceServer will
// result in compilation errors.
type UnsafeEventServiceServer interface {
	mustEmbedUnimplementedEventServiceServer()
}

func RegisterEventServiceServer(s grpc.ServiceRegistrar, srv EventServiceServer) {
	s.RegisterService(&EventService_ServiceDesc, srv)
}

func _EventService_Collect_Handler(srv interface{}, stream grpc.ServerStream) error {
	return srv.(EventServiceServer).Collect(&eventServiceCollectServer{stream})
}

type EventService_CollectServer interface {
	SendAndClose(*v3.Commands) error
	Recv() (*Event, error)
	grpc.ServerStream
}

type eventServiceCollectServer struct {
	grpc.ServerStream
}

func (x *eventServiceCollectServer) SendAndClose(m *v3.Commands) error {
	return x.ServerStream.SendMsg(m)
}

func (x *eventServiceCollectServer) Recv() (*Event, error) {
	m := new(Event)
	if err := x.ServerStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

// EventService_ServiceDesc is the grpc.ServiceDesc for EventService service.
// It's only intended for direct use with grpc.RegisterService,
// and not to be introspected or modified (even as a copy)
var EventService_ServiceDesc = grpc.ServiceDesc{
	ServiceName: "skywalking.v3.EventService",
	HandlerType: (*EventServiceServer)(nil),
	Methods:     []grpc.MethodDesc{},
	Streams: []grpc.StreamDesc{
		{
			StreamName:    "collect",
			Handler:       _EventService_Collect_Handler,
			ClientStreams: true,
		},
	},
	Metadata: "event/Event.proto",
}
