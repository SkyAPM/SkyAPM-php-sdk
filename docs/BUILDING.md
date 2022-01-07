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
 $ apk --update add --no-cache git ca-certificates autoconf automake libtool 
 $ apk --update add --no-cache cmake g++ make file linux-headers file re2c 
 $ apk --update add --no-cache pkgconf openssl openssl-dev curl curl-dev nginx boost-dev
```

## Build gRPC static or shared library (pick one of two)

```shell
 # static
 $ git clone --depth 1 -b v1.34.x https://github.com/grpc/grpc.git /var/local/git/grpc
 $ cd /var/local/git/grpc
 $ git submodule update --init --recursive
 $ mkdir -p cmake/build
 $ cd cmake/build
 $ cmake ../..
 $ make -j$(nproc)
 
 # shared
 $ git clone --depth 1 -b v1.34.x https://github.com/grpc/grpc.git /var/local/git/grpc
 $ cd /var/local/git/grpc
 $ git submodule update --init --recursive
 $ cd third_party/protobuf
 $ ./autogen.sh
 $ ./configure
 $ make -j$(nproc)
 $ sudo make install
 $ sudo ldconfig
 $ cd /var/local/git/grpc
 $ git submodule update --init --recursive
 $ mkdir -p cmake/build
 $ cd cmake/build
 $ cmake ../.. -DBUILD_SHARED_LIBS=ON -DgRPC_INSTALL=ON
 $ make -j$(nproc)
 $ sudo make install
 $ sudo ldconfig
```

## Build from source with static gRPC library (PHP Extension)

Use the `--with-grpc-src` option to set the path of the gRPC static library

```shell script
curl -Lo v4.2.0.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/v4.2.0.tar.gz
tar zxvf v4.2.0.tar.gz
cd SkyAPM-php-sdk-4.2.0
phpize
./configure --with-grpc-src="/var/local/git/grpc"
make
sudo make install
```
