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

#include "sky_plugin_yar.h"
#include "php_skywalking.h"
#include "zend_types.h"

#include "sky_core_segment.h"
#include "sky_utils.h"

std::vector<std::string> clientKeysCommands = {"__call"};
std::vector<std::string> serverKeysCommands = {"__construct"};
SkyCoreSpan *sky_plugin_yar_client(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
  std::string cmd = function_name;
  std::transform(function_name.begin(), function_name.end(), cmd.begin(), ::tolower);
  if (std::find(clientKeysCommands.begin(), clientKeysCommands.end(), cmd) != clientKeysCommands.end()) {
    auto *segment = sky_get_segment(execute_data, -1);
    if (segment) {
      auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::RPCFramework, 8002);
      span->setOperationName(class_name + "->" + function_name);
      span->addTag("yar.type", "client");

      zval *self = &(execute_data->This);
#if PHP_VERSION_ID < 80000
      zval *obj = self;
#else
      zend_object *obj = Z_OBJ_P(self);
#endif

      zval *zval_uri = sky_read_property(self, "_uri", 0);
      /* 检索uri属性的值 */
      if (zval_uri) {
        zval copy_zval_uri;
        ZVAL_DUP(&copy_zval_uri, zval_uri);
        span->addTag("yar.uri", ZSTR_VAL(Z_STR(copy_zval_uri)));

        php_url *url_parse = nullptr;
        url_parse = php_url_parse(ZSTR_VAL(Z_STR(copy_zval_uri)));
        if (url_parse != nullptr && url_parse->scheme != nullptr && url_parse->host != nullptr) {
#if PHP_VERSION_ID >= 70300
          char *php_url_scheme = ZSTR_VAL(url_parse->scheme);
        char *php_url_host = ZSTR_VAL(url_parse->host);
#else
          char *php_url_scheme = url_parse->scheme;
          char *php_url_host = url_parse->host;
#endif

          int peer_port;
          if (url_parse->port) {
            peer_port = url_parse->port;
          } else {
            if (strcasecmp("http", php_url_scheme) == 0) {
              peer_port = 80;
            } else {
              peer_port = 443;
            }
          }
          span->setPeer(std::string(php_url_host) + ":" + std::to_string(peer_port));
        }
        zval_dtor(&copy_zval_uri);
      }

      std::string sw_header = segment->createHeader(span);
      zval params[2];
      ZVAL_LONG(&params[0], YAR_OPT_HEADER);
      zval *yar_headers = (zval *) emalloc(sizeof(zval));
      bzero(yar_headers, sizeof(zval));
      array_init(yar_headers);
      add_next_index_string(yar_headers, ("sw8: " + sw_header).c_str());
      ZVAL_COPY(&params[1], yar_headers);
      zend_call_method(obj, Z_OBJCE_P(self), nullptr, ZEND_STRL("setopt"), nullptr, 2, &params[0], &params[1]);
      zval_dtor(&params[0]);
      zval_dtor(&params[1]);
      zval_dtor(yar_headers);
      efree(yar_headers);
      sw_header.shrink_to_fit();

      uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
      if (arg_count) {

        std::string command;
        for (uint32_t i = 1; i < arg_count + 1; ++i) {
          zval *p = ZEND_CALL_ARG(execute_data, i);
          if (Z_TYPE_P(p) == IS_ARRAY) {
            command = command.append(sky_json_encode(p));
            continue;
          }

          if (i == 1 && Z_TYPE_P(p) == IS_STRING) {
            span->addTag("yar.method", Z_STRVAL_P(p));
          }

          zval str_p;
          ZVAL_COPY(&str_p, p);
          if (Z_TYPE_P(&str_p) != IS_ARRAY && Z_TYPE_P(&str_p) != IS_STRING) {
            convert_to_string(&str_p);
          }

          std::string str = Z_STRVAL_P(&str_p);
          std::transform(str.begin(), str.end(), str.begin(), ::tolower);
          command = command.append(str);
          command.append(" ");
          str.clear();
          str.shrink_to_fit();
          zval_dtor(&str_p);
        }

        if (!command.empty()) {
          std::transform(command.begin(), command.end(), command.begin(), ::tolower);
          span->addTag("yar.call", command);
        }

        command.clear();
        command.shrink_to_fit();
      }
      return span;
    }
  }
  return nullptr;
}

SkyCoreSpan *sky_plugin_yar_server(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
  std::string cmd = function_name;
  std::transform(function_name.begin(), function_name.end(), cmd.begin(), ::tolower);
  if (std::find(serverKeysCommands.begin(), serverKeysCommands.end(), cmd) != serverKeysCommands.end()) {
    auto *segment = sky_get_segment(execute_data, -1);
    if (segment) {
      auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::RPCFramework, 8002);
      span->setOperationName(class_name + "->" + function_name);
      span->addTag("yar.type", "server");
      return span;
    }
  }
  return nullptr;
}