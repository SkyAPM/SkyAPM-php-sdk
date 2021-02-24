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

    $key = "info";
    $log = "test skywalking_log funciton, is_error set to true";
    $is_error = true;
    skywalking_log($key, $log, $is_error);

}
