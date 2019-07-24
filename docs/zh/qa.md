# 常见问题
 -----
1. linux下php不能读取/tmp文件夹下文件，即php无法与/tmp目录下的sock文件进行通信。  
该问题一般是由`systemd`服务的`PrivateTmp`属性配置导致的。  
`systemd`中服务的配置文件都在目录`/lib/systemd/system/`中：  
 * 检查SELinux配置，可尝试关闭SELinux
 * `apache`的配置文件为`apache2.service`
 * `php-fpm`的配置文件为`php-fpm.service` (可能会存在版本号，例如`php7.2-fpm.service`)  
 
 修改`PrivateTmp`属性的取值为`false`, 再执行`sudo systemctl daemon-reload`, 最后再重启`apache`或者`php-fpm`

1. 对于已经安装并启用了`SkyWalking`扩展的PHP环境的主机，所发送的trace信息在哪里？
 * 如果`skywalking.version=5`, 则`http`请求的`header`中会加入一个名为`SW3`的数据。PHP可使用`$_SERVER['HTTP_SW3']`获取。[协议文档](https://github.com/apache/skywalking/blob/master/docs/en/protocols/Skywalking-Cross-Process-Propagation-Headers-Protocol-v1.md)
 * 如果`skywalking.version=6`, 则`http`请求的`header`中会加入一个名为`SW6`的数据。PHP可使用`$_SERVER['HTTP_SW6']`获取。[协议文档](https://github.com/apache/skywalking/blob/master/docs/en/protocols/Skywalking-Cross-Process-Propagation-Headers-Protocol-v2.md)
 * SW6值格式为：`1-{distributedTraceIdEncode}-{traceSegmentIdEncode}-{span_id}-{application_instance}-{entryApplicationInstance}-{peerHostEncode}-{entryEndpointNameEncode}-{parentEndpointNameEncode}`, Encode算法为base64_encode()。示例取值为：`1-OTQuMjIyMzUuMTU2MjkxMDg1MzAwMDM=-OTQuMjIyMzUuMTU2MjkxMDg1MzAwMDM=-1-94-94-IwE6NDQz-Iy9pbmRleC5waHA/ZGRkZGRkZA==-IwE=`
 * 值中`distributedTraceId`即下方的`globalTraceIds[0]`的取值

1. 会传递trace信息的请求类型有哪些？
 * php curl扩展发出的请求
 * php PDO扩展发出的请求
 * php mysqli扩展发出的请求

1. skywalking_get_trace_info()函数的返回值格式？
 * 返回值为数组。如果扩展加载但是未启用(`skywalking.enable=0`), 则返回空数组
```php
 # skywalking_get_trace_info 返回值格式如下：
 [
     'application_instance' => 94, 
     'pid' => 22230,
     'application_id' => 29,
     'version' => 6,
     'segment' => [
         'traceSegmentId' => '94.22412.15629106130002',
         'isSizeLimited' => 0,
         'spans' => [
             [
                 'tags' => [
                     'url' => '/index.php?id=123'
                 ],
                 'spanId' => 0,
                 'parentSpanId' => -1,
                 'startTime' => 1562910613896,
                 'operationName' => '/index.php',
                 'peer' => '127.0.0.1:80',
                 'spanType' => 0,
                 'spanLayer' => 3,
                 'componentId' => 2,
                 'refs' => [
                     [
                         'type' => 0,
                         'parentTraceSegmentId' => '94.22412.15629106130002',
                         'parentSpanId' => 1,
                         'parentApplicationInstanceId' => 94,
                         'networkAddress' => ':443',
                         'entryApplicationInstanceId' => 94,
                         'entryServiceName' => 'index.php?to=123',
                         'parentServiceName' => '',
                     ],
                 ],
             ],
         ],
     ],
     'globalTraceIds' => [
         '94.22235.15629108530003',
     ],
 ]
 ```
 
1. mysqli有过程式风格和对象式风格，上报的数据有何不同？
 * 目前扩展仅监控mysqli_query()、mysqli::query()方法, 并记录对应执行的sql, 在上报到oapServer的时候，统一按照mysqli->query()的格式进行上报