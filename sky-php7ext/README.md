# PHP SkyWalking Class: SkyWalking PHP client


---

- [Installation](#installation)
- [Requirements](#requirements)
- [Quick Start and Examples](#quick-start-and-examples)

---

### Installation

To install PHP SkyWalking extend, simply:
copy ini to php.ini

	git clone https://github.com/SkywalkingContrib/skywalking-php-sdk.git
	cd skywalking-php-sdk/sky-php7ext
	phpize
	./configure
	make -j [number_of_processor_cores] # eg. make -j 4
	make install
### Requirements

the current phpext works with PHP 7.0-7.9.99 。
this extension references something external, use with: pphcurl、phpjson and phpstandard
need to install, Please refer to: https://github.com/grpc/grpc/blob/master/INSTALL.md
### Documentation
- https://github.com/OpenSkywalking/skywalking/wiki

### Quick Start and Examples
set php.ini skywalking.auto_open = On (Automatic writing  **Ignore the following**)  