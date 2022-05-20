#
# Copyright 2021 SkyAPM
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ARG PHP_VERSION
FROM php:${PHP_VERSION}-fpm-alpine

ADD . /var/local/git/skywalking

ENV PROTOBUF_VERSION 3.14.0
ENV PROTOBUF_URL https://github.com/protocolbuffers/protobuf/releases/download/v"$PROTOBUF_VERSION"/protobuf-cpp-"$PROTOBUF_VERSION".zip
ENV RUSTFLAGS="-Ctarget-feature=-crt-static"

RUN set -ex \
    && docker-php-ext-install mysqli \
    && apk --update add --no-cache git ca-certificates build-base unzip autoconf automake libtool g++ make file linux-headers file re2c pkgconf openssl openssl-dev curl-dev nginx \
    && curl https://sh.rustup.rs -sSf | sh -s -- -y \
    && source $HOME/.cargo/env \
    && curl --silent -L -o protobuf.zip "$PROTOBUF_URL" \
    && unzip protobuf.zip \
    && cd protobuf-"$PROTOBUF_VERSION" \
    && ./configure && make -j$(nproc) && make install \
    && cd .. && rm protobuf.zip \
    && echo "--- installing skywalking php ---" \
    && cd /var/local/git/skywalking \
    && git submodule update --init \
    && phpize && ./configure && make -j$(nproc) && make install \
    && cp php.ini $PHP_INI_DIR/conf.d/ext-skywalking.ini \
    && mkdir -p /opt \
    && cp docker/entrypoint.sh /opt/ \
    && cp docker/nginx.conf /etc/nginx/nginx.conf \
    && cp docker/index.php /var/www/html/index.php \
    && cd / \
    && rm -rf /var/cache/apk/* \
    && rm -fr /var/local/git

EXPOSE 9000
EXPOSE 8080

ENTRYPOINT ["/opt/entrypoint.sh"]