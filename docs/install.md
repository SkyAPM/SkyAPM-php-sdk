# Install SkyWalking PHP Agent

## Requirements
When building directly from Git sources or after custom modifications you might also need:
* gRPC and Protobuf
* php v7.0 or newer
* SkyWalking oap server
* SkyWalking UI
* CMake v3.13 or newer

## Install gPRC ans Protobuf

View official documents [GRPC C++ installation](https://github.com/grpc/grpc/blob/master/BUILDING.md)

View official documents [protobuf C++ installation](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)

#### Ubuntu
```shell script
sudo apt-get install build-essential autoconf automake libtool curl make g++ unzip pkg-config cmake libboost-all-dev
git clone --depth 1 -b v1.34.x https://github.com/grpc/grpc.git /var/local/git/grpc
cd /var/local/git/grpc
git submodule update --init --recursive

# protobuf
cd /var/local/git/grpc/third_party/protobuf
./autogen.sh
./configure
make -j$(nproc)
sudo make install
sudo ldconfig
make clean

# grpc
cd /var/local/git/grpc
mkdir -p cmake/build
cd cmake/build
cmake ../.. -DBUILD_SHARED_LIBS=ON -DgRPC_INSTALL=ON
make -j$(nproc)
sudo make install
make clean
sudo ldconfig
```

#### Alpine
```shell script
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64
export LD_RUN_PATH=$LD_RUN_PATH:/usr/local/lib:/usr/local/lib64
apk --update add --no-cache git ca-certificates autoconf automake libtool g++ make file linux-headers file re2c pkgconf openssl openssl-dev curl-dev nginx boost-dev
git clone --depth 1 -b v1.34.x https://github.com/grpc/grpc.git /var/local/git/grpc
cd /var/local/git/grpc
git submodule update --init --recursive

# protobuf
cd /var/local/git/grpc/third_party/protobuf
./autogen.sh
./configure
make -j$(nproc)
sudo make install
make clean

# grpc
cd /var/local/git/grpc
mkdir -p cmake/build
cd cmake/build
cmake ../.. -DBUILD_SHARED_LIBS=ON -DgRPC_INSTALL=ON
make -j$(nproc)
sudo make install
make clean
```

## Install PHP Extension
```shell script
curl -Lo v4.1.2.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/v4.1.2.tar.gz
tar zxvf v4.1.2.tar.gz
cd SkyAPM-php-sdk-4.1.2
phpize
./configure
make
sudo make install
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

## Important

1. Make sure php-fpm is running in foreground mode


## Trouble Shooting
### Nothing Receiving in OapService?
Check your php-fpm start mode. It must start with "--nodaemonize" to stay php-fpm in foreground.
