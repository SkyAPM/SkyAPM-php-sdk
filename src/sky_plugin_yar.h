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
#define YAR_OPT_HEADER	(1<<4)

Span *sky_plugin_yar_client(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

Span *sky_plugin_yar_server(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

#endif //SKYWALKING_SRC_SKY_PLUGIN_YAR_H_