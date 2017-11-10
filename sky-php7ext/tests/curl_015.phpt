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
  $url = 'http://www.example.com/'; 
  $ch  = curl_init($url);
  var_dump($url == curl_getinfo($ch, CURLINFO_EFFECTIVE_URL));
?>
===DONE===
--EXPECTF--
Please use the command( tail -f *.log ) to view
bool(true)
===DONE===
