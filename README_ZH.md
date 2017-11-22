Sky Walking SDK for PHP | [English](README_EN.md)
==========


**SkyWalking php SDK**: 针对SkyWalking APM工具为php开发的客户端工具 (目前只支持写日志方式和只针对curl检测)
SkyWalking APM : https://github.com/OpenSkywalking/skywalking

* PHP 自动探针 **不需要修改应用程序代码**        
  * 使用zendAPI开发的 php扩展
  * 自动钩子,自动SkyWalking节点收集日志。
  * 自动链路header传输
* 手动探针
  * php扩展 和 php代码sdk类包 
  * php代码SkyWalking类进行php各种版本兼容 
  * 需要植入业务代码进行监控



# 使用安装
*  php扩展包(https://github.com/SkywalkingContrib/skywalking-php-sdk/tree/songzhian/sky-php7ext)
*  php代码类包(https://github.com/SkywalkingContrib/skywalking-php-sdk/tree/songzhian/sdk-php)

# 对应版本支持
*  1.0对应版本:Skywalking 3.1

# 将要完成
  * 其它php版本扩展支持
  * 更多系统支持
# 联系
  * 直接提交Issue
  * QQ群: 155841680
