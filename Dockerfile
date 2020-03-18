FROM php:7.4-fpm

ARG SKYWALKING=3.2.7

RUN set -ex \
    && apt-get update && apt-get install -y curl libcurl4-openssl-dev golang git \
    && mkdir -p /tmp/skywalking && cd /tmp/skywalking \
    && curl -L -o skywalking.tar.gz https://github.com/SkyAPM/SkyAPM-php-sdk/archive/${SKYWALKING}.tar.gz \
    && tar zxvf skywalking.tar.gz && cd SkyAPM-php-sdk-${SKYWALKING} \
    && phpize && ./configure && make && make install \
    && cp php.ini /usr/local/etc/php/conf.d/ext-skywalking.ini \
    && ./build-sky-php-agent.sh \
    && cp sky-php-agent-* /usr/local/bin/