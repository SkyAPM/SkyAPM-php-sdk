When building directly from Git sources or after custom modifications you might also need:
* php 7+
* golang 1.13
* SkyWalking Collector


# Install SkyWalking PHP Agent

You can run the following command to install the SkyWalking PHP Agent in your computer.

## Install PHP Extension
```shell script
git clone https://github.com/SkyAPM/SkyAPM-php-sdk.git
cd SkyAPM-php-sdk
phpize && ./configure && make && make install
```

## Install sky-php-agent (download or build)
### Download sky-php-agent
Go to [release](https://github.com/SkyAPM/SkyAPM-php-sdk/releases) page to download the sky-php-agent required by your platform. Currently there are Linux (amd64, arm64) and Mac versions.

### Build
```shell script
cd SkyAPM-php-sdk/src
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
