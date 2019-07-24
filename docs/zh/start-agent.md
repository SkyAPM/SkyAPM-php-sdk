# 启动agent

1.选择你的操作系统对应的agent应用程序

    darwin指MacOS操作系统, linux指Linux操作系统
    x64/x86分别指64位操作系统/32位操作系统

```
sky_php_agent_darwin_x64
sky_php_agent_darwin_x86
sky_php_agent_linux_x64
sky_php_agent_linux_x86
sky_php_agent_linux_arm64
sky_php_agent_linux_arm86
```


2.启动agent(以 Ubuntu x64 平台示例)

```shell
# 添加可执行权限
sudo chmod +x ./sky_php_agent_linux_x64

# 启动
./sky_php_agent_linux_x64 127.0.0.1:11800 /tmp/sky_agent.sock
```


3.agent 参数说明

 * 第一个参数为SkyWalking服务端的GRPC地址
 * 第二个参数为sock文件的绝对路径，必须与php中skywalking.sock_path的路径一致。默认值为：`/tmp/sky_agent.sock`
 * `-h` 可查看帮助信息


