FROM php:7.4-fpm

RUN set -ex \
    && apt-get update -y && apt-get install --no-install-recommends -y \
    curl \
    libcurl4-openssl-dev \
    golang \
    git \
    nginx \
    && rm -rf /var/lib/apt/lists/*
ADD . /tmp/skywalking
RUN set -ex \
    && cd /tmp/skywalking \
    && phpize && ./configure && make && make install \
    && ./build-sky-php-agent.sh \
    && cp sky-php-agent-* /usr/local/bin/ \
    && cp php.ini /usr/local/etc/php/conf.d/ext-skywalking.ini \
    && mkdir -p /opt/www \
    && cp service.sh /opt/ \
    && cp nginx.conf /etc/nginx/nginx.conf \
    && cp skywalking.php /opt/www/index.php \
    && cd /var/www/html \
    && apt-get remove golang git \
    && rm -fr /tmp/skywalking \
    && rm -fr /usr/lib/go-1.11 \
    && rm -fr /root/go \
    && rm -fr /root/.cache

ENTRYPOINT ["/opt/service.sh"]
