
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

; Enable logging report, default 0
skywalking.logging_enable = 0
skywalking.logging_mq_max_message_length = 20480
skywalking.logging_mq_length = 1024

; Enable yii logging report, default 0
skywalking.logging_yii_enable = 0
; Intercept yii\log\FileTarget export method and report logging to the oap server
skywalking.logging_yii_target_name = yii\log\FileTarget

; Enable thinkphp logging report, default 0
skywalking.logging_thinkphp_enable = 0
; Intercept think\log\driver\File save method and report logging to the oap server
skywalking.logging_thinkphp_target_name = think\log\driver\File

; Enable internal (error_log) logging report, default 0
skywalking.logging_internal_enable = 0

```



## Important

1. Make sure php-fpm is running in foreground mode `--nodaemonize`
