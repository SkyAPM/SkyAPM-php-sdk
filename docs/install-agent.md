When building directly from Git sources or after custom modifications you might also need:
* libuuid
* gcc 4.9+
* pkg-config
* grpc
* protoc
* php 7+


# Build

1. build php extension
```shell
git clone --recurse-submodules https://github.com/SkywalkingContrib/skywalking-php-sdk.git
phpize && ./configure && make && make install
```

2. build report_client
```shell
cd src/report
make
```

# Config
php.ini example
```shell
extension=skywalking.so
skywalking.version = 5
skywalking.app_code = app_code
skywalking.grpc = 127.0.0.1:11800
```

# Run
restart and run report_client
```shell
./report_client 120.0.0.1:11800 /tmp
```