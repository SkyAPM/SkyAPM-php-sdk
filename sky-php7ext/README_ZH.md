# PHP SkyWalking Class: SkyWalking PHP client


---

- [安装](#installation)
- [要求](#requirements)
- [Quick Start and Examples](#quick-start-and-examples)

---

### Installation
快速安装

复制ini内容到php.ini

	git clone https://github.com/SkywalkingContrib/skywalking-php-sdk.git
	cd skywalking-php-sdk/sky-php7ext
	phpize
	./configure
	make -j [number_of_processor_cores] # eg. make -j 4
	make install
### Requirements

扩展支持版本 7.0-7.9.99 。
这个扩展需要其他扩展支持: pphcurl、phpjson和phpstandard 包
需要安装GRPC 请参照 : https://github.com/grpc/grpc/blob/master/INSTALL.md

### 快速启动
设置 php.ini skywalking.auto_open = On (自动开启信息抓取  **Ignore the following**)  
