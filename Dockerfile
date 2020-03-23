FROM php:7.4-fpm-alpine AS builder

ADD . /tmp/skywalking

RUN set -ex \
    && apk add --no-cache \
       autoconf dpkg-dev dpkg file g++ gcc libc-dev make pkgconf re2c \
       go git ca-certificates curl-dev nginx \
    && cd /tmp/skywalking \
    && phpize && ./configure && make && make install \
    && go build -o /usr/local/bin/sky-php-agent ./agent/cmd/main.go \
    && cp php.ini $PHP_INI_DIR/conf.d/ext-skywalking.ini \
    && cp service.sh /opt/ \
    && cp nginx.conf /etc/nginx/nginx.conf \
    && cp skywalking.php /var/www/html/index.php \
    && cd / \
    && tar zcvf dist.tar.gz \
       opt/service.sh \
       etc/nginx/nginx.conf \
       var/www/html/index.php \
       usr/local/etc/php/conf.d/ext-skywalking.ini \
       usr/local/bin/sky-php-agent \
       usr/local/lib/php/extensions/no-debug-non-zts-20190902/skywalking.so

FROM php:7.4-fpm-alpine
COPY --from=builder /dist.tar.gz /
RUN set -ex \
    && apk add --no-cache nginx \
    && tar zxvf /dist.tar.gz -C / \
    && rm -fr /usr/src/php.tar.xz \
    && rm -fr /dist.tar.gz

ENTRYPOINT ["/opt/service.sh"]
