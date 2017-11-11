# PHP SkyWalking Class: SkyWalking PHP client


---

- [Installation](#installation)
- [Requirements](#requirements)
- [Quick Start and Examples](#quick-start-and-examples)

---

### Installation

To install PHP SkyWalking extend, simply:
copy ini to php.ini
commands::
	
	git clone https://github.com/SkywalkingContrib/skywalking-php-sdk.git
	cd skywalking-php-sdk/sky-php7ext
	phpize
	.configure
	make -j [number_of_processor_cores] # eg. make -j 4
	make install
### Requirements

the current phpext works with PHP 7.0-7.9.99 。
this extension references something external, use with: pphcurl、phpjson and phpstandard

### Documentation
- https://github.com/OpenSkywalking/skywalking/wiki

### Quick Start and Examples

    set php.ini skywalking.auto_open = On (Automatic writing  **Ignore the following**)

    OR  skywalking.auto_open = OFF (the project int code )As below:
commands::
    // must put these code at the beginning !!!
    
    ....
    ....
    
    
    $ch = curl_init();
    curl_setopt($ch);
    
    SkyWalking::getInstance()->startSpanOfCurl("www.api.com", $headers);
    ....
    ....
    
    curl_setopt($ch);
    $rs = curl_exec($ch);
    
    SkyWalking::getInstance()->endSpanOfcurl($ch);
    
    ....
    ....