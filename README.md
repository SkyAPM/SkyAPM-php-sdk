Sky Walking for php SDK| [中文](README.md)
==========
**SkyWalking php SDK**: A client tool developed for PHP using SkyWalking APM tools. (It is a pity that it only supports writing log and checking curl currently.)
SkyWalking APM : https://github.com/OpenSkywalking/skywalking

* Automatic probe for PHP. **You don't need to modify the application code.**        
  * It is a PHP extension developed by zendAPI.
  * It can hook automatically for monnitoring nodes and collecting logs.
  * It transfers with header informations automatically.
* Manual probe.
  * It is a php extension and contains some php classes. 
  * It can compatible any versions of php.You can look over the codes in the class 'SkyWalking'.
  * You need to add  business codes to monnitor.
  


# Using and installing
*  php extensions(https://github.com/SkywalkingContrib/skywalking-php-sdk/tree/songzhian/sky-php7ext)
*  php classes (https://github.com/SkywalkingContrib/skywalking-php-sdk/tree/songzhian/sdk-php)

# Apply to
*  version 1.0 applies to Skywalking 3.1 .

# In the future.
  * more php versions.
  * more os.
# Contact Us
  * Submit an issue
  * QQ Group: 155841680