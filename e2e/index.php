<?php

require_once __DIR__ . '/vendor/autoload.php';
require_once __DIR__ . '/tests.php';

$ch = curl_init("https://api.github.com/repos");
curl_exec($ch);

testRedis();

testAddTag();

testAddLog();

testMysqli();