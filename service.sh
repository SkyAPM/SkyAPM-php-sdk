#!/usr/bin/env sh

nginx

php-fpm > /var/log/php-fpm.log 2>&1 &

sky-php-agent --grpc ${SW_AGENT_COLLECTOR_BACKEND_SERVICES} --socket /tmp/sky-agent.sock > /var/log/sky-php.log 2>&1 &

while [[ true ]]; do
    sleep 1
done
