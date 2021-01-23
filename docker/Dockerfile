FROM php:7.4-fpm-buster AS builder

ENV GRPC_RELEASE_TAG v1.31.x
ENV PROTOBUF_RELEASE_TAG 3.13.x

RUN set -ex \
    && apt-get update && apt-get install -y \
        autoconf automake libtool curl make g++ unzip \
        build-essential autoconf libtool pkg-config cmake \
        git ca-certificates libcurl4-openssl-dev nginx re2c libboost-dev \
    && rm -fr /var/lib/apt/lists/* \
    && echo "--- clone grpc ---" \
    && git clone --depth 1 -b ${GRPC_RELEASE_TAG} https://github.com/grpc/grpc /var/local/git/grpc \
    && cd /var/local/git/grpc && git submodule update --init --recursive \
    && echo "--- clone protobuf ---" \
    && git clone --depth 1 -b ${PROTOBUF_RELEASE_TAG} https://github.com/protocolbuffers/protobuf.git /var/local/git/protobuf \
    && cd /var/local/git/protobuf && git submodule update --init --recursive \
    && echo "--- installing protobuf ---" \
    && cd /var/local/git/protobuf \
    && ./autogen.sh && ./configure \
    && make -j$(nproc) && make install && make clean && ldconfig \
    && echo "--- installing grpc ---" \
    && cd /var/local/git/grpc \
    && mkdir -p cmake/build \
    && cd cmake/build \
    && cmake ../.. -DBUILD_SHARED_LIBS=ON -DgRPC_INSTALL=ON \
    && make -j$(nproc) && make install && make clean && ldconfig \
    && cd / \
    && rm -fr /var/local/git/grpc \
    && rm -fr /var/local/git/protobuf

ADD . /tmp/skywalking

RUN set -ex \
    && echo "--- installing skywalking php ---" \
    && cd /tmp/skywalking \
    && phpize && ./configure && make && make install \
    && cp php.ini $PHP_INI_DIR/conf.d/ext-skywalking.ini \
    && cp docker/service.sh /opt/ \
    && cp docker/nginx.conf /etc/nginx/nginx.conf \
    && cp docker/index.php /var/www/html/index.php \
    && cd / && rm -fr /tmp/skywalking \
    && rm -fr /usr/src/php.tar.xz

ENTRYPOINT ["/opt/service.sh"]
