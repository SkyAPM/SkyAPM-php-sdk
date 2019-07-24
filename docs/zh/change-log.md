# 20190719
## Agent 修改
1. agent创建的文件权限修改为0666, 文件的类型修改为sock类型。修复php与sock文件进行通信时提示"Permission Deny"的问题  
2. agent的参数接收方式修改, 执行`sky_php_agent -h` 查看可用参数
3. agent支持在参数中指定sock文件的路径


## SkyWalking 扩展修改
1. 新增 skywalking_get_trace_info():array 方法，获取trace数据，以方便业务日志获取当前请求trace信息。
2. 删除无用的skywalking.log_path及skywalking.grpc配置项
3. 新增 skywalking.sock_path 配置项，支持自定义sock文件的路径
