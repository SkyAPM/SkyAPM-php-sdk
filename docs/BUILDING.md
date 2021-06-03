# SkyWalking PHP Agent - Building from source

This document has detailed instructions on how to build SkyWalking PHP Agent from source.

## Pre-requisites

### Linux
```shell
$ [sudo] apt-get install build-essential autoconf automake libtool curl make g++ unzip pkg-config
$ [sudo] apt-get install cmake libboost-all-dev
```

### MacOS

On a Mac, you will first need to install Xcode or Command Line Tools for Xcode and then run the following command from a terminal:

```shell
 $ [sudo] xcode-select --install
```

To build gRPC from source, you may need to install the following packages from Homebrew:

```shell
 $ brew install autoconf automake libtool shtool
```

### Alpine
```shell
 $ apk --update add --no-cache git ca-certificates autoconf automake libtool g++ make file linux-headers file re2c pkgconf openssl openssl-dev curl-dev nginx boost-dev
```

## Build gRPC static library

```shell
 $ git clone --depth 1 -b v1.34.x https://github.com/grpc/grpc.git /var/local/git/grpc
 $ cd /var/local/git/grpc
 $ git submodule update --init --recursive
 $ mkdir -p cmake/build
 $ cd cmake/build
 $ cmake ../..
 $ make -j$(nproc)
```

## Build from source (PHP Extension)

Use the `--with-grpc` option to set the path of the gRPC static library

```shell script
curl -Lo v4.1.2.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/v4.1.2.tar.gz
tar zxvf v4.1.2.tar.gz
cd SkyAPM-php-sdk-4.1.2
phpize
./configure --with-grpc="/var/local/git/grpc"
make
sudo make install
```