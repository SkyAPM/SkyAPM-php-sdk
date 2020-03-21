#!/usr/bin/env bash

nginx

php-fpm > /var/log/php-fpm.log 2>&1 &

sky-php-agent-linux-x64 --grpc ${SW_AGENT_COLLECTOR_BACKEND_SERVICES} --socket /tmp/sky-agent.sock > /var/log/sky-php.log 2>&1 &

while [[ true ]]; do
    sleep 1
done