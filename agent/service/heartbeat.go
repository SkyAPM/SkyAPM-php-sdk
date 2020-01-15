package service

import (
	"agent/agent/pb/agent"
	"agent/agent/pb/register2"
	"context"
	"time"
)

func (t *Agent) heartbeat() {

	t.registerCache.Range(func(key, value interface{}) bool {
		log.Infoln("heartbeat")
		bind := value.(registerCache)

		if bind.Version == 5 {
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			_, err := t.grpcClient.pingClient5.Heartbeat(ctx, &agent.ApplicationInstanceHeartbeat{
				ApplicationInstanceId: bind.InstanceId,
				HeartbeatTime:         time.Now().UnixNano() / 1000000,
			})

			if err != nil {
				log.Error("heartbeat:", err)
			} else {
				log.Infof("heartbeat appId %d appInsId %d", bind.AppId, bind.InstanceId)
			}

		} else if bind.Version == 6 {
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			_, err := t.grpcClient.pintClient6.DoPing(ctx, &register2.ServiceInstancePingPkg{
				ServiceInstanceId:   bind.InstanceId,
				Time:                time.Now().UnixNano() / 1000000,
				ServiceInstanceUUID: bind.Uuid,
			})
			if err != nil {
				log.Error("heartbeat:", err)
			} else {
				log.Infof("heartbeat appId %d appInsId %d", bind.AppId, bind.InstanceId)
			}
		}
		return true
	})
}
