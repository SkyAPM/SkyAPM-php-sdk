// contributor license agreements.  See the NOTICE file distributed with
// this work for additional information regarding copyright ownership.
// The ASF licenses this file to You under the Apache License, Version 2.0
// (the "License"); you may not use this file except in compliance with
// the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef SKYWALKING_SKY_REDIS_H
#define SKYWALKING_SKY_REDIS_H

#include "php_skywalking.h"
#include "sky_utils.h"
#include <string>
#include "span.h"
#include "segment.h"

Span *sky_redis(zend_execute_data *execute_data, char *class_name, char *function_name);

#define REDIS_KEY_KEY "|dump|exists|expire|expireat|move|persist|pexpire|pexpireat|pttl|rename|renamenx|sort|ttl|type|"
#define REDIS_KEY_STRING "|append|bitcount|bitfield|decr|decrby|get|getbit|getrange|getset|incr|incrby|incrbyfloat|psetex|set|setbit|setex|setnx|setrange|strlen|"
#define REDIS_OPERATION_STRING "|bitop|"
#define REDIS_KEY_HASH "|hdel|hexists|hget|hgetall|hincrby|hincrbyfloat|hkeys|hlen|hmget|hmset|hscan|hset|hsetnx|hvals|hstrlen|"
#define REDIS_KEY_LIST "|lindex|linsert|llen|lpop|lpush|lpushx|lrange|lrem|lset|ltrim|rpop|rpush|rpushx|"
#define REDIS_KEY_SET "|sadd|scard|sismember|smembers|spop|srandmember|srem|sscan|"
#define REDIS_KEY_SORT "|zadd|zcard|zcount|zincrby|zrange|zrangebyscore|zrank|zrem|zremrangebyrank|zremrangebyscore|zrevrange|zrevrangebyscore|zrevrank|zscore|zscan|zrangebylex|zrevrangebylex|zremrangebylex|zlexcount|"
#define REDIS_KEY_HLL "|pfadd|watch|"
#define REDIS_KEY_GEO "|geoadd|geohash|geopos|geodist|georadius|georadiusbymember|"

int sky_redis_opt_for_string_key(char *fnamewall);
char *sky_redis_fnamewall(const char *function_name);
std::string sky_redis_peer(zend_execute_data *execute_data);
void sky_redis_command(Span *span, zend_execute_data *execute_data, char *function_name);

#endif // SKYWALKING_SKY_REDIS_H