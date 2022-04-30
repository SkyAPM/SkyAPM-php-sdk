#!/usr/bin/env sh
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
#

grpc=$SW_OAP_ADDRESS

if [ ! $grpc ]; then
    grpc="127.0.0.1:11800"
    sed -i "s/127.0.0.1:11800/$grpc/g" $PHP_INI_DIR/conf.d/ext-skywalking.ini
fi

echo "skywalking oap address:" $grpc

nginx

php-fpm
