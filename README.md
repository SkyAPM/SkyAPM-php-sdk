SkyAPM PHP
==========
![CI](https://github.com/SkyAPM/SkyAPM-php-sdk/workflows/CI/badge.svg)

<img src="https://skyapmtest.github.io/page-resources/SkyAPM/skyapm.png" alt="Sky Walking logo" height="90px" align="right" />

**SkyAPM PHP** is the PHP instrumentation agent, which is compatible with [Apache SkyWalking](https://github.com/apache/skywalking) backend and others compatible agents/SDKs


## Documents
* [Documents in English](docs/README.md)

## Docker image
Go to Docker hub -> [https://hub.docker.com/r/skyapm/skywalking-php](https://hub.docker.com/r/skyapm/skywalking-php)
```shell script
docker run -d -e SW_AGENT_COLLECTOR_BACKEND_SERVICES=127.0.0.1:11800 skywalking-php
```

## Live Demo
Host in Beijing. Go to [demo](http://106.75.237.45:8080/).
- Username: admin
- Password: admin

**Video on youtube.com**

[![RocketBot UI](http://img.youtube.com/vi/JC-Anlshqx8/0.jpg)](http://www.youtube.com/watch?v=JC-Anlshqx8)

## Support List
1. CURL
1. PDO
1. Mysqli
1. Yar Client ([Yar](https://www.php.net/manual/en/book.yar.php))
1. GRPC Client ([GRPC](https://github.com/grpc/grpc-php))
1. Predis Client ([Predis](https://packagist.org/packages/predis/predis))
1. Redis Extension ([Redis Extension](https://github.com/phpredis/phpredis))
1. Memcache Extension

## Contact Us
* Submit an [issue](https://github.com/SkyAPM/SkyAPM-php-sdk/issues)
* Mail list: **dev@skywalking.apache.org**. Mail to `dev-subscribe@skywalking.apache.org`, follow the reply to subscribe the mail list.
* Join `#skywalking` channel at [Apache Slack](https://join.slack.com/t/the-asf/shared_invite/enQtNDQ3OTEwNzE1MDg5LWY2NjkwMTEzMGI2ZTI1NzUzMDk0MzJmMWM1NWVmODg0MzBjNjAxYzUwMjIwNDI3MjlhZWRjNmNhOTM5NmIxNDk)
* QQ Group: 392443393(2000/2000, not available), 901167865(available)

## License
[Apache 2.0](LICENSE)
