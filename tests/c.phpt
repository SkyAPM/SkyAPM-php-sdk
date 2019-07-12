<?php
echo ":::bengin:::\n";
$curl = curl_init("http://192.168.88.132/");
$headers = array(
	"Content-type: text/xml",
	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
	"Cache-Control: no-cache",
	"Pragma: no-cache",
);

curl_setopt($curl, CURLOPT_URL, "http://192.168.88.132/");
//curl_setopt($curl,CURLOPT_HTTPHEADER,$headers);
$aa = curl_exec($curl);
//var_dump($aa);
 $info = curl_getinfo($curl);
 echo ":::end:::\n"
?>
