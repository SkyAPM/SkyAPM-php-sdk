FROM php:7.4-fpm-alpine AS builder

ENV GRPC_RELEASE_TAG v1.31.x

ADD . /tmp/skywalking

RUN set -ex \
    && apk add --no-cache \
       autoconf dpkg-dev dpkg file g++ gcc libc-dev make pkgconf re2c \
       git ca-certificates curl-dev nginx \
    && echo "--- clone grpc ---" \
    && git clone --depth 1 -b ${GRPC_RELEASE_TAG} https://github.com/grpc/grpc /var/local/git/grpc \
    && cd /var/local/git/grpc && git submodule update --init \
    && echo "--- installing protobuf ---" \
    && cd third_party/protobuf \
    && ./autogen.sh && ./configure --enable-shared \
    && make -j$(nproc) && make -j$(nproc) check && make install && make clean && ldconfig \
    && echo "--- grpc protobuf ---" \
    && cd /var/local/git/grpc \
    && make -j$(nproc) && make install && make clean && ldconfig \
    && cd /tmp/skywalking \
    && phpize && ./configure && make && make install \
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
       usr/local/lib/php/extensions/no-debug-non-zts-20190902/skywalking.so

FROM php:7.4-fpm-alpine
COPY --from=builder /dist.tar.gz /
RUN set -ex \
    && apk add --no-cache nginx grpc protoc \
    && tar zxvf /dist.tar.gz -C / \
    && rm -fr /usr/src/php.tar.xz \
    && rm -fr /dist.tar.gz

ENTRYPOINT ["/opt/service.sh"]
