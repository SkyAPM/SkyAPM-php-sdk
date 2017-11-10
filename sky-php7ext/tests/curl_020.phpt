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
  include 'server.inc';
  $host = curl_cli_server_start();

  $url = "{$host}/get.php?test=";
  $ch  = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_exec($ch);
  var_dump(curl_getinfo($ch, CURLINFO_HTTP_CODE));
  curl_close($ch);
?>
===DONE===
--EXPECTF--
Please use the command( tail -f *.log ) to view
Hello World!
Hello World!int(200)
===DONE===
