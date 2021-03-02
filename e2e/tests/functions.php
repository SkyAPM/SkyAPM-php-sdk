<?php

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
