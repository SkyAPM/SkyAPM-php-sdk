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

// download sky-php-agent
// e.g. 3.2.0
wget https://github.com/SkyAPM/SkyAPM-php-sdk/releases/download/3.2.0/sky-php-agent-linux-x64
mv sky-php-agent-linux-x64 sky-php-agent
chmod +x sky-php-agent
cp sky-php-agent /usr/bin
```

# How to use

php.ini

```shell
; Loading extensions in PHP
extension=skywalking.so

; enable skywalking
skywalking.enable = 1

; Set skyWalking collector version (5 or 6)
skywalking.version = 6

; Set app code e.g. MyProjectName
skywalking.app_code = MyProjectName
```

Run `sky-php-agent` to send PHP generated log information to `SkyWalking collector`
```shell
// sky-php-agent [collector grpc address]
// e.g.
sky-php-agent --grpc 127.0.0.1:11800
```

Show help

```bash
sky-php-agent -h
```