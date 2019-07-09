package service

import (
	"agent/agent/pb5"
	"agent/agent/pb6/agent"
	"context"
	"encoding/json"
	"fmt"
	"github.com/golang/protobuf/proto"
	"google.golang.org/grpc"
	"log"
	"strconv"
	"strings"
	"time"
)

type trace struct {
	ApplicationInstance int32    `json:"application_instance"`
	Pid                 int      `json:"pid"`
	ApplicationId       int32    `json:"application_id"`
	Version             int      `json:"version"`
	Segment             segment  `json:"segment"`
	GlobalTraceIds      []string `json:"globalTraceIds"`
}

type segment struct {
	TraceSegmentId string `json:"traceSegmentId"`
	IsSizeLimited  int    `json:"isSizeLimited"`
	Spans          []span `json:"spans"`
}

type span struct {
	Tags          map[string]string `json:"tags"`
	SpanId        int32             `json:"spanId"`
	ParentSpanId  int32             `json:"parentSpanId"`
	StartTime     int64             `json:"startTime"`
	OperationName string            `json:"operationName"`
	Peer          string            `json:"peer"`
	SpanType      int32             `json:"spanType"`
	SpanLayer     int32             `json:"spanLayer"`
	ComponentId   int32             `json:"componentId"`
	ComponentName string            `json:"component"`
	Refs          []ref             `json:"refs"`
	EndTime       int64             `json:"endTime"`
	IsError       int               `json:"isError"`
}

type ref struct {
	Type                        int32  `json:"type"`
	ParentTraceSegmentId        string `json:"parentTraceSegmentId"`
	ParentSpanId                int32  `json:"parentSpanId"`
	ParentApplicationInstanceId int32  `json:"parentApplicationInstanceId"`
	NetworkAddress              string `json:"networkAddress"`
	EntryApplicationInstanceId  int32  `json:"entryApplicationInstanceId"`
	EntryServiceName            string `json:"entryServiceName"`
	ParentServiceName           string `json:"parentServiceName"`
}

func SendTrace(conn *grpc.ClientConn, j string) {
	info := trace{}
	err := json.Unmarshal([]byte(j), &info)

	if err != nil {
		log.Println("trace => ", err)
		return
	}
	if info.Version == 5 {
		log.Println("trace => Start trace...")
		c := pb5.NewTraceSegmentServiceClient(conn)
		ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
		defer cancel()

		client, err := c.Collect(ctx)
		if err != nil {
			log.Println("trace => ", err)
			return
		}

		var globalTrace []*pb5.UniqueId

		for _, v := range info.GlobalTraceIds {
			globalTrace = append(globalTrace, buildUniqueId(v))
		}

		var spans []*pb5.SpanObject

		for _, v := range info.Segment.Spans {
			span := &pb5.SpanObject{
				SpanId:        v.SpanId,
				ParentSpanId:  v.ParentSpanId,
				StartTime:     v.StartTime,
				EndTime:       v.EndTime,
				OperationName: v.OperationName,
				Peer:          v.Peer,
				Component:     v.ComponentName,
				IsError:       v.IsError != 0,
			}

			if v.ComponentId != 0 {
				span.ComponentId = v.ComponentId
			}

			if v.SpanType == 0 {
				span.SpanType = pb5.SpanType_Entry
			} else if v.SpanType == 1 {
				span.SpanType = pb5.SpanType_Exit
			} else if v.SpanType == 2 {
				span.SpanType = pb5.SpanType_Local
			}

			if v.SpanLayer == 3 {
				span.SpanLayer = pb5.SpanLayer_Http
			} else if v.SpanLayer == 1 {
				span.SpanLayer = pb5.SpanLayer_Database
			}

			buildTags(span, v.Tags)
			buildRefs(span, v.Refs)

			spans = append(spans, span)
		}

		segmentObject := &pb5.TraceSegmentObject{
			TraceSegmentId:        buildUniqueId(info.Segment.TraceSegmentId),
			Spans:                 spans,
			ApplicationId:         info.ApplicationId,
			ApplicationInstanceId: info.ApplicationInstance,
			IsSizeLimited:         info.Segment.IsSizeLimited != 0,
		}
		//m := jsonpb.Marshaler{
		//	EnumsAsInts:  true,
		//}
		seg, err := proto.Marshal(segmentObject)
		//fmt.Println(seg)
		if err != nil {
			log.Println("trace => ", err)
			return
		}

		segment := &pb5.UpstreamSegment{
			GlobalTraceIds: globalTrace,
			Segment:        seg,
		}

		err = client.Send(segment)
		if err != nil {
			log.Println("trace => ", err)
			return
		}

		_, err = client.CloseAndRecv()
		if err != nil {
			log.Println("trace =>", err)
		}
		log.Println("trace => send ok")
	} else if info.Version == 6 {
		log.Println("trace => Start trace...")
		c := agent.NewTraceSegmentReportServiceClient(conn)
		ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
		defer cancel()

		client, err := c.Collect(ctx)
		if err != nil {
			log.Println("trace => ", err)
			return
		}
		fmt.Println(client)
		//
		//var globalTrace []*pb6.UniqueId
		//
		//for _, v := range info.GlobalTraceIds {
		//	globalTrace = append(globalTrace, buildUniqueId6(v))
		//}
		//
		//var spans []*pb6.SpanObjectV2
		//
		//for _, v := range info.Segment.Spans {
		//	span := &pb6.SpanObjectV2{
		//		SpanId:        v.SpanId,
		//		ParentSpanId:  v.ParentSpanId,
		//		StartTime:     v.StartTime,
		//		EndTime:       v.EndTime,
		//		OperationName: v.OperationName,
		//		Peer:          v.Peer,
		//		Component:     v.ComponentName,
		//		IsError:       v.IsError != 0,
		//	}
		//
		//	if v.ComponentId != 0 {
		//		span.ComponentId = v.ComponentId
		//	}
		//
		//	if v.SpanType == 0 {
		//		span.SpanType = pb6.SpanType_Entry
		//	} else if v.SpanType == 1 {
		//		span.SpanType = pb6.SpanType_Exit
		//	} else if v.SpanType == 2 {
		//		span.SpanType = pb6.SpanType_Local
		//	}
		//
		//	if v.SpanLayer == 3 {
		//		span.SpanLayer = pb6.SpanLayer_Http
		//	} else if v.SpanLayer == 1 {
		//		span.SpanLayer = pb6.SpanLayer_Database
		//	}
		//
		//	buildTags6(span, v.Tags)
		//	buildRefs6(span, v.Refs)
		//
		//	spans = append(spans, span)
		//}
		//
		//segmentObject := &pb6.SegmentObject{
		//	TraceSegmentId:    buildUniqueId6(info.Segment.TraceSegmentId),
		//	Spans:             spans,
		//	ServiceId:         info.ApplicationId,
		//	ServiceInstanceId: info.ApplicationInstance,
		//	IsSizeLimited:     info.Segment.IsSizeLimited != 0,
		//}
		////m := jsonpb.Marshaler{
		////	EnumsAsInts:  true,
		////}
		//seg, err := proto.Marshal(segmentObject)
		////fmt.Println(seg)
		//if err != nil {
		//	log.Println("trace => ", err)
		//	return
		//}
		//
		//segment := &pb6.UpstreamSegment{
		//	GlobalTraceIds: globalTrace,
		//	Segment:        seg,
		//}
		//
		//err = client.Send(segment)
		//if err != nil {
		//	log.Println("trace => ", err)
		//	return
		//}
		//
		//_, err = client.CloseAndRecv()
		//if err != nil {
		//	log.Println("trace =>", err)
		//}
		//log.Println("trace => send ok")
	}
}

func buildRefs(span *pb5.SpanObject, refs []ref) {
	// refs
	var spanRefs []*pb5.TraceSegmentReference

	for _, rev := range refs {
		var refType pb5.RefType
		if rev.Type == 0 {
			refType = pb5.RefType_CrossProcess
		}

		spanRefs = append(spanRefs, &pb5.TraceSegmentReference{
			RefType:                     refType,
			ParentTraceSegmentId:        buildUniqueId(rev.ParentTraceSegmentId),
			ParentSpanId:                rev.ParentSpanId,
			ParentApplicationInstanceId: rev.ParentApplicationInstanceId,
			NetworkAddress:              rev.NetworkAddress,
			EntryApplicationInstanceId:  rev.EntryApplicationInstanceId,
			EntryServiceName:            rev.EntryServiceName,
			ParentServiceName:           rev.ParentServiceName,
		})
	}

	if len(spanRefs) > 0 {
		span.Refs = spanRefs
	}
}

//func buildRefs6(span *pb6.SpanObjectV2, refs []ref) {
//	// refs
//	var spanRefs []*pb6.SegmentReference
//
//	for _, rev := range refs {
//		var refType pb6.RefType
//		if rev.Type == 0 {
//			refType = pb6.RefType_CrossProcess
//		}
//
//		spanRefs = append(spanRefs, &pb6.SegmentReference{
//			RefType:                 refType,
//			ParentTraceSegmentId:    buildUniqueId6(rev.ParentTraceSegmentId),
//			ParentSpanId:            rev.ParentSpanId,
//			ParentServiceInstanceId: rev.ParentApplicationInstanceId,
//			NetworkAddress:          rev.NetworkAddress,
//			EntryServiceInstanceId:  rev.EntryApplicationInstanceId,
//			EntryEndpoint:           rev.EntryServiceName,
//			ParentEndpoint:          rev.ParentServiceName,
//		})
//	}
//
//	if len(spanRefs) > 0 {
//		span.Refs = spanRefs
//	}
//}

func buildUniqueId(str string) *pb5.UniqueId {
	uniqueId := &pb5.UniqueId{}
	var ids []int64
	for _, idStr := range strings.Split(str, ".") {
		id, err := strconv.ParseInt(idStr, 10, 64)
		if err != nil {
			log.Println("trace => ", err)
			panic(err)
		}
		ids = append(ids, id)
	}

	uniqueId.IdParts = ids
	return uniqueId
}

//func buildUniqueId6(str string) *pb6.UniqueId {
//	uniqueId := &pb6.UniqueId{}
//	var ids []int64
//	for _, idStr := range strings.Split(str, ".") {
//		id, err := strconv.ParseInt(idStr, 10, 64)
//		if err != nil {
//			log.Println("trace => ", err)
//			panic(err)
//		}
//		ids = append(ids, id)
//	}
//
//	uniqueId.IdParts = ids
//	return uniqueId
//}

func buildTags(span *pb5.SpanObject, t map[string]string) {
	// tags
	var tags []*pb5.KeyWithStringValue

	for k, v := range t {
		kv := &pb5.KeyWithStringValue{
			Key:   k,
			Value: v,
		}
		tags = append(tags, kv)
	}

	if len(tags) > 0 {
		span.Tags = tags
	}
}

//func buildTags6(span *pb6.SpanObjectV2, t map[string]string) {
//	// tags
//	var tags []*pb6.KeyStringValuePair
//
//	for k, v := range t {
//		kv := &pb6.KeyStringValuePair{
//			Key:   k,
//			Value: v,
//		}
//		tags = append(tags, kv)
//	}
//
//	if len(tags) > 0 {
//		span.Tags = tags
//	}
//}
