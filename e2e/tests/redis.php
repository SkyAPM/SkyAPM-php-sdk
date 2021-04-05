<?php

function testRedis() {
    $redis = new Redis();
    $redis->connect('127.0.0.1', 6379);

    $key = 'skywalking';
    // strings
    $redis->append($key, "test");
    $redis->bitcount($key);
    $redis->bitcount($key, 0);
    $redis->bitcount($key, 0, 1);
    $redis->decr($key);
    $redis->get($key);
    $redis->getSet($key, "test");
    $redis->incr($key);
    $redis->setnx($key, "test");
    $redis->strlen($key);
    $redis->set($key, "test");
    $redis->set($key, "test", 100);
    $redis->set($key, "test", ["nx", "ex" => 200]);
    $redis->setex($key, 300, "test");

    // multiple keys
    $redis->mget([$key .'_1', $key . '_2', $key . '_3']);
    $redis->getMultiple([$key .'_1', $key . '_2', $key . '_3']);

    // uncertain keys
    $redis->eval('return {1,2,3};');
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
}
