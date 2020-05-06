Tips: It is recommended that you use SkyWalking and use nginx as load balancing

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
build them from source.

You can run the following commands to install the SkyWalking PHP Agent.

## Install PHP Extension 
```shell script
curl -Lo v3.2.8.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/v3.2.8.tar.gz
tar zxvf v3.2.8.tar.gz
cd SkyAPM-php-sdk-3.2.8
phpize && ./configure && make && make install
```

## Install sky-php-agent
### Build
For installing the sky-php-agent, you first need to build it:

```shell script
cd SkyAPM-php-sdk-3.2.8
go build -o sky-php-agent cmd/main.go
chmod +x sky-php-agent
cp sky-php-agent /usr/local/bin
```

## How to use

### Add skywalking config to php.ini and restart php-fpm

```shell script
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set skyWalking collector version (5 or 6 or 7 or 8)
skywalking.version = 6

; Set app code e.g. MyProjectName
skywalking.app_code = MyProjectName

; sock file path（default /tmp/sky-agent.sock）
skywalking.sock_path=/tmp/sky-agent.sock
```

## Select startup script and log output method 
## If you use CentOS 7 Use the startup script below to  You need to change the corresponding address, version
```shell script
[Unit]
Description=The Sw-Php-Agent Process Manager
After=syslog.target network.target

[Service]
Type=simple
#Modify the corresponding directory and address here
ExecStart=/usr/local/bin/sky-php-agent-linux-X64 --grpc=127.0.0.1:11800 --sky-version=7 --socket=/tmp/sky-agent.sock
ExecStop=/bin/kill -SIGINT $MAINPID
Restart=on-failure

[Install]
WantedBy=multi-user.target

```
## If you use CentOS6  Log management with lograted
## This is Start script You need to change the address, version and Copy and run it with root
```shell script
echo "
#/bin/bash

start(){
[ -f /tmp/php-agent.pid ]&&echo "Alredy running"&&exit 1
/usr/local/bin/sky-php-agent-linux-X64 --grpc=127.0.0.1:11800 --sky-version=7 --socket=/tmp/sky-agent.sock 1>>/var/log/phpagent/php-agent.log 2>>/var/log/phpagent/php-agent-error.log & 
if [ $? -eq 0 ];then
	echo $! >/tmp/php-agent.pid
	echo "sky-php-agent startup!!!"
fi

}

stop(){
kill -9 `cat /tmp/php-agent.pid`
rm -f /tmp/php-agent.pid
[[ $? == 0 ]]&&echo "sky-php-agent stop!!!"
}



# add the restart method

case $1 in

     start)

       start
     ;;

     stop)

       stop
     ;;

     restart)

       stop

       start
    ;;


     *)

      echo "USAGE: $0 {start |stop |restart  }"
    ;;

esac

exit 0 " > /etc/init.d/php-agent
```
## This is Log management script，Copy and run it with root
```shell script
mkdir /var/log/phpagent/
echo "
/var/log/phpagent/*.log {
        rotate 10
        daily
        dateext
        nocompress
        missingok
        notifempty
        create 0644  root  root
		postrotate
			/etc/init.d/php-agent restart
		endscript
}" >/etc/logrotate.d/phpagent
```

### Agent parameter description
```shell script
# View help information
./sky-php-agent -h

# Specify grpc address
/usr/local/bin/sky-php-agent --grpc 127.0.0.1:11800

# Specify the socket file. The path is the path in the php.ini configuration
/usr/local/bin/sky-php-agent --socket=/tmp/sky-agent.sock

# Specify the version of skywalking
/usr/local/bin/sky-php-agent --sky-version=7
```

### ⚠️⚠️⚠️ Warning *Make sure PHP has read and write permissions on the socks file*
