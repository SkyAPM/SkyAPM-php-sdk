SkyAPM PHP
==========
![CI](https://github.com/SkyAPM/SkyAPM-php-sdk/workflows/CI/badge.svg)
![CI](https://travis-ci.org/SkyAPM/SkyAPM-php-sdk.svg?branch=master)
![release](https://img.shields.io/github/release/SkyAPM/SkyAPM-php-sdk.svg)
![PHP](https://img.shields.io/badge/PHP-%3E%3D%207.0-brightgreen.svg)
![contributors](https://img.shields.io/github/contributors/SkyAPM/SkyAPM-php-sdk.svg)
![platform](https://img.shields.io/badge/platform-macos%20%7C%20linux-brightgreen.svg)
![license](https://img.shields.io/badge/license-Apache%202.0-green.svg)
![issues](https://img.shields.io/github/issues/SkyAPM/SkyAPM-php-sdk.svg)


<img src="https://skyapmtest.github.io/page-resources/SkyAPM/skyapm.png" alt="Sky Walking logo" height="90px" align="right" />

**SkyAPM PHP** is the PHP instrumentation agent, which is compatible with [Apache SkyWalking](https://github.com/apache/skywalking) backend and others compatible agents/SDKs

## Support List
1. CURL
1. PDO
1. Mysqli
1. Yar Client ([Yar](https://www.php.net/manual/en/book.yar.php)) (only Yar version > 2.0.4 support set header)
1. Yar Server ([Yar](https://www.php.net/manual/en/book.yar.php))
1. GRPC Client ([GRPC](https://github.com/grpc/grpc-php))
1. Predis Client ([Predis](https://packagist.org/packages/predis/predis))
1. Redis Extension ([Redis Extension](https://github.com/phpredis/phpredis))
1. Memcache Extension ([Memcache Extension](https://www.php.net/manual/en/book.memcached.php))
1. RabbitMQ
1. Swoole ([Swoole](https://github.com/swoole/swoole-src))
1. Hyperf ([Hyperf](https://github.com/hyperf/hyperf))
1. Swoft ([Swoft](https://github.com/swoft-cloud/swoft))
1. Tars-php ([Tars-php](https://github.com/TarsPHP/TarsPHP))
1. LaravelS ([LaravelS](https://github.com/hhxsv5/laravel-s))

## Documents
* [Documents in English](docs/README.md)

## Docker image (Quick start)
Go to Docker hub -> [https://hub.docker.com/u/skyapm](https://hub.docker.com/u/skyapm)
```shell script
docker run -d -e SW_OAP_ADDRESS=127.0.0.1:11800 skyapm/skywalking-php-8.0-fpm-alpine
```

## Downloads
Please head to the [releases page](https://pecl.php.net/package/skywalking) to download a release of SkyAPM PHP.

## Live Demo
Find the [demo](https://skywalking.apache.org/#demo) and [screenshots](https://skywalking.apache.org/#arch) on our website.

**Video on youtube.com**

[![RocketBot UI](http://img.youtube.com/vi/mfKaToAKl7k/0.jpg)](http://www.youtube.com/watch?v=mfKaToAKl7k)

## Contact Us
* Submit an [issue](https://github.com/SkyAPM/SkyAPM-php-sdk/issues)
* Mail list: **dev@skywalking.apache.org**. Mail to `dev-subscribe@skywalking.apache.org`, follow the reply to subscribe the mail list.
* Join `#skywalking` channel at [Apache Slack](https://join.slack.com/t/the-asf/shared_invite/enQtNDQ3OTEwNzE1MDg5LWY2NjkwMTEzMGI2ZTI1NzUzMDk0MzJmMWM1NWVmODg0MzBjNjAxYzUwMjIwNDI3MjlhZWRjNmNhOTM5NmIxNDk)
* QQ Group: 392443393(2000/2000, not available), 901167865(available)

## License
[Apache 2.0](LICENSE)

## Stargazers over time

[![Stargazers over time](https://starchart.cc/SkyAPM/SkyAPM-php-sdk.svg)](https://starchart.cc/SkyAPM/SkyAPM-php-sdk)
