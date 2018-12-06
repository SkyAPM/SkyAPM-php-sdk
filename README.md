Sky Walking for php SDK| [中文](README_ZH.md)
==========
**SkyWalking php SDK**: A client tool developed for PHP using SkyWalking APM tools. (It is a pity that it only supports checking curl currently.)
SkyWalking APM : https://github.com/apache/incubator-skywalking

* Automatic probe for PHP. **You don't need to modify the application code.**        
  * It is a PHP extension developed by zendAPI.
  * It can hook automatically for monnitoring nodes and collecting logs.
  * It transfers with header informations automatically.
* Manual probe.
  * It is a php extension and contains some php classes. 
  * It can compatible any versions of php.You can look over the codes in the class 'SkyWalking'.
  * You need to add  business codes to monnitor.

# Dep
* pkg-config
* grpc
* protoc
* php 7+


# Install
1. build php extension
```shell
git clone --recurse-submodules https://github.com/SkywalkingContrib/skywalking-php-sdk.git
cd sky-php7ext
phpize && ./configure && make && make install
```

2. make report_client
```shell
cd sky-php7ext/report
make
```

# Config
* php.ini
```shell
extension=skywalking.so
skywalking.app_code = app_code
skywalking.grpc = 127.0.0.1:11800
```

# Run
```shell
php-fpm
./report_client 120.0.0.1:11800 /tmp
```

# Apply to
*  version 1.0 applies to Skywalking 3.1 .
*  version 2.0 applies to Skywalking 3.X .
*  version 5.0 applies to Skywalking 5.X .
# In the future.
  * more php versions.
  * more os.
# Contact Us
  * Submit an issue
  * QQ Group: 155841680