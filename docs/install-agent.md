When building directly from Git sources or after custom modifications you might also need:
* php 7+
* SkyWalking Collector


# Install SkyWalking PHP Agent

You can run the following command to install the SkyWalking PHP Agent in your computer.

```shell
// install php extension
git clone https://github.com/SkyAPM/SkyAPM-php-sdk.git
cd SkyAPM-php-sdk
phpize && ./configure && make && make install

// download report_client_linux
wget https://github.com/..... && cp report_client_linux report_client /usr/bin
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
```

Run `report_client` to send PHP generated log information to `SkyWalking collector`
```shell
// report_client [collector grpc address]
// e.g.
report_client 127.0.0.1:11800
```