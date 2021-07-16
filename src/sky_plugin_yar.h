//
// Created by kilingzhang on 2021/6/24.
//

#ifndef SKYWALKING_SRC_SKY_PLUGIN_YAR_H_
#define SKYWALKING_SRC_SKY_PLUGIN_YAR_H_

#include "php_skywalking.h"
#include "sky_utils.h"
#include <string>
#include <functional>
#include <map>
#include "span.h"
#include "segment.h"

Span *sky_plugin_yar(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

std::string sky_plugin_yar_peer(zend_execute_data *execute_data);

#endif //SKYWALKING_SRC_SKY_PLUGIN_YAR_H_
