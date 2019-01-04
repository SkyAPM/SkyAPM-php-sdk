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
$headers = array(
    "Content-type: text/xml",
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
    "Cache-Control: no-cache",
    "Pragma: no-cache",
);
include 'server.inc';
$host = curl_cli_server_start();

// start testing
echo "*** skywalking extension is available ***\n";

$url = "{$host}/get.php?test=get";

$ch = curl_init();
curl_setopt($ch,CURLOPT_HTTPHEADER,$headers);
curl_setopt($ch, CURLOPT_HEADER, 0);
curl_setopt($ch, CURLOPT_COOKIELIST, 'Set-Cookie: C1=v1; expires=Thu, 31-Dec-2037 23:59:59 GMT; path=/; domain=.php.net');
curl_setopt($ch, CURLOPT_COOKIELIST, 'Set-Cookie: C2=v2; expires=Thu, 31-Dec-2037 23:59:59 GMT; path=/; domain=.php.net');
curl_setopt($ch, CURLOPT_URL, $url); //set the url we want to use
var_dump(curl_getinfo($ch, CURLINFO_COOKIELIST));
  $ok = curl_exec($ch);
  var_dump($ok);
?>
--EXPECT--
Please use the command( tail -f *.log ) to view
*** skywalking extension is available ***
array(2) {
  [0]=>
  string(38) ".php.net	TRUE	/	FALSE	2145916799	C1	v1"
  [1]=>
  string(38) ".php.net	TRUE	/	FALSE	2145916799	C2	v2"
}
Hello World!
Hello World!bool(true)