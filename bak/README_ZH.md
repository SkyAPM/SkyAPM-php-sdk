# PHP SkyWalking 类: SkyWalking PHP 客户端 


---

- [安装](#installation)
- [要求](#requirements)
- [快速开始实例](#quick-start-and-examples)

---

### Installation

使用安装 SkyWalking 类, 操作:


### Requirements

PHP Curl Class works with PHP 5.2 - 7.9.99 , and HHVM.

### Quick Start and Examples
    
    // must put these code at the beginning !!!
    // that would auto register shutdown function !!!
    
    include_once ("./sdk-php/SkyWalking.php");// or use composer
    
    //LOG_PATH is skywalking's logfile path
    SkyWalking::getInstance("api")->setLogPath(LOG_PATH)->setSamplingRate(5);
    
    ....
    ....

    $ch = curl_init();
    curl_setopt($ch);
    
    ....
    ....

    SkyWalking::getInstance()->startSpanOfCurl("www.api.com", $headers);
    
    ....
    ....
    
    curl_setopt($ch);
    $rs = curl_exec($ch);
    
    SkyWalking::getInstance()->endSpanOfcurl($ch);
    
    ....
    ....