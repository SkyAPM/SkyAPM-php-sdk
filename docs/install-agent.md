# Install SkyWalking PHP Agent

## Recuriements
When building directly from Git sources or after custom modifications you might also need:
* php 7+
* golang 1.13
* SkyWalking Collector

## PHP extension + Agent
The collect data from your instance you need both the PHP extension, and the agent. 
No pre built binaries or PHP extension are availble at the moment, so you need to
build them from source.

You can run the following commands to install the SkyWalking PHP Agent.

## Install PHP Extension
```shell script
git clone https://github.com/SkyAPM/SkyAPM-php-sdk.git
cd SkyAPM-php-sdk
phpize && ./configure && make && make install
```

## Install sky-php-agent
### Build
For installing the sky-php-agent, you first need to build it:

```shell script
cd SkyAPM-php-sdk
go build -o sky-php-agent agent/cmd/main.go
chmod +x sky-php-agent
cp sky-php-agent /usr/bin
```

## How to use

### Add skywalking config to php.ini and restart php-fpm

```shell script
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set skyWalking collector version (5 or 6)
skywalking.version = 6

; Set app code e.g. MyProjectName
skywalking.app_code = MyProjectName
```

### Run `sky-php-agent` to send PHP generated log information to `SkyWalking collector`
```shell script
// sky-php-agent [collector grpc address]
// e.g.
sky-php-agent --grpc 127.0.0.1:11800
```

### ⚠️⚠️⚠️ Warning *Make sure PHP has read and write permissions on the socks file*

## Dockerfile

### Multi stage build example (starting point)
```bash
FROM golang:1.13 AS builder
ARG SKY_AGENT_VERSION=3.2.6
WORKDIR /go/src/app
RUN wget https://github.com/SkyAPM/SkyAPM-php-sdk/archive/${SKY_AGENT_VERSION}.tar.gz; \
    tar xvf ${SKY_AGENT_VERSION}.tar.gz
RUN mv SkyAPM-php-sdk-${SKY_AGENT_VERSION} build
RUN cd build; \
    GOOS=linux GOARCH=amd64 go build -o sky-php-agent-linux-x64 ./agent/cmd/main.go

FROM php:7.2.28-fpm-alpine3.11
# Copy from builder
COPY --from=builder /go/src/app/build/sky-php-agent-linux-x64 /usr/local/bin/sky-php-agent
RUN apk update && apk upgrade && apk add --no-cache autoconf libc6-compat libcurl curl-dev icu-dev & \
    apk add --no-cache --virtual .build-deps $PHPIZE_DEPS
```
