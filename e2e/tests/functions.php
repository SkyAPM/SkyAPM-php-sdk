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
function testAddTag()
{
    $key = "key";
    $value = "value";
    skywalking_tag(__METHOD__, $key, $value);
}

function testAddLog(){
    $key = "info";
    $log = "test skywalking_log funciton, the third parameter [is_error] default is false.";
    skywalking_log(__METHOD__, $key, $log);
    skywalking_log(__METHOD__, $key, $log, 1);
}
