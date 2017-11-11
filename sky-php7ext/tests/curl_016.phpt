--TEST--
skywalking extension is available
--CREDITS--

--SKIPIF--
<?php if (!extension_loaded("skywalking")) print "skip"; ?>
--FILE--
<?php
  
  $logPath = ini_get("skywalking.log_path");
  $logFilename = strtolower($logPath . DIRECTORY_SEPARATOR .  'skywalking.' . date("Ymd") . '.log');
  echo "Please use the command( tail -f *.log ) to view\n";
  $ch   = curl_init();
  $info = curl_getinfo($ch);
  var_dump($info);
?>
===DONE===
--EXPECTF--
Please use the command( tail -f *.log ) to view
array(2%d) {
  [%u|b%"url"]=>
  string(0) ""
  ["content_type"]=>
  NULL
  ["http_code"]=>
  int(0)
  ["header_size"]=>
  int(0)
  ["request_size"]=>
  int(0)
  ["filetime"]=>
  int(0)
  ["ssl_verify_result"]=>
  int(0)
  ["redirect_count"]=>
  int(0)
  ["total_time"]=>
  float(0)
  ["namelookup_time"]=>
  float(0)
  ["connect_time"]=>
  float(0)
  ["pretransfer_time"]=>
  float(0)
  ["size_upload"]=>
  float(0)
  ["size_download"]=>
  float(0)
  ["speed_download"]=>
  float(0)
  ["speed_upload"]=>
  float(0)
  ["download_content_length"]=>
  float(%f)
  ["upload_content_length"]=>
  float(%f)
  ["starttransfer_time"]=>
  float(0)
  ["redirect_time"]=>
  float(0)
}
===DONE===
