#!/usr/bin/env sh

grpc=$SW_OAP_ADDRESS

if [ ! $grpc ]; then
    grpc="127.0.0.1:11800"
fi

echo "sw oap address:" $grpc

sed -i "s/127.0.0.1:11800/$grpc/g" $PHP_INI_DIR/conf.d/ext-skywalking.ini
nginx

php-fpm
