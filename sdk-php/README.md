# PHP SkyWalking Class: SkyWalking PHP client


---

- [Installation](#installation)
- [Requirements](#requirements)
- [Quick Start and Examples](#quick-start-and-examples)

---

### Installation

To install PHP SkyWalking Class, simply:


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