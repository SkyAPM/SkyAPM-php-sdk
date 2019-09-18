package service

import (
	"agent/agent/pb/agent"
	"agent/agent/pb/register2"
	"context"
	"fmt"
	"time"
)

func (t *Agent) heartbeat() {
	var client5 agent.InstanceDiscoveryServiceClient
	var client6 register2.ServiceInstancePingClient

	t.registerCache.Range(func(key, value interface{}) bool {
		fmt.Println("heartbeat => ...")
		bind := value.(registerCache)

		if bind.Version == 5 {
			if client5 == nil {
				client5 = agent.NewInstanceDiscoveryServiceClient(t.conn)
			}

			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			_, err := client5.Heartbeat(ctx, &agent.ApplicationInstanceHeartbeat{
				ApplicationInstanceId: bind.InstanceId,
				HeartbeatTime:         time.Now().UnixNano() / 1000000,
			})

			if err != nil {
				fmt.Println("heartbeat =>", err)
			} else {
				fmt.Printf("heartbeat => %d %d\n", bind.AppId, bind.InstanceId)
			}

		} else if bind.Version == 6 {
			if client6 == nil {
				client6 = register2.NewServiceInstancePingClient(t.conn)
			}

			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			_, err := client6.DoPing(ctx, &register2.ServiceInstancePingPkg{
				ServiceInstanceId:   bind.InstanceId,
				Time:                time.Now().UnixNano() / 1000000,
				ServiceInstanceUUID: bind.Uuid,
			})
			if err != nil {
				fmt.Println("heartbeat =>", err)
			} else {
				fmt.Printf("heartbeat => %d %d\n", bind.AppId, bind.InstanceId)
			}
		}
		return true
	})
}
