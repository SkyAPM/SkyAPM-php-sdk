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
$ch = curl_init();

curl_exec($ch);
var_dump(curl_error($ch));
var_dump(curl_errno($ch));
curl_close($ch);


?>
--EXPECTF--
Please use the command( tail -f *.log ) to view
%string|unicode%(%d) "No URL set!%w"
int(3)
