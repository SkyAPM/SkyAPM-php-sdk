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
$url = "http://www.example.org";
$ch = curl_init();
curl_setopt($ch, CURLOPT_PROXY, uniqid().":".uniqid());
curl_setopt($ch, CURLOPT_URL, $url);

curl_exec($ch);
var_dump(curl_error($ch));
var_dump(curl_errno($ch));
curl_close($ch);


?>
--EXPECTF--
Please use the command( tail -f *.log ) to view
%unicode|string%(%d) "%r(Couldn't resolve proxy|Could not resolve proxy:|Could not resolve host:|Could not resolve:)%r %s"
int(5)
