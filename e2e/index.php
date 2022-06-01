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
require_once __DIR__ . '/vendor/autoload.php';
require_once __DIR__ . '/tests.php';

$ch = curl_init("https://api.github.com/repos");
curl_exec($ch);
echo("curl success\n");

testRedis();
echo("redis success\n");

testAddTag();
echo("tag success\n");

testAddLog();
echo("log success\n");

//testMysqli();
echo("mysqli success\n");

//testMemcached();
echo("memcached success\n");

//testYar();
echo("yar success\n");