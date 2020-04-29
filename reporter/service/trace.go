package service

import (
	"context"
	"encoding/json"
	nc2 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/common/v2"
	nc3 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/common/v3"
	nla1 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v1"
	nla2 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v2"
	nla3 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v3"
	"io"
	"strconv"
	"strings"
	"time"

	"github.com/golang/protobuf/proto"
)

type upstreamSegment struct {
	Version int
	segment interface{}
}

type trace struct {
	ApplicationInstance int32    `json:"application_instance"`
	Pid                 int      `json:"pid"`
	ApplicationId       int32    `json:"application_id"`
	Uuid                string   `json:"uuid"`
	Version             int      `json:"version"`
	Segment             segment  `json:"segment"`
	GlobalTraceIds      []string `json:"globalTraceIds"`

	// sw8
	TraceId         string `json:"traceId"`
	Service         string `json:"service"`
	ServiceInstance string `json:"serviceInstance"`
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

	//	sw8
	TraceId               string `json:"traceId"`
	ParentService         string `json:"parentService"`
	ParentServiceInstance string `json:"parentServiceInstance"`
	ParentEndpoint        string `json:"parentEndpoint"`
	TargetAddress         string `json:"targetAddress"`
}

func (t *Agent) send(segments []*upstreamSegment) {
	var err error

	log.Infof("start sending trace..., count %d", len(segments))

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel()

	var tsrsc1 nla1.TraceSegmentService_CollectClient
	var tsrsc2 nla2.TraceSegmentReportService_CollectClient
	var tsrsc3 nla3.TraceSegmentReportService_CollectClient
	if t.version == 5 {
		tsrsc1, err = t.grpcClient.tssc1.Collect(ctx)
	} else if t.version == 6 || t.version == 7 {
		tsrsc2, err = t.grpcClient.tsrsc2.Collect(ctx)
	} else if t.version == 8 {
		tsrsc3, err = t.grpcClient.tsrsc3.Collect(ctx)
	}

	if err != nil {
		log.Warningln(err)
	}

	if tsrsc1 == nil && tsrsc2 == nil && tsrsc3 == nil {
		log.Error("no stream available")
	}

	for _, segment := range segments {
		var err error

		if t.version == 5 {
			err = tsrsc1.Send(segment.segment.(*nla1.UpstreamSegment))
		} else if t.version == 6 || t.version == 7 {
			err = tsrsc2.Send(segment.segment.(*nla2.UpstreamSegment))
		} else if t.version == 8 {
			err = tsrsc3.Send(segment.segment.(*nla3.SegmentObject))
		}

		if err != nil {
			if err == io.EOF {
				log.Warn(err)
			} else {
				log.Error(err)
			}
		}
	}

	if t.version == 5 {
		tsrsc1.CloseAndRecv()
	} else if t.version == 6 || t.version == 7 {
		tsrsc2.CloseAndRecv()
	} else if t.version == 8 {
		tsrsc3.CloseAndRecv()
	}

	log.Info("✅ sending success")
}

func format(version int, j string) (trace, *upstreamSegment) {
	info := trace{}
	err := json.Unmarshal([]byte(j), &info)

	if err != nil {
		log.Error("trace json decode:", err)
		return info, nil
	}

	if version == 5 {
		var globalTrace []*nla1.UniqueId

		for _, v := range info.GlobalTraceIds {
			globalTrace = append(globalTrace, buildUniqueId1(v))
		}

		var spans []*nla1.SpanObject

		for _, v := range info.Segment.Spans {
			span := &nla1.SpanObject{
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
				span.SpanType = nla1.SpanType_Entry
			} else if v.SpanType == 1 {
				span.SpanType = nla1.SpanType_Exit
			} else if v.SpanType == 2 {
				span.SpanType = nla1.SpanType_Local
			}

			if v.SpanLayer == 3 {
				span.SpanLayer = nla1.SpanLayer_Http
			} else if v.SpanLayer == 1 {
				span.SpanLayer = nla1.SpanLayer_Database
			}

			buildTags(span, v.Tags)
			buildRefs(span, v.Refs)

			spans = append(spans, span)
		}

		segmentObject := &nla1.TraceSegmentObject{
			TraceSegmentId:        buildUniqueId1(info.Segment.TraceSegmentId),
			Spans:                 spans,
			ApplicationId:         info.ApplicationId,
			ApplicationInstanceId: info.ApplicationInstance,
			IsSizeLimited:         info.Segment.IsSizeLimited != 0,
		}
		//m := jsonpb.Marshaler{
		//	EnumsAsInts:  true,
		//}
		seg, err := proto.Marshal(segmentObject)
		//log.Info(seg)
		if err != nil {
			log.Error("trace proto encode:", err)
			return info, nil
		}

		segment := &nla1.UpstreamSegment{
			GlobalTraceIds: globalTrace,
			Segment:        seg,
		}

		return info, &upstreamSegment{
			Version: info.Version,
			segment: segment,
		}
	} else if version == 6 || version == 7 {

		var globalTrace []*nla2.UniqueId

		for _, v := range info.GlobalTraceIds {
			globalTrace = append(globalTrace, buildUniqueId(v))
		}

		var spans []*nla2.SpanObjectV2

		for _, v := range info.Segment.Spans {
			span := &nla2.SpanObjectV2{
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
				span.SpanType = nla2.SpanType_Entry
			} else if v.SpanType == 1 {
				span.SpanType = nla2.SpanType_Exit
			} else if v.SpanType == 2 {
				span.SpanType = nla2.SpanType_Local
			}

			if v.SpanLayer == 3 {
				span.SpanLayer = nla2.SpanLayer_Http
			} else if v.SpanLayer == 1 {
				span.SpanLayer = nla2.SpanLayer_Database
			}

			buildTags2(span, v.Tags)
			buildRefs2(span, v.Refs)

			spans = append(spans, span)
		}

		segmentObject := &nla2.SegmentObject{
			TraceSegmentId:    buildUniqueId(info.Segment.TraceSegmentId),
			Spans:             spans,
			ServiceId:         info.ApplicationId,
			ServiceInstanceId: info.ApplicationInstance,
			IsSizeLimited:     info.Segment.IsSizeLimited != 0,
		}
		//m := jsonpb.Marshaler{
		//	EnumsAsInts:  true,
		//}
		seg, err := proto.Marshal(segmentObject)
		//log.Info(seg)
		if err != nil {
			log.Error("trace proto encode:", err)
			return info, nil
		}

		segment := &nla2.UpstreamSegment{
			GlobalTraceIds: globalTrace,
			Segment:        seg,
		}

		return info, &upstreamSegment{
			Version: info.Version,
			segment: segment,
		}
	} else if version == 8 {

		var spans []*nla3.SpanObject

		for _, v := range info.Segment.Spans {
			span := &nla3.SpanObject{
				SpanId:        v.SpanId,
				ParentSpanId:  v.ParentSpanId,
				StartTime:     v.StartTime,
				EndTime:       v.EndTime,
				OperationName: v.OperationName,
				Peer:          v.Peer,
				ComponentId:   v.ComponentId,
				IsError:       v.IsError != 0,
			}

			if v.SpanType == 0 {
				span.SpanType = nla3.SpanType_Entry
			} else if v.SpanType == 1 {
				span.SpanType = nla3.SpanType_Exit
			} else if v.SpanType == 2 {
				span.SpanType = nla3.SpanType_Local
			}

			if v.SpanLayer == 3 {
				span.SpanLayer = nla3.SpanLayer_Http
			} else if v.SpanLayer == 1 {
				span.SpanLayer = nla3.SpanLayer_Database
			}

			buildTags3(span, v.Tags)
			buildRefs3(span, v.Refs)

			spans = append(spans, span)
		}

		segmentObject := &nla3.SegmentObject{
			TraceId:         info.TraceId,
			TraceSegmentId:  info.Segment.TraceSegmentId,
			Spans:           spans,
			Service:         info.Service,
			ServiceInstance: info.ServiceInstance,
			IsSizeLimited:   info.Segment.IsSizeLimited != 0,
		}

		return info, &upstreamSegment{
			Version: info.Version,
			segment: segmentObject,
		}
	}
	return info, nil
}

func buildRefs2(span *nla2.SpanObjectV2, refs []ref) {
	// refs
	var spanRefs []*nla2.SegmentReference

	for _, rev := range refs {
		var refType nla2.RefType
		if rev.Type == 0 {
			refType = nla2.RefType_CrossProcess
		}

		var reference = &nla2.SegmentReference{
			RefType:                 refType,
			ParentTraceSegmentId:    buildUniqueId(rev.ParentTraceSegmentId),
			ParentSpanId:            rev.ParentSpanId,
			ParentServiceInstanceId: rev.ParentApplicationInstanceId,
			EntryServiceInstanceId:  rev.EntryApplicationInstanceId,
		}

		if rev.NetworkAddress[0:1] == "#" {
			reference.NetworkAddress = rev.NetworkAddress[1:]
		} else {
			i, _ := strconv.ParseInt(rev.NetworkAddress, 10, 64)
			reference.NetworkAddressId = int32(i)
		}

		if rev.EntryServiceName[0:1] == "#" {
			reference.EntryEndpoint = rev.EntryServiceName[1:]
		} else {
			i, _ := strconv.ParseInt(rev.EntryServiceName, 10, 64)
			reference.EntryEndpointId = int32(i)
		}

		if rev.ParentServiceName[0:1] == "#" {
			reference.ParentEndpoint = rev.ParentServiceName[1:]
		} else {
			i, _ := strconv.ParseInt(rev.ParentServiceName, 10, 64)
			reference.ParentEndpointId = int32(i)
		}

		spanRefs = append(spanRefs, reference)
	}

	if len(spanRefs) > 0 {
		span.Refs = spanRefs
	}
}

func buildRefs(span *nla1.SpanObject, refs []ref) {
	// refs
	var spanRefs []*nla1.TraceSegmentReference

	for _, rev := range refs {
		var refType nla1.RefType
		if rev.Type == 0 {
			refType = nla1.RefType_CrossProcess
		}

		spanRefs = append(spanRefs, &nla1.TraceSegmentReference{
			RefType:                     refType,
			ParentTraceSegmentId:        buildUniqueId1(rev.ParentTraceSegmentId),
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

func buildTags(span *nla1.SpanObject, t map[string]string) {
	// tags
	var tags []*nla1.KeyWithStringValue

	for k, v := range t {
		kv := &nla1.KeyWithStringValue{
			Key:   k,
			Value: v,
		}
		tags = append(tags, kv)
	}

	if len(tags) > 0 {
		span.Tags = tags
	}
}

func buildRefs3(span *nla3.SpanObject, refs []ref) {
	// refs
	var spanRefs []*nla3.SegmentReference

	for _, rev := range refs {
		var refType nla3.RefType
		if rev.Type == 0 {
			refType = nla3.RefType_CrossProcess
		}

		var reference = &nla3.SegmentReference{
			RefType:                  refType,
			TraceId:                  rev.TraceId,
			ParentTraceSegmentId:     rev.ParentTraceSegmentId,
			ParentSpanId:             rev.ParentSpanId,
			ParentService:            rev.ParentService,
			ParentServiceInstance:    rev.ParentServiceInstance,
			ParentEndpoint:           rev.ParentEndpoint,
			NetworkAddressUsedAtPeer: rev.NetworkAddress,
		}

		spanRefs = append(spanRefs, reference)
	}

	if len(spanRefs) > 0 {
		span.Refs = spanRefs
	}
}

func buildUniqueId1(str string) *nla1.UniqueId {
	uniqueId := &nla1.UniqueId{}
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

func buildUniqueId(str string) *nla2.UniqueId {
	uniqueId := &nla2.UniqueId{}
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

func buildTags2(span *nla2.SpanObjectV2, t map[string]string) {
	// tags
	var tags []*nc2.KeyStringValuePair

	for k, v := range t {
		kv := &nc2.KeyStringValuePair{
			Key:   k,
			Value: v,
		}
		tags = append(tags, kv)
	}

	if len(tags) > 0 {
		span.Tags = tags
	}
}

func buildTags3(span *nla3.SpanObject, t map[string]string) {
	// tags
	var tags []*nc3.KeyStringValuePair

	for k, v := range t {
		kv := &nc3.KeyStringValuePair{
			Key:   k,
			Value: v,
		}
		tags = append(tags, kv)
	}

	if len(tags) > 0 {
		span.Tags = tags
	}
}
