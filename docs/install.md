Tips: It is recommended to use nginx as the load balancer of SkyWalking-oap-server

# Install SkyWalking PHP Agent

## Requirements
When building directly from Git sources or after custom modifications you might also need:
* php 7+
* golang 1.13
* SkyWalking oap server
* SkyWalking UI

## PHP extension + Agent
The collect data from your instance you need both the PHP extension, and the agent. 
No pre built binaries or PHP extension are availble at the moment, so you need to
build them from a source.

You can run the following commands to install the SkyWalking PHP Agent.

## Install PHP Extension
```shell script
curl -Lo v3.3.1.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/v3.3.1.tar.gz
tar zxvf v3.3.1.tar.gz
cd SkyAPM-php-sdk-3.3.1
phpize && ./configure && make && make install
```

## Install sky-php-agent
### Build
For installing the sky-php-agent, you first need to build it:

```shell script
cd SkyAPM-php-sdk-3.3.1
go build -o sky-php-agent cmd/main.go
chmod +x sky-php-agent
cp sky-php-agent /usr/local/bin
```

## How to use

### Add SkyWalking config to php.ini and restart php-fpm

```shell script
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set skyWalking collector version (5 or 6 or 7 or 8)
skywalking.version = 8

; Set app code e.g. MyProjectName
skywalking.app_code = MyProjectName

; sock file path default /tmp/sky-agent.sock
; Warning *[systemd] please disable PrivateTmp feature*
; Warning *Make sure PHP has read and write permissions on the socks file*
skywalking.sock_path=/tmp/sky-agent.sock
```

## sky-php-agent `systemd` example

```shell script
[Unit]
Description=The SkyWalking PHP-Agent Process Manager
After=syslog.target network.target

[Service]
Type=simple
# Modify the corresponding directory and address here
ExecStart=/usr/local/bin/sky-php-agent --grpc=127.0.0.1:11800 --sky-version=8 --socket=/tmp/sky-agent.sock
ExecStop=/bin/kill -SIGINT $MAINPID
Restart=on-failure

[Install]
WantedBy=multi-user.target
```
