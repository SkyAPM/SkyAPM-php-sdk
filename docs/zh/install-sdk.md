# 环境

PHP7+


# 安装

## 安装PHP的SkyWalking扩展(Ubuntu环境)


1.安装php-dev (具体的PHP版本以你的环境为准，必须php7+)

```shell
sudo apt install php7.2-dev
```


2.安装curl开发包

```shell
sudo apt install curl-dev 
```

3.确定根目录

```shell
mkdir -p /data/skywalking && cd /data/skywalking
```

4.git clone 源码

```shell
git clone https://github.com/SkyAPM/SkyAPM-php-sdk.git
```


4.编译安装SkyAPM-php-sdk

```shell
cd SkyAPM-php-sdk
phpize
./configure
make
sudo make install
```


5.新建php的skywalking扩展配置文件，写入配置

```shell
# 假设php.ini文件为 /usr/local/php/etc/php.ini

sudo cat >> /usr/local/php/etc/php.ini<<EOF
; Loading extensions in PHP
extension=skywalking.so
; enable skywalking
skywalking.enable = 1
; Set skyWalking collector version
skywalking.version = 6
; Set app code e.g. MyProjectName
skywalking.app_code = your_php_project_name
; Set skywalking php agent sock path
skywalking.sock_path=/tmp/sky_agent.sock
EOF
```
扩展so
extension=skywalking.so
是否启用：0 关闭；1 启用 (默认值为0)
skywalking.enable=1
skywalking的版本：5或者6（默认值为6）
skywalking.version=6
app_code代码，不要含特殊字符，请使用数字、字母、下换线。(默认为：hello_skywalking)
skywalking.app_code=hello_skywalking
sock文件路径（默认值为/tmp/sky_agent.sock）
skywalking.sock_path=/tmp/sky_agent.sock


6.重启php-fpm服务

```shell
sudo service php-fpm restart
```


7.查看skywalking扩展是否成功加载

```shell
php -m | grep skywalking
```

查看具体配置，请使用`php -i`或者`phpinfo()`函数

