
# Config SkyWalking PHP Agent

## Add SkyWalking config to php.ini and restart php-fpm

```shell
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set skyWalking collector version (5 or 6 or 7 or 8)
skywalking.version = 8

; Set app code e.g. MyProjectName
skywalking.app_code = the_skywalking_php_agent

; Set grpc address
skywalking.grpc=127.0.0.1:11800
```



## Important

1. Make sure php-fpm is running in foreground mode `--nodaemonize`
