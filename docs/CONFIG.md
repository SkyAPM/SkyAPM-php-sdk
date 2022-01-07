
# Config SkyWalking PHP Agent

## Add SkyWalking config to php.ini and restart php-fpm

```shell
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set app code e.g. MyProjectName
skywalking.app_code = the_skywalking_php_agent

; Set grpc address
skywalking.grpc=127.0.0.1:11800
; skywalking.grpc_tls_enable
; skywalking.grpc_tls_pem_root_certs
; skywalking.grpc_tls_pem_private_key
; skywalking.grpc_tls_pem_cert_chain

; Enable log, default 0
skywalking.log_enable = 0
; skywalking.log_path = /tmp/skywalking-php.log

; Enable with cURL response, default 0
; skywalking.curl_response_enable = 0
```



## Important

1. Make sure php-fpm is running in foreground mode `--nodaemonize`
