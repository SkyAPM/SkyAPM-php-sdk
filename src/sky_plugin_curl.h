/*
 * Copyright 2021 SkyAPM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef SKYWALKING_SKY_PLUGIN_CURL_H
#define SKYWALKING_SKY_PLUGIN_CURL_H

#include "php.h"
#include <curl/curl.h>

#if PHP_VERSION_ID >= 80000
#include "ext/curl/php_curl.h"
#endif

#define SKY_CURLOPT_HTTPHEADER 9923

void sky_plugin_curl_hooks();

ZEND_NAMED_FUNCTION(sky_curl_setopt_handler);

ZEND_NAMED_FUNCTION(sky_curl_setopt_array_handler);

ZEND_NAMED_FUNCTION(sky_curl_exec_handler);

ZEND_NAMED_FUNCTION(sky_curl_close_handler);

#endif //SKYWALKING_SKY_PLUGIN_CURL_H
