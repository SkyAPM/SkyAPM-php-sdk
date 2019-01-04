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
  var_dump($ch);
?>
===DONE===
--EXPECTF--
Please use the command( tail -f *.log ) to view
resource(%d) of type (curl)
===DONE===
