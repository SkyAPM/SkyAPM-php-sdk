FROM php:7.4-fpm

RUN set -ex \
    && apt-get update -y && apt-get install --no-install-recommends -y \
    curl \
    libcurl4-openssl-dev \
    golang \
    git \
    procps \
    nginx \
    && rm -rf /var/lib/apt/lists/*
ADD . /tmp/skywalking
RUN set -ex \
    && cd /tmp/skywalking \
    && phpize && ./configure && make && make install \
    && ./build-sky-php-agent.sh \
    && cp sky-php-agent-* /usr/local/bin/ \
    && cp php.ini /usr/local/etc/php/conf.d/ext-skywalking.ini \
    && cp service.sh /opt/ \
    && cp nginx.conf /etc/nginx/nginx.conf \
    && cd /var/www/html \
    && rm -fr /tmp/skywalking

ENTRYPOINT ["/opt/service.sh"]