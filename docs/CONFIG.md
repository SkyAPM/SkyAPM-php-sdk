
# Config SkyWalking PHP Agent

## Add SkyWalking config to php.ini and restart php-fpm

```shell
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set app code e.g. MyProjectName
skywalking.service = the_skywalking_php_agent

; skywalking oap version
skywalking.oap_version = 9.0.0

; Set grpc address
skywalking.grpc_address = 127.0.0.1:11800
; skywalking.grpc_tls_enable
; skywalking.grpc_tls_pem_root_certs
; skywalking.grpc_tls_pem_private_key
; skywalking.grpc_tls_pem_cert_chain

; Enable log, default disable
; Options trace, debug, info, warn, error
skywalking.log_level = disable
; skywalking.log_path = /tmp/skywalking-php.log

; Enable with cURL response, default 0
; skywalking.curl_response_enable = 0
```

