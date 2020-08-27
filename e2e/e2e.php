<?php

require_once __DIR__ . '/vendor/autoload.php';

function info($msg) {
    echo chr(27) . "[1;34m" . $msg . chr(27) . "[0m\n";
}

function call() {
    $ch = curl_init('http://127.0.0.1:8080/call');
    curl_exec($ch);
}

function check_xx() {
    call();
    sleep(1);
    // todo graphql
    return [0, false];
}

$check = ['check_xx'];

foreach($check as $func) {
    info('exec ' . $func);

    list($result, $status) = $func();
    
    $status = false;
    for($i = 1; $i <= 10; $i++) {
         info("test $func $i/10...");
         list($result, $status) = $func();
         if ($status === true) {
             $status = true;
             break;
         }
    }

    if (!$status) {
        info("test $func fail...");
        exit(2);
    }
}
