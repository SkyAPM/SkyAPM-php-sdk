//
// Created by kilingzhang on 2021/6/24.
//

#ifndef SKYWALKING_SRC_SKY_PLUGIN_MEMCACHED_H_
#define SKYWALKING_SRC_SKY_PLUGIN_MEMCACHED_H_

#include "php_skywalking.h"
#include "sky_utils.h"
#include <string>
#include <functional>
#include <map>
#include "span.h"
#include "segment.h"

Span *sky_plugin_memcached(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

std::string sky_plugin_memcached_peer(zend_execute_data *execute_data);

std::string sky_plugin_memcached_key_cmd(zend_execute_data *execute_data, std::string cmd);
#endif //SKYWALKING_SRC_SKY_PLUGIN_MEMCACHED_H_
