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
namespace Predis\Connection {

    class Parameters {
        protected $parameters;
        public function __construct($parameters) {
            $this->parameters = $parameters;
        }
    }

    abstract class AbstractConnection {
        protected $parameters;

        public function __construct($parameters) {
            $this->parameters = $parameters;
        }
    }
    class StreamConnection extends AbstractConnection {
        public function __construct($parameters) {
            parent::__construct(new Parameters($parameters));
        }
    }
}

namespace Predis\Command {
    interface CommandInterface {
        public function getId();
        public function getArguments();
    }

    class TestCommand implements CommandInterface {
        public $commandID;
        public $arguments;
        public function __construct($commandID, $arguments) {
            $this->commandID = $commandID;
            $this->arguments = $arguments;
        }

        public function getId() {
            return $this->commandID;
        }

        public function getArguments() {
            return $this->arguments;
        }
    }
}

namespace Predis {
    class Client {

        protected $connection;

        public function __construct($parameters = null) {
            $this->connection = new \Predis\Connection\StreamConnection($parameters);
        }

        public function __call($commandID, $arguments) {
            return $this->executeCommand(new \Predis\Command\TestCommand($commandID, $arguments));
        }

        public function executeCommand(\Predis\Command\CommandInterface $command) {

        }
    }
}

namespace Grpc {
    class BaseStub {
        private $hostname;
        private $hostname_override;

        public function __construct($hostname) {
            $this->hostname = $hostname;
        }

        public function _simpleRequest($method) {}
    }

    class HelloClient extends BaseStub {

        public function __construct($hostname) {
            parent::__construct($hostname);
        }

        public function hello() {
            $this->_simpleRequest("user.mock");
        }
    }
}

namespace PhpAmqpLib\Connection {

    class AbstractConnection {
        protected $construct_params;
    }
    class AMQPStreamConnection extends AbstractConnection {


        public function __construct($host, $port) {
            $this->construct_params = func_get_args();
        }
    }
}

namespace PhpAmqpLib\Channel {
    class AbstractChannel {
        protected $connection;
        public function __construct($connection) {
            $this->connection = $connection;
        }
    }
    class AMQPChannel extends AbstractChannel {

        public function __construct($connection, $channel_id = null, $auto_decode = true, $channel_rpc_timeout = 0) {
            parent::__construct($connection);
        }

        public function basic_publish($msg, $exchange = '') {
        }

    }
}

namespace {
    $channel = new \PhpAmqpLib\Channel\AMQPChannel(new \PhpAmqpLib\Connection\AMQPStreamConnection("127.0.0.1", 2222));
    $channel->basic_publish("test", "exchange");
    $client = new \Predis\Client(["host" => "127.0.0.1", "port" => 6379]);
    $client->set('foo', 'bar');
    $client->get('foo');

    // test grpc
    $hello = new \Grpc\HelloClient("127.0.0.1:8888");
    $hello->hello();
    var_dump(skywalking_trace_id());

    skywalking_tag("test", "foo", "bar");
    skywalking_log("test", 'info', 'skywalking custom log function skywalking_log(string $key, string $msg[, bool $is_error])', false);
//
//     // test pdo
//     $dbh = new \PDO("mysql:host=127.0.0.1;port=3306;dbname=mock", "root", "111111");
//     $dbh->exec("SET names utf8");
//     $dbh->query("select * from mock");
//     $dbh->prepare("select * from mock where id = ?");
//
//     $dbh->beginTransaction();
//     $dbh->query("select * from mock");
//     $dbh->commit();

    // test curl
    // $ch = curl_init("https://api.github.com/repos");
    // curl_setopt($ch, 9923, []);
    // curl_exec($ch);
    // $ch = curl_init("https://api.github.com/repos");
    // curl_exec($ch);
}
?>
