<?php
/*
 * Copyright 2021 SkyAPM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
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

    public $instanceQuery = <<<'GRAPHQL'
query queryInstances($serviceId: ID!, $duration: Duration!) {
    instances: getServiceInstances(duration: $duration, serviceId: $serviceId) {
        key: id
        label: name
        attributes {
            name
            value
        }
        instanceUUID
    }
}
GRAPHQL;


    public $startTime;

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

        $json = [
            'query' => $query,
            'variables' => $variables
        ];

        $this->info("query request body: " . json_encode($json));
        $res = $client->request("POST", $this->url, [
            'json' => $json
        ]);

        $this->info("query response status code: " . $res->getStatusCode());
        if ($res->getStatusCode() != 200) {
            return "";
        }

        $body = $res->getBody()->getContents();
        $this->info("query response body: " . $body);
        return $body;
    }

    public function info($msg) {
        echo chr(27) . "[1;34m" . $msg . chr(27) . "[0m\n";
    }

    public function call() {
        $ch = curl_init('http://127.0.0.1:8083/call');
        curl_exec($ch);
        if (curl_getinfo($ch, CURLINFO_HTTP_CODE) != 200) {
            return false;
        }
        return true;
    }

    public function verifyServices() {
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

            $found = false;
            foreach ($data['data']['services'] as $service) {

                if ($service['label'] == "skywalking") {
                    $found = true;
                    if (!$this->verifyServiceMetrics($service)) {
                        return false;
                    }

                    $instances = $this->verifyServiceInstances($service);
                    if ($instances === false) {
                        return false;
                    }
                }
            }
            if (!$found) {
                return false;
            }
            return true;
        }

        return false;
    }

    public function verifyServiceMetrics($service) {

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

        return true;
    }

    public function verifyServiceInstances($service) {

        $key = $service['key'];
        $label = $service['label'];
        $this->info("verifying instance ($key:$label)");

        $variables = [
            'duration' => [
                "start" => date("Y-m-d His", $this->startTime),
                "end" => date("Y-m-d His"),
                "step" => "SECOND"
            ],
            "serviceId" => $key
        ];

        $res = $this->query($this->instanceQuery, $variables);
        if (!empty($res)) {
            $data = json_decode($res, true);

            if (count($data['data']['instances']) > 0) {
                foreach ($data['data']['instances'] as $instance) {
                    $status = $instance['key'] == '' or $instance['label'] == '';
                    if ($status) {
                        return false;
                    } else {
                        foreach ($instance['attributes'] as $attr) {
                            if ($attr['value'] == '') {
                                return false;
                            }
                        }
                    }
                }
                return $data['data']['instances'];
            }
            return false;
        } else {
            return false;
        }
    }
}

$check = ['verifyServices'];
$e2e = new E2E();

foreach($check as $func) {
    $e2e->info('php version:' . $argv[1]);
    $e2e->info('exec ' . $func);

    $status = false;
    for($i = 1; $i <= 10; $i++) {
         $e2e->info("test $func $i/10...");
         $status = $e2e->call();
         if (!$status) {
            break;
         }
         sleep(5);
         $status = $e2e->$func();
         if ($status === true) {
             $status = true;
             break;
         }
         sleep(10);
    }

    if (!$status) {
        $e2e->info("test $func fail...");
        echo(file_get_contents("/var/log/php" . $argv[1] . "-fpm.log"));
        system("sudo chmod -R +rwx /var/crash/*");
        exit(2);
    }

    $e2e->info("test $func success...");
}
