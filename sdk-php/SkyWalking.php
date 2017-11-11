<?php

/**
 * Class SkyWalking
 * 需要 设置 LOG_PATH 常量 默认会放到/tmp/目录下
 *
 * @package SkyWalking
 */
class SkyWalking
{
    /**
     * 当前类对象单列
     * @var new self
     */
    private static $_instance = null;


    /**
     * 当前应用系统名称
     * @var $_appCode
     */
    private static $_appCode = null;
    /**
     * SkyWalking 包含的头信息
     * @var array
     */
    private $_swHeaderInfo = array();
    /**
     * 唯一的事务编号
     * @var
     */
    private $_traceId;
    private $_distributedTraceIds;
    /**
     * span节点的集合 数组
     * 接口的 ss 列
     * @var array
     */
    private $_spansNodeData = array();
    /**
     * 总节点数据
     * @var array
     */
    private $_allNodeData = array();
    /**
     * 父节点数据
     * @var array
     */
    private $_fatherNodeData = array();
    /**
     * 单个span节点数据
     * @var array
     */
    private $_spanNodeData = array();
    /**
     * 第一个span节点数据
     * @var array
     */
    private $_spanFirstNodeData = array();
    /**
     * 生产的SPAN_ID
     * @var
     */
    private $_spanID = 0;
    private $_logPath;

    /**
     * 采样率，默认为100%
     * @var int
     */
    private $_samplingRate = null;


    private $_swHeaderText = null;

    /**
     * 本次进程是否采样
     */
    private $_isSampling = null;

    /**
     * 定义替换简单字符常量
     */
    const TRACEID = 'ts';   //TraceId
    const STARTTIME = 'st'; //开始时间
    const ENDTIME = 'et';  //结束时间
    const APP_CODE = 'ac'; // App Code
    const FATHER_NODE_DATA = 'rs'; //父节点数据
    const SPANS_NODE_DATA = 'ss'; //span节点数据集合
    const DISTRIBUTED_TRACEIDS = 'gt';//DistributedTraceIds
    const SPAN_ID = 'si'; //SpanId
    const PEERHOST = 'ph'; //PeerHost
    const FATHER_SPAN_ID = 'ps'; //父节点传过来的SpanId
    const SPAN_SERVER_URI = 'on';// Span 的服务URI
    const SPAN_STRING_PARAM = 'ts';//Span 的字符串型参数
    const SPAN_BOOL_PARAM = 'tb';//Span 的字符串型参数
    const SPAN_INT_PARAM = 'ti';//Span 的字符串型参数
    const SPAN_LOG = 'lo'; //Span 的日志

    /**
     * 全部节点结构
     *
     *   TRACEID["ts": "Segment.1490097253214.-866187727.57515.1.1"],  --TraceId
     *   STARTTIME["st": 1490097253198],                 --开始时间，整个节点处理前的时间点
     *   ENDTIME["et": "1494965637898"],               --结束时间，整个节点处理完的时间点
     *   FATHER_NODE_DATA["rs": [......]]                       --父节点
     *   SPANS_NODE_DATA["ss": [......]]                       --span节点
     *   APP_CODE["ac": [......]]                       --App Code
     *   DISTRIBUTED_TRACEIDS["ss": [......]]                       --父节点传过来的DistributedTraceIds，如果没有父节点则再创建一个TraceId当做DistributedTraceIds
     * @var array
     */
    private static $_allPartsNodesStruct = array
    (
        self::TRACEID => null,//TraceId
        self::STARTTIME => null,//总开始时间
        self::ENDTIME => null,//总结束时间
        self::FATHER_NODE_DATA => array(),//父节点数据
        self::SPANS_NODE_DATA => array(),//span节点数据集合
        self::APP_CODE => null,//span节点数据集合
        self::DISTRIBUTED_TRACEIDS => null,//DistributedTraceIds
    );
    /**
     * 父节点数据结构
     * 数据格式
     * "rs": [父节点
     *   ["ts": "parent_trace_0"],  --父节点，传给本子节点的TraceId
     *   ["si": 1],                 --父节点，传给本子节点的SpanId
     *   ["ac": "REMOTE_APP"],      --父节点，传给本子节点的App Code
     *   ["ph": "10.2.3.16:8080"]   --父节点，传给本子节点的PeerHost
     *   ]
     * @var array
     */
    private static $_fatherNodesStruct = array(
        self::TRACEID => null,//父节点，传给本子节点的TraceId
        self::SPAN_ID => null,//父节点，传给本子节点的SpanId
        self::APP_CODE => null,//父节点，传给本子节点的App Code
        self::PEERHOST => null,//父节点，传给本子节点的PeerHost
    );

    /**
     * span节点结构
     * 数据格式
     * [
     *    ["si": 1],    --Span A的SpanId
     *    ["ps": -1],   --父节点传过来的SpanId
     *    ["st": 1490097253228],    --Span A的开始时间，创建Span A时设置
     *    ["et": 1494965637898]     --Span A的结束时间，Span A处理完时设置
     *    ["on": "/serviceA"],      --Span A的服务URI
     *    ["ts":                    --Span A的字符串型参数
     *    ["span.layer": "http"]    --Span A的协议，分为http、rpc、db
     *    ["component": "Tomcat"]       --Span A的节点组件，如Tomcat、Nginx、HttpClient、DbClient
     *    ["peer.host": "127.0.0.1"]    --Span A的请求源IP
     *    ["span.kind": "server"]       --Span A的节点组件类型，分为server、client
     *   ["url": "10.2.3.16:8080/serviceA"] --Span A的访问地址URL
     *   ],
     *    ["tb": []],       --Span A的布尔值型参数
     *    ["ti": [          --Span A的整数值型参数
     *    "peer.port": 80       --Span A的请求源Port
     *    ]],
     *    ["lo": []]        --Span A的日志
     *    ]
     * @var array
     */
    private static $_spanNodeDataStruct = array(
        self::SPAN_ID => '',//SpanId
        self::FATHER_SPAN_ID => -1,//父节点传过来的SpanId
        self::STARTTIME => '',//开始时间
        self::ENDTIME => '',//结束时间
        self::SPAN_SERVER_URI => '',// Span 的服务URI
        self::SPAN_STRING_PARAM => array(),//Span 的字符串型参数
        self::SPAN_BOOL_PARAM => array(),//Span 的布尔型参数
        self::SPAN_INT_PARAM => array(),//Span 的整型参数
        self::SPAN_LOG => array(),//Span 的日志
    );

    /**
     * 进行单例处理
     * @param $appCode
     * @return SkyWalking
     */
    public static function getInstance($appCode = '')
    {
        if (!(self::$_instance instanceof self)) {
            self::$_instance = new self($appCode);
        }
        return self::$_instance;
    }

    private function __construct($appCode)
    {
        self::$_appCode = $appCode;
        $this->_init();
    }

    /**
     * 初始化节点信息
     * TRACEID   TraceId
     * STARTTIME  总开始时间
     * FATHER_NODE_DATA  父节点数据
     * APP_CODE  App Code
     * DISTRIBUTED_TRACEIDS DistributedTraceIds
     */
    private function _init()
    {
        if (empty(self::$_appCode)) {
            throw new Exception("Error ： Must set appCode");
        }

        //注册一个结束函数
        register_shutdown_function(array($this, "__finishAll"));

        //对节点数据进行结构初始化
        $this->_allNodeData = self::$_allPartsNodesStruct;

        /*
         * 接收头信息  并对 _swHeaderInfo 进行赋值
         * _swHeaderInfo 保存父节点
         */
        $this->receiveSWHeaderFromCaller();

        $this->_allNodeData[self::TRACEID] = $this->_generateTraceId();

        //设置最开始的时间
        $this->_allNodeData[self::STARTTIME] = $this->getMillisecond();
        $this->_allNodeData[self::FATHER_NODE_DATA] = $this->getFatherNodeData();
        $this->_allNodeData[self::APP_CODE] = self::$_appCode;
        $this->_allNodeData[self::DISTRIBUTED_TRACEIDS][] = $this->_generateDistributedTraceIds();

        /**
         * 第一个span节点初始操作
         * 当前页
         */
        $pageUrlAndPeer = $this->getPageUrlAndPeer();
        $this->_spanFirstNodeData = self::$_spanNodeDataStruct;
        $this->_spanFirstNodeData[self::SPAN_ID] = $this->_generateSpanId();
        $this->_spanFirstNodeData[self::STARTTIME] = $this->getMillisecond();
        $this->_spanFirstNodeData[self::SPAN_STRING_PARAM]['span.layer'] = 'http';
        $this->_spanFirstNodeData[self::SPAN_STRING_PARAM]['component'] = 'php-server';
        //获取客户端来源ip
        $this->_spanFirstNodeData[self::SPAN_STRING_PARAM]['peer.host'] = $this->getIp();
        //$this->_spanFirstNodeData[self::SPAN_STRING_PARAM]['peer.host'] = $pageUrlAndPeer[1];
        $this->_spanFirstNodeData[self::SPAN_SERVER_URI]     = $pageUrlAndPeer[0];
        $this->_spanFirstNodeData[self::SPAN_STRING_PARAM]['url'] = $pageUrlAndPeer[0];
        $this->_spanFirstNodeData[self::SPAN_STRING_PARAM]['span.kind'] = 'server';
    }


    /**
     * 开始span信息的生产开始流程
     * TODO 待扩展
     */
    public function startSpan()
    {
    }


    /**
     * 结束span信息 生产流程
     * TODO 待扩展
     */
    public function endSpan()
    {
    }

    /**
     * 开始span的curl信息的生产开始流程
     * 请求curl前的span的生产
     *
     * @param string $peerHost 目标地址 生成SWTraceContext header使用
     * @param array $headers CURL headers 数组内容
     * @throws Exception
     */
    public function startSpanOfCurl($peerHost, &$headers)
    {
        //初始化sanp节点信息
        $this->_spanNodeData = self::$_spanNodeDataStruct;
        $this->_spanNodeData[self::SPAN_ID] = $this->_generateSpanId();
        //使用第一span节点id当场父节点id
        $this->_spanNodeData[self::FATHER_SPAN_ID] = $this->_spanFirstNodeData[self::SPAN_ID];
        $this->_spanNodeData[self::STARTTIME] = $this->getMillisecond();

        $this->_spanNodeData[self::SPAN_STRING_PARAM]['span.layer'] = 'http';
        $this->_spanNodeData[self::SPAN_STRING_PARAM]['component'] = 'php-curl';

        //记录当前机器地址
        //$this->_spanNodeData[self::SPAN_STRING_PARAM]['peer.host'] = $_SERVER["SERVER_NAME"];
        //默认80有的情况使用有的值
        $peerPort = 80;
        //判断peerHost 还是url
        if(filter_var($peerHost, FILTER_VALIDATE_URL) === false){
            //peerHost 处理
            $peerHostArray = explode(":", $peerHost);
            $peerHostV = $peerHostArray[0];
            if(!empty($peerHostArray[1])){
                $peerPort = (int)$peerHostArray[1];
            }
        }else{
            //url 处理
            $peerUrlArray = parse_url($peerHost);
            $peerHostV = $peerUrlArray['host'];
            if(!empty($peerUrlArray['port'])){
                $peerPort = (int)$peerUrlArray['port'];
            }
        }

        $this->_spanNodeData[self::SPAN_STRING_PARAM]['peer.host'] = $peerHostV;
        $this->_spanNodeData[self::SPAN_INT_PARAM]['peer.port'] = $peerPort;

        //span.kind - string client 或 server, 指定这个span代表一个客户端还是服务端, curl属于调用别人接口 永远都是客户端
        $this->_spanNodeData[self::SPAN_STRING_PARAM]['span.kind'] = 'client';

        $_SWTraceHeader = $this->_buildSWHeaderValue($peerHost);
        array_push($headers, "SWTraceContext: " . $_SWTraceHeader['SWTraceContext']);
    }


    /**
     * @param resource $curl
     * @throws Exception
     */
    public function endSpanOfCurl($curl)
    {
        //此次是否采样
        if (!$this->isSampling()) {
            return;
        }

        if (empty($this->_spanNodeData[self::STARTTIME])) {
            throw new Exception("Need setting start time");
        }
        if (!is_resource($curl) || strtolower(get_resource_type($curl)) != 'curl') {
            throw new Exception("Need setting curl Object");
        }
        $curlInfo = curl_getinfo($curl);
        $this->_spanNodeData[self::ENDTIME] = $this->getMillisecond();
        $this->_spanNodeData[self::SPAN_SERVER_URI] = $curlInfo['url'];
        $this->_spanNodeData[self::SPAN_INT_PARAM]['status_code'] = $curlInfo['http_code'];
        //获取当前服务端口号
        //$this->_spanNodeData[self::SPAN_INT_PARAM]['peer.port'] = (int)$_SERVER["SERVER_PORT"];
        if ($curlInfo['http_code'] != 200) {
            $this->_spanNodeData[self::SPAN_BOOL_PARAM]['error'] = "true";
        }
        if (empty($this->_spanNodeData[self::SPAN_BOOL_PARAM])) {
            $this->_spanNodeData[self::SPAN_BOOL_PARAM] = new stdClass();
        }

        $this->_spanNodeData[self::SPAN_STRING_PARAM]['url'] = $curlInfo['url'];

        $this->setSpanNodeSData($this->_spanNodeData);
    }

    /**
     * 生产最终结果 如果有 完成的回调处理并调用回调处理函数
     * 已经被注册成结束函数自动调用
     * @return string json
     */
    public function __finishAll()
    {
        //此次是否采样
        if (!$this->isSampling()) {
            return;
        }
        //删除rs
        if (empty($this->_allNodeData[self::FATHER_NODE_DATA])) {
            unset($this->_allNodeData[self::FATHER_NODE_DATA]);
        }
        /**
         * 首节点插入到span节点集中
         */
        $this->_spanFirstNodeData[self::ENDTIME] = $this->getMillisecond();
        $this->_spanFirstNodeData[self::SPAN_BOOL_PARAM] = new stdClass();
        $this->_spanFirstNodeData[self::SPAN_INT_PARAM]['status_code'] = 200;
        $this->_spanFirstNodeData[self::SPAN_INT_PARAM]['peer.port'] = 80;
        array_unshift($this->_spansNodeData, $this->_spanFirstNodeData);

        $this->_allNodeData[self::ENDTIME] = $this->getMillisecond();
        $this->_allNodeData[self::SPANS_NODE_DATA] = $this->_spansNodeData;
        $results = json_encode($this->_allNodeData);
        //默认使用写日志的方式
        $this->writeLog($results);
        return $results;
    }

    /**
     * 设置 要写日志的路径
     * @param $logPath
     * @return $this
     */
    public function setLogPath($logPath)
    {
        $this->_logPath = $logPath;
        return $this;
    }

    /**
     * 设置采样率 最大值 100
     * @param int $percent
     * @return $this
     */
    public function setSamplingRate($percent)
    {
        if (!empty($percent) && ctype_digit("$percent")) {
            if ($percent >= 100) {
                $percent = 100;
            }
            $this->_samplingRate = $percent;
        }
        return $this;
    }

    /**
     * 本次进程是否采样
     * 若接收到header信息，则从header信息中继承采样率设置
     * 若没有接收到header信息，并且也没有设置采样率，则为默认为 100%
     * @return bool
     */
    private function isSampling()
    {
        //判断没有计算过采样率
        if (is_bool($this->_isSampling) !== true) {

            //继承接受的采样率
            if (!empty($this->_swHeaderText)) {
                $this->_isSampling = true;
                if ($this->_swHeaderInfo['IsSample'] != 1) {
                    $this->_isSampling = false;
                }
            } /**
             * 没有继承接收到的采样率
             */
            else {
                //若若没有设置采样率，则默认为100%采样
                $this->_isSampling = true;
                if ($this->_samplingRate === null) {
                    //throw new Exception("Please set sampling rate");
                    $this->_samplingRate = 100;
                }

                //根据设置的值计算采样率
                if ($this->_samplingRate < 100) {
                    $r = rand(1, 100);
                    $this->_isSampling = $r <= $this->_samplingRate;
                }
            }
        }
        return $this->_isSampling;
    }

    /**
     * 写日志
     * @param $text
     * @throws Exception
     */
    private function writeLog($text)
    {
        if (!empty($this->_logPath)) {
            $logPath = rtrim($this->_logPath, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
        } else {
//            throw new Exception("Please set log path: setLogPath()");
            $logPath = DIRECTORY_SEPARATOR . 'tmp' . DIRECTORY_SEPARATOR;
        }
        $logFilename = strtolower($logPath . 'skywalking.' . date("Ymd") . '.log');
        error_log($text . "\n", 3, $logFilename);
    }


    /**
     * 生成TraceID，用于此次请求的ID
     */
    private function _generateTraceId()
    {
        //沿用父节点的TraceId ; 本节点生成的全链路系统唯一的事务编号，推荐生成规则为：Segment.毫秒时间戳.UUID后7位的哈希码.当前进程号PID.当前线程ID.当前线程生成的流水号.ip
        if (!empty($this->_swHeaderInfo)) {
            return $this->_swHeaderInfo['TraceId'];
        }
        if (empty($this->_traceId)) {
            $this->_traceId = "Segment." . $this->makeTraceId();
        }
        return $this->_traceId;
    }


    /**
     * 沿用父节点的DistributedTraceIds，如果没有父节点则再创建一个TraceId当做DistributedTraceIds，Trace.毫秒时间戳.UUID后7位的哈希码.当前进程号PID.当前线程ID.当前线程
     * @return mixed|string
     */
    private function _generateDistributedTraceIds()
    {
        if (!empty($this->_swHeaderInfo)) {
            return $this->_swHeaderInfo['DistributedTraceIds'];
        }
        if (empty($this->_distributedTraceIds)) {
            $this->_distributedTraceIds = "Trace." . $this->makeTraceId();
        }
        return $this->_distributedTraceIds;
    }

    private function makeTraceId()
    {
        //生产唯一码毫秒时间戳.uuid.当前进程号PID.当前线程ID.当前线程生成的流水号.ip
        $millisecond = $this->getMillisecond();
        $uuid = uniqid();
        if (defined('PHP_SAPI') && PHP_SAPI == 'cli') {
            $ip = '127.0.0.1';
        } elseif (isset($_SERVER)) {
            if (isset($_SERVER['SERVER_ADDR'])) {
                $ip = $_SERVER['SERVER_ADDR'];
            } else {
                $ip = $_SERVER['LOCAL_ADDR'];
            }
        } else {
            $ip = getenv('SERVER_ADDR');
        }

        return sprintf("%s.%s.%s.0.0.%s", $millisecond, $uuid, getmypid(), $ip);
    }

    private function getIp()
    {
        if (!empty($_SERVER['HTTP_DD_REAL_IP'])) {
            $ip = $_SERVER['HTTP_DD_REAL_IP'];
        } elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {
            $ip = $_SERVER['HTTP_X_FORWARDED_FOR'];
        } elseif (!empty($_SERVER['HTTP_CLIENT_IP'])) {
            $ip = $_SERVER['HTTP_CLIENT_IP'];
        } else {
            $ip = $_SERVER['REMOTE_ADDR'];
        }
        if (strpos($ip, ',') > 0) {
            $ips = explode(',', $ip);
            $ip = $ips[0];
        }

        return $ip;
    }

    /**
     * 获取当前的url
     * @return string
     */
    private function getPageUrlAndPeer(){
        $pageURL = 'http';
        if (isset($_SERVER["HTTPS"]) && ($_SERVER["HTTPS"] == "on")) {
            $pageURL .= "s";
        }
        $pageURL .= "://";
        $peer = $_SERVER["HTTP_HOST"];
        $pageURL .= $_SERVER["HTTP_HOST"] . $_SERVER["REQUEST_URI"];
        return array($pageURL, $peer);
    }

    /**
     * 生成 SpanId
     * 用于此次调用的区块标示
     */
    private function _generateSpanId()
    {
        return $this->_spanID++;
    }

    private function getMillisecond()
    {
        list($t1, $t2) = explode(' ', microtime());
        $millisecond = (float)sprintf('%.0f', (floatval($t1) + floatval($t2)) * 1000);
        return $millisecond;
    }

    /**
     * 设置appCode，用于标示应用本身的名称
     * @param string $appCode
     */
    public function setAppCode($appCode)
    {

    }


    /**
     * 设置父节点数据
     * 数据格式
     * "rs": [父节点
     *   ["ts": "parent_trace_0"],  --父节点，传给本子节点的TraceId
     *   ["si": 1],                 --父节点，传给本子节点的SpanId
     *   ["ac": "REMOTE_APP"],      --父节点，传给本子节点的App Code
     *   ["ph": "10.2.3.16:8080"]   --父节点，传给本子节点的PeerHost
     *   ]
     * @return array
     */
    public function getFatherNodeData()
    {
        if (empty($this->_swHeaderInfo)) {
            return array();
        }
        //对父节点进行初始化
        $this->_fatherNodeData = self::$_fatherNodesStruct;
        $this->_fatherNodeData[self::TRACEID] = $this->_swHeaderInfo['TraceId'];
        $this->_fatherNodeData[self::SPAN_ID] = (int)$this->_swHeaderInfo['SpanId'];
        $this->_fatherNodeData[self::APP_CODE] = $this->_swHeaderInfo['AppCode'];
        $this->_fatherNodeData[self::PEERHOST] = $this->_swHeaderInfo['PeerHost'];

        return $this->_fatherNodeData;
    }

    /**
     * 设置span节点数据
     * @param $nodeData
     * 数据格式
     * [
     *    ["si": 1],        --Span A的SpanId
     *    ["ps": -1],       --父节点传过来的SpanId
     *    ["st": 1490097253228],        --Span A的开始时间，创建Span A时设置
     *    ["et": 1494965637898],        --Span A的结束时间，Span A处理完时设置
     *    ["on": "/serviceA"],          --Span A的服务URI
     *    ["ts":                    --Span A的字符串型参数
     *    ["span.layer": "http"]    --Span A的协议，分为http、rpc、db
     *    ["component": "Tomcat"    ]   --Span A的节点组件，如Tomcat、Nginx、HttpClient、DbClient
     *    ["peer.host": "127.0.0.1"]    --Span A的请求源IP
     *    ["span.kind": "server"]       --Span A的节点组件类型，分为server、client
     *   ["url": "10.2.3.16:8080/serviceA"] --Span A的访问地址URL
     *   ],
     *    ["tb": []],   --Span A的布尔值型参数
     *    ["ti": [      --Span A的整数值型参数
     *    "peer.port": 80   --Span A的请求源Port
     *    ]],
     *    ["lo": []]    --Span A的日志
     *    ]
     * @return $this
     */
    private function setSpanNodeSData($nodeData)
    {
        array_push($this->_spansNodeData, $nodeData);
        return $this;
    }

    /**
     * 获取接收到 SWTraceContext 的 header
     */
    private function receiveSWHeaderFromCaller()
    {
        if (!empty($_SERVER['HTTP_SWTRACECONTEXT'])) {
            $this->_swHeaderText = $_SERVER['HTTP_SWTRACECONTEXT'];
            list(
                $this->_swHeaderInfo['TraceId'],
                $this->_swHeaderInfo['SpanId'],
                $this->_swHeaderInfo['AppCode'],
                $this->_swHeaderInfo['PeerHost'],
                $this->_swHeaderInfo['DistributedTraceIds'],
                $this->_swHeaderInfo['IsSample'],
                ) = explode('|', $this->_swHeaderText);
        }
        return $this->_swHeaderInfo;
    }

    /**
     *  生成调用接口的 header
     * @param string $peerHost
     * @return array
     * @throws \Exception
     */
    private function _buildSWHeaderValue($peerHost)
    {
        $_SWHeader = array();

        $_SWHeader['traceId'] = $this->_generateTraceId();
        $_SWHeader['SpanId'] = $this->_generateSpanId();
        $_SWHeader['AppCode'] = self::$_appCode;
        $_SWHeader['PeerHost'] = $peerHost;
        $_SWHeader['DistributedTraceIds'] = $this->_generateDistributedTraceIds();
        $_SWHeader['IsSample'] = (int)$this->isSampling();

        return array('SWTraceContext' => implode('|', $_SWHeader));
    }
}
