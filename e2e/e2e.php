<?php

require_once __DIR__ . '/vendor/autoload.php';

class E2E {
//     public $url = "http://122.112.182.72:8080/graphql";
    public $url = "http://127.0.0.1:12800/graphql";
    public $services = <<<'GRAPHQL'
query queryServices($duration: Duration!,$keyword: String!) {
    services: searchServices(duration: $duration, keyword: $keyword) {
        key: id
        label: name
    }
}
GRAPHQL;

    public $startTime;

    public function __construct() {
        $this->startTime = date("Y-m-d His");
    }

    public function services() {
        $client = new \GuzzleHttp\Client();
        $res = $client->request("POST", $this->url, [
            'json' => [
                'query' => $this->services,
                'variables' => [
                    'duration' => [
                        "start" => $this->startTime,
                        "end" => date("Y-m-d His"),
                        "step" => "SECOND"
                    ],
                    "keyword" => ""
                ]
            ]
        ]);

        if ($res->getStatusCode() != 200) {
            return "";
        }

        return $res->getBody()->getContents();
    }

    public function info($msg) {
        echo chr(27) . "[1;34m" . $msg . chr(27) . "[0m\n";
    }

    public function call() {
        $ch = curl_init('http://127.0.0.1:8080/call');
        curl_exec($ch);
    }

    public function check_services() {
        $this->call();
        sleep(1);
        $res = $this->services();

        $this->info($res);
        if (!empty($res)) {
            $data = json_decode($res);
            if (count($data['data']['services']) <= 0) {
                return [[], false];
            }
            return [$data['data'], true];
        }

        return [[], false];
    }

}

$check = ['check_services'];
$e2e = new E2E();

foreach($check as $func) {
    $e2e->info('exec ' . $func);

    $status = false;
    for($i = 1; $i <= 10; $i++) {
         $e2e->info("test $func $i/10...");
         list($result, $status) = $e2e->$func();
         if ($status === true) {
             $status = true;
             break;
         }
         sleep(1);
    }

    if (!$status) {
        $e2e->info("test $func fail...");
        exit(2);
    }

    $e2e->info("test $func success...");
}
