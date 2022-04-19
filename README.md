SkyAPM PHP
==========
![CI](https://github.com/SkyAPM/SkyAPM-php-sdk/workflows/CI/badge.svg)
![release](https://img.shields.io/github/release/SkyAPM/SkyAPM-php-sdk.svg)
![PHP](https://img.shields.io/badge/PHP-%3E%3D%207.0-brightgreen.svg)
![contributors](https://img.shields.io/github/contributors/SkyAPM/SkyAPM-php-sdk.svg)
![platform](https://img.shields.io/badge/platform-macos%20%7C%20linux-brightgreen.svg)
![license](https://img.shields.io/badge/license-Apache%202.0-green.svg)
![issues](https://img.shields.io/github/issues/SkyAPM/SkyAPM-php-sdk.svg)


<img src="https://skyapmtest.github.io/page-resources/SkyAPM/skyapm.png" alt="Sky Walking logo" height="90px" align="right" />

**SkyAPM PHP** is the PHP instrumentation agent, which is compatible with [Apache SkyWalking](https://github.com/apache/skywalking) backend and others compatible agents/SDKs

## Support List
1. cURL ([PHP cURL](https://www.php.net/manual/en/book.curl.php))
2. PDO ([PHP PDO](https://www.php.net/manual/en/book.pdo.php))
3. Mysqli ([PHP Mysqli](https://www.php.net/manual/en/book.mysqli.php))
4. Redis Extension ([Redis Extension](https://github.com/phpredis/phpredis))
5. Predis Client ([Predis](https://packagist.org/packages/predis/predis))
6. Memcache Extension ([Memcache Extension](https://www.php.net/manual/en/book.memcached.php))
7. Yar Client (version > 2.0.4) ([Yar](https://www.php.net/manual/en/book.yar.php))
8. Yar Server ([Yar](https://www.php.net/manual/en/book.yar.php))
9. GRPC Client ([GRPC](https://github.com/grpc/grpc-php))
10. RabbitMQ

## Support List (Swoole ecosystem)
11. Swoole ([Swoole](https://github.com/swoole/swoole-src))
12. Hyperf ([Hyperf](https://github.com/hyperf/hyperf))
13. Swoft ([Swoft](https://github.com/swoft-cloud/swoft))
14. Tars-php ([Tars-php](https://github.com/TarsPHP/TarsPHP))
15. LaravelS ([LaravelS](https://github.com/hhxsv5/laravel-s))

## Documentation
* [Official documentation](docs/README.md)

## Docker image (Quick start)
Go to Docker hub -> [https://hub.docker.com/u/skyapm](https://hub.docker.com/u/skyapm)
```shell script
$ docker run --restart always -d -e SW_OAP_ADDRESS=oap:11800 skyapm/skywalking-php
```

## Downloads
Please head to the [releases page](https://pecl.php.net/package/skywalking) to download a release of SkyAPM PHP.

## Live Demo
- Find the [live demo](https://skywalking.apache.org/#demo) and [screenshots](https://skywalking.apache.org/#arch) on our website.
- Follow the [showcase](https://skywalking.apache.org/docs/skywalking-showcase/latest/readme/) to set up preview deployment quickly.

# Contact Us
* Mail list: **dev@skywalking.apache.org**. Mail to `dev-subscribe@skywalking.apache.org`, follow the reply to subscribe the mail list.
* Send `Request to join SkyWalking slack` mail to the mail list(`dev@skywalking.apache.org`), we will invite you in.
* Twitter, [ASFSkyWalking](https://twitter.com/AsfSkyWalking)
* QQ Group: 901167865(Recommended), 392443393
* [bilibili B站 视频](https://space.bilibili.com/390683219)

## Stargazers over time

[![Stargazers over time](https://starchart.cc/SkyAPM/SkyAPM-php-sdk.svg)](https://starchart.cc/SkyAPM/SkyAPM-php-sdk)

## License
[Apache 2.0](LICENSE)