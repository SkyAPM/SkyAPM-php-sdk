package service

import (
	"agent/agent/pb/agent"
	"agent/agent/pb/agent2"
	"agent/agent/pb/common"
	"context"
	"encoding/json"
	"github.com/golang/protobuf/proto"
	"io"
	"strconv"
	"strings"
	"time"
)

type upstreamSegment struct {
	Version int
	segment *agent.UpstreamSegment
}

type trace struct {
	ApplicationInstance int32    `json:"application_instance"`
	Pid                 int      `json:"pid"`
	ApplicationId       int32    `json:"application_id"`
	Uuid                string   `json:"uuid"`
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

func (t *Agent) send(segments []*upstreamSegment) {
	var err error

	log.Infof("start sending trace..., count %d", len(segments))

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel()
	streamV5, err := t.grpcClient.segmentClientV5.Collect(ctx)
	if err != nil {
		log.Warningln(err)
	}

	ctx6, cancel6 := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel6()
	streamV6, err := t.grpcClient.segmentClientV6.Collect(ctx6)
	if err != nil {
		log.Warningln(err)
	}

	if streamV5 == nil && streamV6 == nil {
		log.Error("no stream available")
	}

	for _, segment := range segments {
		if segment.Version == 5 {
			if streamV5 != nil {
				if err = streamV5.Send(segment.segment); err != nil {
					if err == io.EOF {
						log.Warn(err)
					} else {
						log.Error(err)
					}

				}
			} else {
				log.Warn("stream not open, sending fail")
			}

		} else if segment.Version == 6 {
			if streamV6 != nil {
				if err = streamV6.Send(segment.segment); err != nil {
					if err == io.EOF {
						log.Warn(err)
					} else {
						log.Error(err)
					}
				}
			} else {
				log.Warn("stream not open, sending fail")
			}
		}
	}
	if streamV5 != nil {
		streamV5.CloseAndRecv()
	}
	if streamV6 == nil {
		streamV6.CloseAndRecv()
	}
	log.Info("sending success...")
}

func format(j string) *upstreamSegment {
	info := trace{}
	err := json.Unmarshal([]byte(j), &info)

	if err != nil {
		log.Error("trace json decode:", err)
		return nil
	}
	if info.Version == 5 {
		var globalTrace []*agent.UniqueId

		for _, v := range info.GlobalTraceIds {
			globalTrace = append(globalTrace, buildUniqueId(v))
		}

		var spans []*agent.SpanObject

		for _, v := range info.Segment.Spans {
			span := &agent.SpanObject{
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
				span.SpanType = agent.SpanType_Entry
			} else if v.SpanType == 1 {
				span.SpanType = agent.SpanType_Exit
			} else if v.SpanType == 2 {
				span.SpanType = agent.SpanType_Local
			}

			if v.SpanLayer == 3 {
				span.SpanLayer = agent.SpanLayer_Http
			} else if v.SpanLayer == 1 {
				span.SpanLayer = agent.SpanLayer_Database
			}

			buildTags(span, v.Tags)
			buildRefs(span, v.Refs)

			spans = append(spans, span)
		}

		segmentObject := &agent.TraceSegmentObject{
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
		//log.Info(seg)
		if err != nil {
			log.Error("trace json encode:", err)
			return nil
		}

		segment := &agent.UpstreamSegment{
			GlobalTraceIds: globalTrace,
			Segment:        seg,
		}
		return &upstreamSegment{
			Version: info.Version,
			segment: segment,
		}
	} else if info.Version == 6 {

		var globalTrace []*agent.UniqueId

		for _, v := range info.GlobalTraceIds {
			globalTrace = append(globalTrace, buildUniqueId(v))
		}

		var spans []*agent2.SpanObjectV2

		for _, v := range info.Segment.Spans {
			span := &agent2.SpanObjectV2{
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
				span.SpanType = agent.SpanType_Entry
			} else if v.SpanType == 1 {
				span.SpanType = agent.SpanType_Exit
			} else if v.SpanType == 2 {
				span.SpanType = agent.SpanType_Local
			}

			if v.SpanLayer == 3 {
				span.SpanLayer = agent.SpanLayer_Http
			} else if v.SpanLayer == 1 {
				span.SpanLayer = agent.SpanLayer_Database
			}

			buildTags6(span, v.Tags)
			buildRefs6(span, v.Refs)

			spans = append(spans, span)
		}

		segmentObject := &agent2.SegmentObject{
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
			return nil
		}

		segment := &agent.UpstreamSegment{
			GlobalTraceIds: globalTrace,
			Segment:        seg,
		}

		return &upstreamSegment{
			Version: info.Version,
			segment: segment,
		}
	}
	return nil
}

func buildRefs(span *agent.SpanObject, refs []ref) {
	// refs
	var spanRefs []*agent.TraceSegmentReference

	for _, rev := range refs {
		var refType agent.RefType
		if rev.Type == 0 {
			refType = agent.RefType_CrossProcess
		}

		spanRefs = append(spanRefs, &agent.TraceSegmentReference{
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

func buildRefs6(span *agent2.SpanObjectV2, refs []ref) {
	// refs
	var spanRefs []*agent2.SegmentReference

	for _, rev := range refs {
		var refType agent.RefType
		if rev.Type == 0 {
			refType = agent.RefType_CrossProcess
		}

		var reference = &agent2.SegmentReference{
			RefType:                 refType,
			ParentTraceSegmentId:    buildUniqueId(rev.ParentTraceSegmentId),
			ParentSpanId:            rev.ParentSpanId,
			ParentServiceInstanceId: rev.ParentApplicationInstanceId,
			EntryServiceInstanceId:  rev.EntryApplicationInstanceId,
		}

		if rev.NetworkAddress[0:1] == "#" {
			reference.NetworkAddress = rev.NetworkAddress
		} else {
			i, _ := strconv.ParseInt(rev.NetworkAddress, 10, 64)
			reference.NetworkAddressId = int32(i)
		}

		if rev.EntryServiceName[0:1] == "#" {
			reference.EntryEndpoint = rev.EntryServiceName
		} else {
			i, _ := strconv.ParseInt(rev.EntryServiceName, 10, 64)
			reference.EntryEndpointId = int32(i)
		}

		if rev.ParentServiceName[0:1] == "#" {
			reference.ParentEndpoint = rev.ParentServiceName
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

func buildUniqueId(str string) *agent.UniqueId {
	uniqueId := &agent.UniqueId{}
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

func buildTags(span *agent.SpanObject, t map[string]string) {
	// tags
	var tags []*agent.KeyWithStringValue

	for k, v := range t {
		kv := &agent.KeyWithStringValue{
			Key:   k,
			Value: v,
		}
		tags = append(tags, kv)
	}

	if len(tags) > 0 {
		span.Tags = tags
	}
}

func buildTags6(span *agent2.SpanObjectV2, t map[string]string) {
	// tags
	var tags []*common.KeyStringValuePair

	for k, v := range t {
		kv := &common.KeyStringValuePair{
			Key:   k,
			Value: v,
		}
		tags = append(tags, kv)
	}

	if len(tags) > 0 {
		span.Tags = tags
	}
}
