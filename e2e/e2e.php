<?php

require_once __DIR__ . '/vendor/autoload.php';

class E2E {
//     public $url = "http://122.112.182.72:8080/graphql";
    public $url = "http://127.0.0.1:12800/graphql";
    public $servicesQuery = <<<'GRAPHQL'
query queryServices($duration: Duration!,$keyword: String!) {
    services: searchServices(duration: $duration, keyword: $keyword) {
        key: id
        label: name
    }
}
GRAPHQL;

    public $metricsQuery = <<<'GRAPHQL'
query ($id: ID!, $duration: Duration!) {
    metrics: getLinearIntValues(metric: {
        name: "{metricsName}"
        id: $id
    }, duration: $duration) {
       values {
           value
       }
    }
}
GRAPHQL;

    public $startTime;

    public $services;

    public $allServiceMetrics = [
        'service_sla',
        'service_cpm',
        'service_resp_time',
        'service_apdex'
    ];

    public function __construct() {
        $this->startTime = time() - 15 * 60;
    }

    public function query($query, $variables) {
        $client = new \GuzzleHttp\Client();
        $res = $client->request("POST", $this->url, [
            'json' => [
                'query' => $query,
                'variables' => $variables
            ]
        ]);

        $this->info("query response status code: " . $res->getStatusCode());
        if ($res->getStatusCode() != 200) {
            return "";
        }

        $body = $res->getBody()->getContents();
        $this->info($body);
        return $body;
    }

    public function info($msg) {
        echo chr(27) . "[1;34m" . $msg . chr(27) . "[0m\n";
    }

    public function call() {
        $ch = curl_init('http://127.0.0.1:8080/call');
        curl_exec($ch);
    }

    public function verifyServices() {
        $this->call();
        sleep(1);
        $variables = [
            'duration' => [
                "start" => date("Y-m-d His", $this->startTime),
                "end" => date("Y-m-d His"),
                "step" => "SECOND"
            ],
            "keyword" => ""
        ];
        $res = $this->query($this->servicesQuery, $variables);
        if (!empty($res)) {
            $data = json_decode($res, true);
            if (count($data['data']['services']) <= 0) {
                return false;
            }
            $this->services = $data['data']['services'];
            // todo
            return true;
        }

        return false;
    }

    public function verifyServiceMetrics() {

        foreach ($this->services as $service) {
            foreach ($this->allServiceMetrics as $metrics) {
                $key = $service['key'];
                $label = $service['label'];
                $this->info("verifying service ($key:$label), metrics: $metrics");

                $variables = [
                    'duration' => [
                        "start" => date("Y-m-d Hi", $this->startTime),
                        "end" => date("Y-m-d Hi"),
                        "step" => "MINUTE"
                    ],
                    "id" => $key
                ];
                $query = str_replace("{metricsName}", $metrics, $this->metricsQuery);

                $res = $this->query($query, $variables);
                if (!empty($res)) {
                    $data = json_decode($res, true);
                    if (count($data['data']['metrics']['values']) > 0) {
                        $check = false;
                        foreach ($data['data']['metrics']['values'] as $item) {
                            if ($item['value'] > 0) {
                                $check = true;
                                break;
                            }
                        }
                        if (!$check) {
                            return false;
                        }
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }
        return true;
    }

}

$check = ['verifyServices', 'verifyServiceMetrics'];
$e2e = new E2E();

foreach($check as $func) {
    $e2e->info('exec ' . $func);

    $status = false;
    for($i = 1; $i <= 10; $i++) {
         $e2e->info("test $func $i/10...");
         $status = $e2e->$func();
         if ($status === true) {
             $status = true;
             break;
         }
         sleep(1);
    }

    if (!$status) {
        $e2e->info("test $func fail...");
        exit(0);
    }

    $e2e->info("test $func success...");
}
