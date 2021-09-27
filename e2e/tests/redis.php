<?php
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
function testRedis() {
    $redis = new Redis();
    $redis->connect('127.0.0.1', 6379);
    $redis->select(1);

    $key = 'skywalking';
    // strings
    $redis->append($key, "test");
    $redis->bitcount($key);
    $redis->bitcount($key, 0);
    $redis->bitcount($key, 0, 1);
    $redis->bitpos($key, 0, 1, 2);
    $redis->bitop('AND', $key, $key . '_01', $key . '_02', $key . '_03');
    $redis->setbit($key, 1, 1);
    $redis->setbit($key, 1, 0);
    $redis->setbit($key, 1, true);
    $redis->setbit($key, 1, false);
    $redis->getbit($key, 1);
    $redis->decr($key);
    $redis->get($key);
    $redis->getSet($key, "test");
    $redis->incr($key);
    $redis->incrBy($key, 10);
    $redis->decrBy($key, 10);
    $redis->incrByFloat($key, 10.5);
    $redis->setnx($key, "test");
    $redis->strlen($key);
    $redis->set($key, "test");
    $redis->set($key, "test", 100);
    $redis->set($key, "test", ["nx", "ex" => 200]);
    $redis->setex($key, 300, "test");
    $redis->setrange($key, 1, "222");
    $redis->getrange($key, 1, 4);

    // multiple keys
    $redis->mget([$key .'_1', $key . '_2', $key . '_3']);
    $redis->getMultiple([$key .'_1', $key . '_2', $key . '_3']);

    // multiple key-value
    $redis->mset([$key . '_1' => '111', $key . '_2' => '222', $key . '_3' => '333']);
    $redis->msetnx([$key . '_1' => '111', $key . '_2' => '222', $key . '_3' => '333']);

    // uncertain keys
    $redis->eval('return {1,2,3};');
    $redis->eval('return {1,2,3};', ['val_01', 'val_02']);
    $redis->eval('return {1,2,3};', [$key . '_01', $key . '_02', 'val_01', 'val_02'], 2);
    $redis->del($key);
    $redis->del($key, $key . '_1');
    $redis->del([$key, $key . '_1', $key . '_2']);
    $redis->delete($key);
    $redis->unlink($key);
    $redis->exists($key);

    // empty commands
    $redis->ping();
    $redis->pipeline();
    $redis->set($key, '1');
    $redis->set($key . '_1', '2');
    $redis->set($key . '_2', '3');
    $redis->exec();

    // expire
    $redis->expire($key, 3600);
    $redis->expireAt($key, time() + 3600);
    $redis->pExpireAt($key, time() + 3600);
    $redis->setTimeout($key, 3600);

    // sets
    $redis->sAdd($key, "member1");
    $redis->sAdd($key, "member2", "member3");
}
