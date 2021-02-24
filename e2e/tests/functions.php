<?php

function testAddTag()
{
    $key = "key";
    $value = "value";
    skywalking_tag($key, $value);
}

function testAddLog(){
    $key = "info";
    $log = "test skywalking_log funciton, the third parameter [is_error] default is false.";
    skywalking_log($key, $log);
}
