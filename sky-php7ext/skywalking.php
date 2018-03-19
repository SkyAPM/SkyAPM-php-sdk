<?php

$file = new SplFileObject('appid.pid', "w");
$written = $file->fwrite("124124");



 $file = new SplFileObject('appid.pid');
$aa = $file->current();

var_dump($aa);
exit;
function getfiles($path){ 
    foreach(scandir($path) as $afile)
    {
        if($afile=='.'||$afile=='..') continue; 
        if(is_file($path.'/'.$afile)){
        
            echo $afile. "\n"; 
        } 
    } 
} //简单的demo,列出当前目录下所有的文件
getfiles(__DIR__);
 
exit;
var_dump(base_convert(uniqid(), 16, 10));
getMillisecond();
function getMillisecond()
    {
        list($t1, $t2) = explode(' ', microtime());
        /*$t3 = $t1 * 100000000;
        var_dump($t2, substr($t2, -6));
        $aa = (float)($t3 . substr($t2, -6));
        var_dump( (float)($t3 . substr($t2, -6)));
        var_dump( json_encode(array("aaa"=>  $aa)));
        exit;*/
        $millisecond = (float)sprintf('%.0f', (floatval($t1) + floatval($t2)) * 10000);
         var_dump( json_encode(array("aaa"=>  $millisecond)));
        var_dump( $millisecond);
        return $millisecond;
    }
    exit;
var_dump(json_encode(["applicationCode"=>"mapi"]));exit;
exit;
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('skywalking')) {
	dl('skywalking.' . PHP_SHLIB_SUFFIX);
}
$module = 'skywalking';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
echo "$str\n";
?>
