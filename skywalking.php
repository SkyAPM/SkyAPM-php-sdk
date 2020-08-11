<?php

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
            $this->connection = new \Predis\Connection\StreamConnection([
                "host" => "127.0.0.1",
                "port" => 6379
            ]);
        }

        public function __call($commandID, $arguments) {
            return $this->executeCommand(new \Predis\Command\TestCommand($commandID, $arguments));
        }

        public function executeCommand(\Predis\Command\CommandInterface $command) {

        }
    }
}

namespace {
    $client = new \Predis\Client();
    $client->set('foo', 'bar');
    $client->get('foo');
}


// $ch = curl_init("https://api.github.com/repos");
// curl_setopt($ch, 9923, []);
// curl_exec($ch);
/*
namespace Grpc;

class BaseStub {
    private $hostname;

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
        $this->_simpleRequest("user.aa");
    }
}

$hello = new HelloClient("127.0.0.1:8888");
$hello->hello();
// $ch = curl_init("https://api.github.com/repos");
// curl_exec($ch);
*/
?>
