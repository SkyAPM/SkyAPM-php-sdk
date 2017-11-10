<?php


/**
 * 其他项目调用此项目会传递类似 HEADER 信息
 * $_SERVER['HTTP_SWTRACECONTEXT'] = 'Segment1504173645811.59a7de4dc618f.14768.0.0.127.0.0.1|59a7de4dc61c5|api|baidu.com|Segment1504173645811.59a7de4dc61b0.14768.0.0.127.0.0.1|1';
 *
 */
include_once ("./SkyWalking.php");
SkyWalking::getInstance("mapi");

/**
 * 定义日志目录，目前传递数据使用log方式
 * 设置log写入路径的方法：
 */
define('LOG_PATH', dirname(__FILE__) . DIRECTORY_SEPARATOR . 'log');
SkyWalking::getInstance("api")->setLogPath(LOG_PATH)->setSamplingRate(5);


//发起B请求
sendA();
//发起B请求
sendB();
//发起B请求
sendC();


function sendA(){

    /** start 应用本身的业务代码 **/
    $headers = array(
        "Content-type: text/xml",
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
        "Cache-Control: no-cache",
        "Pragma: no-cache",
    );
    /** end 应用本身的业务代码 **/


    SkyWalking::getInstance()->startSpanOfCurl("api.com", $headers);
    //var_dump($headers);


    /** start 应用本身的业务代码 **/
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, 'http://www.api.com/');
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl,CURLOPT_HTTPHEADER,$headers);
    curl_setopt($curl, CURLOPT_HEADER, 0);
    $result = curl_exec($curl);
    $errno = curl_errno($curl);
    $error = curl_error($curl);
    /** end 应用本身的业务代码 **/


    SkyWalking::getInstance()->endSpanOfcurl($curl);


    /** start 应用本身的业务代码 **/
    curl_close($curl);
    /** end 应用本身的业务代码 **/
}


function sendB(){

    /** start 应用本身的业务代码 **/
    $headers = array(
        "Content-type: text/xml",
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
        "Cache-Control: no-cache",
        "Pragma: no-cache",
    );
    /** end 应用本身的业务代码 **/


    SkyWalking::getInstance()->startSpanOfCurl("api.com", $headers);
    //var_dump($headers);


    /** start 应用本身的业务代码 **/
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, 'http://www.api.com/');
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl,CURLOPT_HTTPHEADER,$headers);
    curl_setopt($curl, CURLOPT_HEADER, 0);
    $result = curl_exec($curl);
    $errno = curl_errno($curl);
    $error = curl_error($curl);
    /** end 应用本身的业务代码 **/


    SkyWalking::getInstance()->endSpanOfcurl($curl);


    /** start 应用本身的业务代码 **/
    curl_close($curl);
    /** end 应用本身的业务代码 **/
}

function sendC(){

    /** start 应用本身的业务代码 **/
    $headers = array(
        "Content-type: text/xml",
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
        "Cache-Control: no-cache",
        "Pragma: no-cache",
    );
    /** end 应用本身的业务代码 **/


    SkyWalking::getInstance()->startSpanOfCurl("api.com", $headers);
    //var_dump($headers);


    /** start 应用本身的业务代码 **/
    /** start 应用本身的业务代码 **/
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, 'http://www.api.com/');
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl,CURLOPT_HTTPHEADER,$headers);
    curl_setopt($curl, CURLOPT_HEADER, 0);
    $result = curl_exec($curl);
    $errno = curl_errno($curl);
    $error = curl_error($curl);
    /** end 应用本身的业务代码 **/


    SkyWalking::getInstance()->endSpanOfcurl($curl);


    /** start 应用本身的业务代码 **/
    curl_close($curl);
    /** end 应用本身的业务代码 **/
}