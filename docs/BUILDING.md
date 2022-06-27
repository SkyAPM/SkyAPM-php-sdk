# SkyWalking PHP Agent - Building from source

This document has detailed instructions on how to build SkyWalking PHP Agent from source.

## Pre-requisites

1. gcc
2. make
3. rust
4. protoc

### Linux
```shell
$ [sudo] apt-get install build-essential autoconf automake libtool curl make gcc unzip pkg-config
$ [sudo] apt-get install cmake rust cargo rustfmt
```

### MacOS

On a Mac, you will first need to install Xcode or Command Line Tools for Xcode and then run the following command from a terminal:

```shell
 $ [sudo] xcode-select --install
 $ brew install autoconf automake libtool shtool
```

### Alpine
```shell
 $ apk --update add --no-cache git ca-certificates autoconf automake libtool 
 $ apk --update add --no-cache cmake gcc make file linux-headers file re2c 
 $ apk --update add --no-cache pkgconf openssl openssl-dev curl curl-dev nginx protoc rust rustfmt cargo
```

## Build from source (PHP Extension)

```shell script
curl -Lo v5.0.0.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/v5.0.0.tar.gz
tar zxvf v5.0.0.tar.gz
cd SkyAPM-php-sdk-5.0.0
phpize
./configure
make
sudo make install
```
