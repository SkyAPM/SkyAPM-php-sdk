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
curl_setopt($ch, CURLOPT_COOKIELIST, 'Set-Cookie: C1=v1; expires=Thu, 31-Dec-2037 23:59:59 GMT; path=/; domain=.php.net');
curl_setopt($ch, CURLOPT_COOKIELIST, 'Set-Cookie: C2=v2; expires=Thu, 31-Dec-2037 23:59:59 GMT; path=/; domain=.php.net');
var_dump(curl_getinfo($ch, CURLINFO_COOKIELIST));

?>
--EXPECT--
Please use the command( tail -f *.log ) to view
array(2) {
  [0]=>
  string(38) ".php.net	TRUE	/	FALSE	2145916799	C1	v1"
  [1]=>
  string(38) ".php.net	TRUE	/	FALSE	2145916799	C2	v2"
}
