#!/usr/bin/env php
<?php

// check file lists
$version = "";
$root_dir = dirname(__FILE__);
$file_list_raw = explode(PHP_EOL, `git ls-files`);

$header = file_get_contents("$root_dir/php_skywalking.h");

if(preg_match("/PHP_SKYWALKING_VERSION \"(\d.\d.\d)\"/i", $header, $matches)) {
    if(isset($matches[1])) {
        $version = $matches[1];
    }
}

if ($version === "") {
    exit("❌No version number found");
}

echo "version: $version\n";

$file_list = [];
foreach ($file_list_raw as $file) {
    if (empty($file)) {
        continue;
    }
    if (is_dir("{$root_dir}/{$file}")) {
        continue;
    }
    if ($file === 'package.xml' || substr($file, 0, 1) === '.' || $file === 'package-template.xml') {
        continue;
    }
    if (strpos($file, 'tests') === 0) {
        $role = 'test';
    } elseif (strpos($file, 'examples') === 0) {
        $role = 'doc';
    } else {
        $ext = pathinfo($file, PATHINFO_EXTENSION);
        $role = 'src';
        switch ($ext) {
            case 'phpt':
                $role = 'test';
                break;
            case 'md':
                $role = 'doc';
                break;
            case '':
                static $spacial_source_list = [
                    'Makefile' => true
                ];
                if ($spacial_source_list[pathinfo($file, PATHINFO_BASENAME)] ?? false) {
                    break;
                }
                if (substr(file_get_contents("{$root_dir}/{$file}"), 0, 2) !== '#!') {
                    $role = 'doc';
                }
                break;
        }
    }
    $file_list[] = "<file role=\"{$role}\" name=\"{$file}\" />\n";
}

$template = file_get_contents(__DIR__ . '/package-template.xml');
$template = str_replace("{{file_list}}", implode("            ", $file_list), $template);
$template = str_replace("{{version}}", $version, $template);
$template = str_replace("{{date}}", date("Y-m-d"), $template);
file_put_contents("package.xml", $template);


