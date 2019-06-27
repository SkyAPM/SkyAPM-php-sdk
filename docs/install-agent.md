When building directly from Git sources or after custom modifications you might also need:
* php 7+
* SkyWalking Collector


# Install SkyWalking PHP Agent

You can run the following command to install the SkyWalking PHP Agent in your computer.

```shell
// install php extension
git clone https://github.com/SkyAPM/SkyAPM-php-sdk.git
cd SkyAPM-php-sdk
phpize && ./configure && make && make install

// download sky_php_agent
// e.g. 3.0.0
wget https://github.com/SkyAPM/SkyAPM-php-sdk/releases/download/3.0.0/sky_php_agent_linux_x64
mv sky_php_agent_linux_x64 sky_php_agent
chmod +x sky_php_agent
cp sky_php_agent /usr/bin
```

# How to use

php.ini

```shell
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set skyWalking collector version (5 or 6)
skywalking.version = 5

; Set app code e.g. MyProjectName
skywalking.app_code = MyProjectName
```

Run `sky_php_agent` to send PHP generated log information to `SkyWalking collector`
```shell
// sky_php_agent [collector grpc address]
// e.g.
sky_php_agent 127.0.0.1:11800
```