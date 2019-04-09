When building directly from Git sources or after custom modifications you might also need:
* pkg-config
* grpc latest version
* protoc latest version
* php 7+
* SkyWalking Collector


# Install SkyWalking PHP Agent

You can run the following command to install the SkyWalking PHP Agent in your computer.

```shell
// install php extension
git clone https://github.com/SkyAPM/SkyAPM-php-sdk.git
cd SkyAPM-php-sdk
phpize && ./configure && make && make install

// install report_client
cd src/report
make
cp report_client /usr/bin
```

# How to use

php.ini

```shell
; Loading extensions in PHP
extension=skywalking.so
; enable skywalking
skywalking.enable = 1
; Set skyWalking collector version
skywalking.version = 6
; Set app code e.g. MyProjectName
skywalking.app_code = MyProjectName
; Set skyWalking collector grpc address
skywalking.grpc = 127.0.0.1:11800
; Set log path
skywalking.log_path = /tmp
; Set http header version
header_version = 2
```

Run `report_client` to send PHP generated log information to `SkyWalking collector`
```shell
// report_client [collector grpc address] [log path]
// e.g.
report_client 120.0.0.1:11800 /tmp
```