package service

import (
	nla1 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/language/agent/v1"
	v3 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/management/v3"
	nr2 "github.com/SkyAPM/SkyAPM-php-sdk/reporter/network/register/v2"
	"syscall"
	"time"
)
import "context"

func (t *Agent) heartbeat() {

	var heartList []registerCache
	t.registerCacheLock.Lock()
	for pid, bind := range t.registerCache {
		if err := syscall.Kill(pid, 0); err != nil {
			delete(t.registerCache, pid)
			continue
		}
		heartList = append(heartList, bind)
	}
	t.registerCacheLock.Unlock()

	for _, bind := range heartList {
		if t.version == 5 {
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			_, err := t.grpcClient.idsc1.Heartbeat(ctx, &nla1.ApplicationInstanceHeartbeat{
				ApplicationInstanceId: bind.InstanceId,
				HeartbeatTime:         time.Now().UnixNano() / 1000000,
			})
			if err != nil {
				log.Error("heartbeat:", err)
			} else {
				log.Infof("heartbeat appId %d appInsId %d", bind.AppId, bind.InstanceId)
			}
		} else if t.version == 6 || t.version == 7 {
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			_, err := t.grpcClient.sipc2.DoPing(ctx, &nr2.ServiceInstancePingPkg{
				ServiceInstanceId:   bind.InstanceId,
				Time:                time.Now().UnixNano() / 1000000,
				ServiceInstanceUUID: bind.Uuid,
			})
			if err != nil {
				log.Error("heartbeat:", err)
			} else {
				log.Infof("heartbeat appId %d appInsId %d", bind.AppId, bind.InstanceId)
			}
		} else if t.version == 8 {
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			defer cancel()

			_, err := t.grpcClient.msc3.KeepAlive(ctx, &v3.InstancePingPkg{
				Service:         bind.Service,
				ServiceInstance: bind.ServiceInstance,
			})

			if err != nil {
				log.Error("heartbeat:", err)
			} else {
				log.Infof("heartbeat service %s, service instance %s", bind.Service, bind.ServiceInstance)
			}
		}
	}
}
