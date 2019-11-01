# 启动agent

1.选择你的操作系统对应的agent应用程序

    darwin指MacOS操作系统, linux指Linux操作系统
    x64/x86分别指64位操作系统/32位操作系统

```
sky-php-agent-darwin-x64
sky-php-agent-darwin-x86
sky-php-agent-linux-x64
sky-php-agent-linux-x86
sky-php-agent-linux-arm64
sky-php-agent-linux-arm86
```


2.启动agent(以 Ubuntu x64 平台示例)

```shell
# 添加可执行权限
sudo chmod +x ./sky-php-agent-linux-x64

# 启动
./sky-php-agent-linux-x64 127.0.0.1:11800 /var/run/sky-agent.sock
```


3.agent 参数说明

 * 第一个参数为SkyWalking服务端的GRPC地址
 * 第二个参数为sock文件的绝对路径，必须与php中skywalking.sock_path的路径一致。默认值为：`/var/run/sky-agent.sock`
 * `-h` 可查看帮助信息


