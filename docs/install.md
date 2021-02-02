# Install SkyWalking PHP Agent

## Requirements
When building directly from Git sources or after custom modifications you might also need:
* grpc and protobuf
* php 7+
* SkyWalking oap server
* SkyWalking UI

## Install Protobuf

View official documents [protobuf C++ installation](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)

Or use

```shell script
sudo apt-get install autoconf automake libtool curl make g++ unzip
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git submodule update --init --recursive
./autogen.sh

./configure
make -j$(nproc)
make check
sudo make install
sudo ldconfig # refresh shared library cache.
```

## Install GRPC

View official documents [GRPC C++ installation](https://github.com/grpc/grpc/blob/master/BUILDING.md)

Or use

```shell script
sudo apt-get install build-essential autoconf libtool pkg-config cmake
git clone https://github.com/grpc/grpc.git
cd grpc
git submodule update --init --recursive

mkdir -p cmake/build
cd cmake/build
cmake ../.. -DBUILD_SHARED_LIBS=ON -DgRPC_INSTALL=ON
make -j$(nproc)
sudo make install
make clean
sudo ldconfig
```

## Install PHP Extension
```shell script
curl -Lo v4.1.1.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/v4.1.1.tar.gz
tar zxvf v4.1.1.tar.gz
cd SkyAPM-php-sdk-4.1.1
phpize && ./configure && make && make install
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

; Set grpc address
skywalking.grpc=127.0.0.1:11800
```
## Trouble Shooting
### Nothing Receiving in OapService?
Check your php-fpm start mode. It must start with "--nodaemonize" to stay php-fpm in foreground.
