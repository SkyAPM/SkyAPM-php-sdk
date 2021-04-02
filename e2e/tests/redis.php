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
}
