Sky Walking for php SDK| [English](README.md)
==========

<img src="https://skywalkingtest.github.io/page-resources/3.0/skywalking.png" alt="Sky Walking logo" height="90px" align="right" />

**SkyWalking php SDK **: 针对SkyWalking APM工具为php开发的客户端工具, (目前只支持写日志方式和只针对curl检测).

[![Build Status](https://travis-ci.org/OpenSkywalking/skywalking.svg?branch=master)](https://travis-ci.org/OpenSkywalking/skywalking)
[![Coverage Status](https://coveralls.io/repos/github/OpenSkywalking/skywalking/badge.svg?branch=master&forceUpdate=2)](https://coveralls.io/github/OpenSkywalking/skywalking?branch=master)
[![Join the chat at https://gitter.im/openskywalking/Lobby](https://badges.gitter.im/openskywalking/Lobby.svg)](https://gitter.im/openskywalking/Lobby)
[![OpenTracing-1.x Badge](https://img.shields.io/badge/OpenTracing--1.x-enabled-blue.svg)](http://opentracing.io)


* PHP 自动探针，**不需要修改应用程序代码**
  * 使用php扩展开发,自动钩子,自动探针日志。
  * 是有配置文件配置相关常量数据
* 手动探针
  * php扩展 和 php代码sdk类包 
  * php代码SkyWalking类进行php各种版本兼容 
  * 需要植入业务代码进行监控


# wiki文档
* [WIKI](https://github.com/OpenSkywalking/skywalking/wiki)



# 使用安装
  php扩展包(https://github.com/SkywalkingContrib/skywalking-php-sdk/tree/songzhian/sky-php7ext)
  php代码类包(https://github.com/SkywalkingContrib/skywalking-php-sdk/tree/songzhian/sdk-php)

# 对应版本支持
  1.0对应版本:Skywalking 3.1

# 将要完成
  * 其它php版本扩展支持
  * 更多系统支持
