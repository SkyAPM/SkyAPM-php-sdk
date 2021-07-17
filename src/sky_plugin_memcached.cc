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
#include "sky_plugin_memcached.h"
#include "php_skywalking.h"

#include "segment.h"
#include "sky_utils.h"

std::vector<std::string> mecKeysCommands = {"set", "setbykey", "setmulti", "setmultibykey", "add", "addbykey", "replace", "replacebykey",
    "append", "appendbykey", "prepend", "prependbykey", "cas", "casbykey", "get", "getbykey",
    "getmulti", "getmultibykey", "getallkeys", "delete", "deletebykey", "deletemulti",
    "deletemultibykey", "increment", "incrementbykey", "decrement", "decrementbykey", "getstats",
    "ispersistent", "ispristine", "flush", "flushbuffers", "getdelayed", "getdelayedbykey", "fetch",
    "fetchall", "addserver", "addservers", "getoption", "setoption", "setoptions", "getresultcode",
    "getserverlist", "resetserverlist", "getversion", "quit", "setsaslauthdata", "touch",
    "touchbykey"};;
std::vector<std::string> mecStrKeysCommands = {"set", "setbykey", "setmulti", "setmultibykey", "add", "addbykey", "replace", "replacebykey",
    "append", "appendbykey", "prepend", "prependbykey", "cas", "casbykey", "get", "getbykey",
    "getmulti", "getmultibykey", "getallkeys", "delete", "deletebykey", "deletemulti",
    "deletemultibykey", "increment", "incrementbykey", "decrement", "decrementbykey"};

Span *sky_plugin_memcached(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
  std::string cmd = function_name;
  std::transform(function_name.begin(), function_name.end(), cmd.begin(), ::tolower);
  if (std::find(mecKeysCommands.begin(), mecKeysCommands.end(), cmd) != mecKeysCommands.end()) {
    auto *segment = sky_get_segment(execute_data, -1);
    if (segment) {
      auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Cache, 36);
      span->setOperationName(class_name + "->" + function_name);
      span->addTag("db.type", "memcached");

      // peer
      if (std::find(mecStrKeysCommands.begin(), mecStrKeysCommands.end(), cmd) != mecStrKeysCommands.end()) {
        auto peer = sky_plugin_memcached_peer(execute_data);
        if (!peer.empty()) {
          span->setPeer(peer);
        }
      }

      span->addTag("memcached.command", sky_plugin_memcached_key_cmd(execute_data, cmd));
      return span;
    }
  }
  return nullptr;
}

std::string sky_plugin_memcached_peer(zend_execute_data *execute_data) {
  std::string peer;
  zval *p = ZEND_CALL_ARG(execute_data, 1);
  zval str_p;
  ZVAL_COPY(&str_p, p);
  std::string str = Z_STRVAL_P(&str_p);
  zval *self = &(execute_data->This);
#if PHP_VERSION_ID < 80000
  zval *obj = self;
#else
  zend_object *obj = Z_OBJ_P(self);
#endif
  zval server;
  zval params[1];
  ZVAL_STRING(&params[0], str.c_str());
  zend_call_method(obj, Z_OBJCE_P(self), nullptr, ZEND_STRL("getserverbykey"), &server, 1, params, nullptr);
  zval *str_zval;
  long long port = 0;
  str_zval = zend_hash_str_find(Z_ARRVAL_P(&server), "host", 4);
  std::string host = Z_STRVAL_P(str_zval);
  str_zval = zend_hash_str_find(Z_ARRVAL_P(&server), "port", 4);
  port = Z_LVAL_P(str_zval);
  peer = host + ":" + std::to_string(port);
  host.clear();
  host.shrink_to_fit();
  zval_dtor(&server);
  zval_dtor(&params[0]);
  zval_dtor(str_zval);
  str.clear();
  str.shrink_to_fit();
  zval_dtor(&str_p);
  return peer;
}

std::string sky_plugin_memcached_key_cmd(zend_execute_data *execute_data, std::string cmd) {
  uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
  std::string command = cmd;
  for (uint32_t i = 1; i < arg_count + 1; ++i) {
    command = command.append(" ");
    zval *p = ZEND_CALL_ARG(execute_data, i);
    if (Z_TYPE_P(p) == IS_ARRAY) {
      command = command.append(sky_json_encode(p));
      continue;
    }

    zval str_p;

    ZVAL_COPY(&str_p, p);
    if (Z_TYPE_P(&str_p) != IS_ARRAY && Z_TYPE_P(&str_p) != IS_STRING) {
      convert_to_string(&str_p);
    }

    std::string str = Z_STRVAL_P(&str_p);

    command = command.append(str);
    str.clear();
    str.shrink_to_fit();
    zval_dtor(&str_p);
  }

  if (!command.empty()) {
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);
  }
  return command;
}
