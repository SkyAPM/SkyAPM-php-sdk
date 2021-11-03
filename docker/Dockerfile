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

ENV GRPC_RELEASE_TAG v1.31.x
ENV PROTOBUF_RELEASE_TAG 3.13.x

ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64
ENV LD_RUN_PATH=$LD_RUN_PATH:/usr/local/lib:/usr/local/lib64

ADD . /var/local/git/skywalking

RUN set -ex \
    && docker-php-ext-install mysqli \
    && apk --update add --no-cache boost-dev git ca-certificates autoconf automake libtool g++ make file linux-headers file re2c pkgconf openssl openssl-dev curl-dev nginx \
    && echo "--- clone grpc ---" \
    && git clone --depth 1 -b ${GRPC_RELEASE_TAG} https://github.com/grpc/grpc /var/local/git/grpc \
    && cd /var/local/git/grpc \
    && git submodule update --init --recursive \
    && echo "--- download cmake ---" \
    && cd /var/local/git \
    && curl -L -o cmake-3.19.1.tar.gz  https://github.com/Kitware/CMake/releases/download/v3.19.1/cmake-3.19.1.tar.gz \
    && tar zxf cmake-3.19.1.tar.gz \
    && cd cmake-3.19.1 && ./bootstrap && make -j$(nproc) && make install \
    && echo "--- installing grpc ---" \
    && cd /var/local/git/grpc \
    && mkdir -p cmake/build && cd cmake/build && cmake ../.. \
    && make -j$(nproc) \
    && echo "--- installing skywalking php ---" \
    && cd /var/local/git/skywalking \
    && phpize && ./configure --with-grpc-src=/var/local/git/grpc && make && make install \
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