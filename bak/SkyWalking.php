<?php

/**
 * Class SkyWalking
 * 需要 设置 LOG_PATH 常量 默认会放到/tmp/目录下
 * 需要设置registerUrl 注册apm项  默认 appinfo.mservice.m.api:8080
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
     * 总段节点数据
     */
    private $_allSegment = array();
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
     * appid 相关信息
     * @var array
     */
    private $_appIds = array();
    
    /**
     * 生产的SPAN_ID
     * @var
     */
    private $_spanID = 0;
    /**
     * 日志目录
     * @var unknown
     */
    private $_logPath;
    /**
     * apm注册地址
     */
    private $_registerUrl;
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
     * 关闭所有功能
     */
    private $_isClose = 0;

    /**
     * 定义替换简单字符常量
     */
    const SEGMENT = 'sg'; //全部段节点
    const DISTRIBUTED_TRACEIDS = 'gt';//DistributedTraceIds
    const TRACE_SEGMENT_ID = 'ts';//本次请求id
    const APPLICATION_ID = 'ai';//app id
    const APPLICATION_INSTANCE_ID = 'ii';//实例id
    const FATHER_NODE_DATA = 'rs'; //父节点数据
    const SPANS_NODE_DATA = 'ss'; //span节点数据集合
    
    const PARENT_TRACE_SEGMENT_ID = "ts"; //本次请求id
    const PARENT_APPLICATION_ID = "ai";//app id
    const PARENT_SPAN_ID = "si";//spanid
    const PARENT_SERVICE_ID = "vi";//
    const PARENT_SERVICE_NAME = "vn";
    const NETWORK_ADDRESS_ID = "ni";
    const NETWORK_ADDRESS = "nn";
    const ENTRY_APPLICATION_INSTANCE_ID = "ea";
    const ENTRY_SERVICE_ID = "ei";
    const ENTRY_SERVICE_NAME = "en";
    const REF_TYPE_VALUE = "rv";
    
    const SPAN_ID = 'si'; //SpanId
    const SPAN_TYPE_VALUE = 'tv';
    const SPAN_LAYER_VALUE = "lv";
    const FATHER_SPAN_ID = 'ps'; //父节点传过来的SpanId
    const STARTTIME = 'st'; //开始时间
    const ENDTIME = 'et';  //结束时间
    const COMPONENT_ID = 'ci';
    const COMPONENT_NAME = 'cn';
    const OPERATION_NAME_ID = 'oi';
    const OPERATION_NAME = 'on';// Span 的服务URI
    const PEER_ID = 'pi';
    const PEER = 'pn';
    const IS_ERROR = 'ie';
    const TAGS = 'to';
    const LOGS = 'lo';
    
    const KEY = "k";
    const VALUE = "v";
    
    const TIME = "ti";
    const LOG_DATA = "ld";
    
    /**
     * 值设置常量
     */
    const IS_ARRAY = true;
    const DEFAULT_REGISTER_URL = 'http://apm.api:12800';
    
    /**
     * 全部节点结构
     * @var array
     */
    private static $_allNodesStruct = array(
        self::DISTRIBUTED_TRACEIDS  => null,
        self::SEGMENT               => null,
    );
    
    /**
     * 全部段节点结构
     *
     * @var array
     */
    private static $_allSegmentStruct = array
    (
        self::TRACE_SEGMENT_ID          => array(),//请求的 id
        self::APPLICATION_ID            => null,//appid
        self::APPLICATION_INSTANCE_ID   => null,//实例id
        self::FATHER_NODE_DATA          => array(),//父节点数据
        self::SPANS_NODE_DATA           => array(),//span节点数据集合
    );
    /**
     * 父节点数据结构
     * 数据格式
     * @var array
     */
    private static $_fatherNodesStruct = array(
        self::PARENT_TRACE_SEGMENT_ID           => array(),//父节点，传给本子节点的TraceId
        self::PARENT_APPLICATION_ID             => null,
        self::PARENT_SPAN_ID                    => null,
        self::PARENT_SERVICE_ID                 => null,
        self::PARENT_SERVICE_NAME               => null,
        self::NETWORK_ADDRESS_ID                => null,
        self::NETWORK_ADDRESS                   => null,
        self::ENTRY_APPLICATION_INSTANCE_ID     => null,
        self::ENTRY_SERVICE_ID                  => null,
        self::ENTRY_SERVICE_NAME                => null,
        self::REF_TYPE_VALUE                    => null,
    );

    /**
     * span节点结构
     * 数据格式
     * @var array
     */
    private static $_spanNodeDataStruct = array(
        
        self::SPAN_ID => '',//SpanId
        self::SPAN_TYPE_VALUE => 0,
        self::SPAN_LAYER_VALUE => 0,
        self::FATHER_SPAN_ID => -1,
        self::STARTTIME => '',
        self::ENDTIME => '',
        //self::COMPONENT_ID => '',
        self::COMPONENT_NAME => '',
        //self::OPERATION_NAME_ID => '',
        //self::PEER_ID => '',
        self::PEER => '',
        self::IS_ERROR => false,
        
        self::TAGS => array(),//Span 的整型参数
        self::LOGS => array(),//Span 的日志
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
        if(!$this->appInitRegister()){
            return ;
        }
        //注册一个结束函数
        register_shutdown_function(array($this, "__finishAll"));
        
        //对节点数据进行结构初始化
        $this->_allNodeData = self::$_allNodesStruct;
        //对段节点进行初始化
        $this->_allSegment = self::$_allSegmentStruct;
        
        /*
         * 接收头信息  并对 _swHeaderInfo 进行赋值
         * _swHeaderInfo 保存父节点
         */
        $this->receiveSWHeaderFromCaller();
        
        //总结阶段的链路全局id
        $this->_allNodeData[self::DISTRIBUTED_TRACEIDS] = array($this->_generateTraceId(self::IS_ARRAY));

        //设置节点id
        $this->_allSegment[self::TRACE_SEGMENT_ID]  = $this->_generateTraceId(self::IS_ARRAY);
        $this->_allSegment[self::APPLICATION_ID]    = $this->_getAppId();
        $this->_allSegment[self::APPLICATION_INSTANCE_ID] = $this->_getAppInstanceId();
        $this->_allSegment[self::FATHER_NODE_DATA]  = $this->getFatherNodeData();
        
        
        /**
         * 第一个span节点初始操作
         * 当前页
         */
        $pageUrlAndPeer = $this->getPageUrlAndPeer();
        $this->_spanFirstNodeData = self::$_spanNodeDataStruct;
        $this->_spanFirstNodeData[self::SPAN_ID] = $this->_generateSpanId();
        $this->_spanFirstNodeData[self::SPAN_TYPE_VALUE] = 0;
        $this->_spanFirstNodeData[self::SPAN_LAYER_VALUE] = 3;
        $this->_spanFirstNodeData[self::STARTTIME] = $this->getMillisecond();
        //$this->_spanFirstNodeData[self::COMPONENT_ID] = 'php-server';
        $this->_spanFirstNodeData[self::COMPONENT_NAME] = 'php-server';
        //$this->_spanFirstNodeData[self::PEER_ID] = 'server';
        $this->_spanFirstNodeData[self::PEER] = '127.0.0.1';
        
       // $this->_spanFirstNodeData[self::OPERATION_NAME_ID] = $pageUrlAndPeer[0];
        $parseUrl = parse_url($pageUrlAndPeer[0]);
        $host = $parseUrl['host'];
        if( !empty( $parseUrl['port'] ) ){
            $host .= ':' .  $parseUrl['port'];
        }
        $this->_spanFirstNodeData[self::OPERATION_NAME] =  $host . $parseUrl['path'];
        $this->_spanFirstNodeData[self::IS_ERROR] = false;
        $this->_spanFirstNodeData[self::TAGS][] = array('k'=>'url','v'=>$pageUrlAndPeer[0]);
        
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
        $this->_spanNodeData[self::SPAN_TYPE_VALUE] = 1;
        
        //使用第一span节点id当场父节点id
        $this->_spanNodeData[self::FATHER_SPAN_ID] = $this->_spanFirstNodeData[self::SPAN_ID];
        $this->_spanNodeData[self::STARTTIME] = $this->getMillisecond();
        //$this->_spanNodeData[self::COMPONENT_ID] = 'php-curl';
        $this->_spanNodeData[self::COMPONENT_NAME] = 'php-curl';
        
        $this->_spanNodeData[self::SPAN_LAYER_VALUE] = 3;
        
        //$this->_spanNodeData[self::PEER_ID] = 'client';
        

        $_SWTraceHeader = $this->_buildSWHeaderValue($peerHost);
        array_push($headers, "sw3: " . $_SWTraceHeader['SWTraceContext']);
    }


    /**
     * @param resource $curl
     * @throws Exception
     */
    public function endSpanOfCurl($curl)
    {
        //此次是否采样
        if (!$this->isSampling() || $this->_isClose) {
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
        //$this->_spanNodeData[self::OPERATION_NAME_ID] = $curlInfo['url'];
        $parseUrl = parse_url($curlInfo['url']);
        $host = $parseUrl['host'];
        if( !empty( $parseUrl['port'] ) ){
            $host .= ':' .  $parseUrl['port'];
        }
        $this->_spanNodeData[self::OPERATION_NAME] = $host . $parseUrl['path'];
        $this->_spanNodeData[self::PEER] = $host;

        //获取当前服务端口号
        if ($curlInfo['http_code'] != 200) {
            $this->_spanNodeData[self::IS_ERROR] = true;
        }

        $this->_spanNodeData[self::TAGS][] = array('k'=>'url','v'=>$curlInfo['url']);
    
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
        if (!$this->isSampling() || $this->_isClose) {
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
        
        array_unshift($this->_spansNodeData, $this->_spanFirstNodeData);
        $this->_allSegment[self::SPANS_NODE_DATA] = $this->_spansNodeData;
        
       // $this->_allNodeData[self::ENDTIME] = $this->getMillisecond();
        $this->_allNodeData[self::SEGMENT] = $this->_allSegment;
        $results = json_encode(array($this->_allNodeData));
        //默认使用写日志的方式
        $this->writeLog($results);
  
        return $results;
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
    
        $this->_fatherNodeData[self::PARENT_TRACE_SEGMENT_ID]   = explode('.', $this->_swHeaderInfo['TraceId']);
        $this->_fatherNodeData[self::PARENT_APPLICATION_ID]     = $this->_swHeaderInfo['ParentAppInstanceid'];
        $this->_fatherNodeData[self::PARENT_SPAN_ID]            = $this->_swHeaderInfo['SpanId'];
        $this->_fatherNodeData[self::PARENT_SERVICE_ID]         = $this->_swHeaderInfo['ParentAppname'];
        $this->_fatherNodeData[self::PARENT_SERVICE_NAME]       = $this->_swHeaderInfo['ParentAppname'];
        $this->_fatherNodeData[self::NETWORK_ADDRESS_ID]        = $this->_swHeaderInfo['PeerHost'];
        $this->_fatherNodeData[self::NETWORK_ADDRESS]           = $this->_swHeaderInfo['PeerHost'];
        $this->_fatherNodeData[self::ENTRY_APPLICATION_INSTANCE_ID] = $this->_swHeaderInfo['EntryAppInstanceid'];
        $this->_fatherNodeData[self::ENTRY_SERVICE_ID]          = $this->_swHeaderInfo['EntryAppname'];
        $this->_fatherNodeData[self::ENTRY_SERVICE_NAME]        = $this->_swHeaderInfo['EntryAppname'];
        $this->_fatherNodeData[self::REF_TYPE_VALUE]            = 0;
    
    
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
    public function setSpanNodeSData($nodeData)
    {
        array_push($this->_spansNodeData, $nodeData);
        return $this;
    }
    
    public function close(){
        $this->_isClose = 1;
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
     * 获取写日志路径
     * 
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    public function getLogPath(){
        if (!empty($this->_logPath)) {
            $logPath = rtrim($this->_logPath, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
        } else {
            $logPath = DIRECTORY_SEPARATOR . 'tmp' . DIRECTORY_SEPARATOR;
        }
        return $logPath;
    }
    /**
     * 设置注册apm地址
     * @param  $registerUrl
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    public function setRegisterUrl($registerUrl){
        $this->_registerUrl = $registerUrl;
        return $this;
    }
    /**
     * 获取注册apm地址
     * @return string
     * @since 2017年11月28日
     * @copyright
     * @return string
     */
    public function getRegisterUrl(){
        if(!empty($this->_registerUrl))  return $this->_registerUrl;
       
        return  self::DEFAULT_REGISTER_URL;
    }
    
    /**
     * 设置采样率 最大值 100
     * @param int $percent
     * @return $this
     */
    public function setSamplingRate($percent)
    {
        if ($percent >= 100) {
            $percent = 100;
        }
        $this->_samplingRate = $percent;
     
        return $this;
    }
    
    public function closeSampling(){
        $this->_isSampling = false;
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
        if (!isset($this->_isSampling)) {
         
            $this->_isSampling = true;
            if ($this->_samplingRate === null) {
                $this->_samplingRate = 100;
            }
            //根据设置的值计算采样率
            if ($this->_samplingRate < 100) {
                $r = rand(1, 100);
                $this->_isSampling = ($r <= $this->_samplingRate);
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
            $logPath = DIRECTORY_SEPARATOR . 'tmp' . DIRECTORY_SEPARATOR;
        }
        $logFilename = $logPath . 'skywalking.' . date("Ymd") . '.log';
        error_log($text . "\n", 3, $logFilename);
    }
    
    /**
     * 获取 id parts
     * 
     * @since 2017年11月23日
     * @copyright
     * @return return_type
     */
    private function _getIdParts(){
        
        $serverIp = $this->_getCurrentMachineIp();
        $intUuid = base_convert(uniqid(), 16, 10);
        return array(
            (int)ip2long($serverIp),
            (int)getmypid(),
            $intUuid,
        );
    }
    
    /**
     * 生成TraceID，用于此次请求的ID
     * @param boolean $isArray
     * @return mixed|string
     * @since 2017年11月27日
     * @copyright
     * @return mixed|string
     */
    private function _generateTraceId($isArray = false)
    {
        //沿用父节点的TraceId ; 本节点生成的全链路系统唯一的事务编号。
        if (!empty($this->_swHeaderInfo)) {
            $this->_traceId =  $this->_swHeaderInfo['TraceId'];
        }
        if (empty($this->_traceId)) {
            $this->_traceId =  $this->makeTraceId();
        }
        
        if($isArray){
            $traceIdArray = explode('.', $this->_traceId);
            foreach ( $traceIdArray as $key => $val ){
                $traceIdArray[$key] = (int)$val;
            }
            return $traceIdArray;
        }
        return $this->_traceId;
    }
    /**
     * 获取父节点注册id
     * 
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    private function _parentAppInstanceid(){
        
        //沿用父节点的ParentAppInstanceid ; 本节点生成的全链路系统唯一的事务编号。
        return $this->_getAppInstanceId();
    }
    /**
     * 获取入口节点数据
     *
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    private function _entryAppnameOperationId(){
        if(!empty( $this->_swHeaderInfo['EntryAppnameOperationId'] )){
            return $this->_swHeaderInfo['EntryAppnameOperationId'];
        }
        return $this->_spanFirstNodeData[self::SPAN_ID];
    }
    private function _parentAppnameOperationId(){
        return $this->_spanFirstNodeData[self::SPAN_ID];
    }
    /**
     * 获取入口节点数据
     * 
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    private function _entryAppName(){
        if(!empty( $this->_swHeaderInfo['EntryAppname'] )){
            return $this->_swHeaderInfo['EntryAppname'];
        }
        return self::$_appCode;
    }
    private function _parentAppName(){
        return self::$_appCode;
    }
    /**
     * 获取入口节点注册id
     * 
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    private function _entryAppInstanceid(){
        if(!empty( $this->_swHeaderInfo['EntryAppInstanceid'] )){
            return $this->_swHeaderInfo['EntryAppInstanceid'];
        }
        return $this->_getAppInstanceId();
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
            $this->_distributedTraceIds = $this->makeTraceId();
        }
        return $this->_distributedTraceIds;
    }
    
    /**
     * 当前机器ip
     * 
     * @since 2017年11月23日
     * @copyright
     * @return return_type
     */
    private function _getCurrentMachineIp(){
        
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
        return $ip;
    }
    
    private function makeTraceId()
    {
        //生产唯一码毫秒时间戳.uuid.当前进程号PID.当前线程ID.当前线程生成的流水号.ip
       
        $makeTraceIdArray = $this->_getIdParts();
        
        return implode('.', $makeTraceIdArray);
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
     * 获取接收到 SWTraceContext 的 header
     */
    private function receiveSWHeaderFromCaller()
    {
        if (!empty($_SERVER['HTTP_SWTRACECONTEXT'])) {
            $this->_swHeaderText = $_SERVER['HTTP_SWTRACECONTEXT'];
            list(
                $this->_swHeaderInfo['traceId'],
                $this->_swHeaderInfo['SpanId'],
                $this->_swHeaderInfo['ParentAppInstanceid'],
                $this->_swHeaderInfo['EntryAppInstanceid'],
                $this->_swHeaderInfo['PeerHost'],
                $this->_swHeaderInfo['EntryAppnameOperationId'],
                $this->_swHeaderInfo['ParentAppnameOperationId'],
                $this->_swHeaderInfo['DistributedTraceIds']
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
        $_SWHeader['ParentAppInstanceid'] = $this->_parentAppInstanceid();
        $_SWHeader['EntryAppInstanceid'] = $this->_entryAppInstanceid();
        $_SWHeader['PeerHost'] = '#' . $peerHost;
        $_SWHeader['EntryAppnameOperationId'] = $this->_entryAppnameOperationId();
        $_SWHeader['ParentAppnameOperationId'] = $this->_parentAppnameOperationId();
        $_SWHeader['DistributedTraceIds'] = $this->_generateDistributedTraceIds();

        return array('SWTraceContext' => implode('|', $_SWHeader));
    }
    
    /**
     * 设置应用注册
     * 
     * @since 2017年11月23日
     * @copyright
     * @return return_type
     */
    private function appInitRegister(){
        
        if($this->_isClose){
            return false;
        }
        $appId = $this->AppId();
        if(empty($appId)){
            return false;
        }
        $appInstanceId = $this->AppInstanceId($appId);
        if($appInstanceId !== 0 && empty($appInstanceId)){
            return false;
        }
        $this->_appIds =  array($appId, $appInstanceId);
       
        
        return true;
    }
    
    /**
     * 获取 app_id
     * @return mixed
     * @since 2017年11月27日
     * @copyright
     * @return mixed
     */
    private function _getAppId(){
        return $this->_appIds[0];
    }
    
    /**
     * 获取 app_id
     * @return mixed
     * @since 2017年11月27日
     * @copyright
     * @return mixed
     */
    private function _getAppInstanceId(){
        return $this->_appIds[1];
    }
    
    
    /**
     * 获取appid
     * 
     * @since 2017年11月23日
     * @copyright
     * @return return_type
     */
    private function AppId(){
        
        $processNo = getmypid();
        $fileName =   $this->getLogPath() . $processNo . '.appid.pid';
        $appId = $this->getfilesText($fileName);
        if(empty($appId)){
            
            $param ='["'. self::$_appCode .'"]';
            $appIds = $this->doRequest($param, $this->getRegisterUrl() . '/application/register');
            $appIds = json_decode($appIds, true);
            $appId  = $appIds[0]['i'];
            if(!empty($appId)){
                $this->fwriteFilesText($fileName, $appId);
            }
        }
        if(empty($appId)){
           return false; 
        }
        
        return (int)$appId;
    }
    
    /**
     * 获取appid的实例id
     * @param unknown $applicationId
     * @since 2017年11月23日
     * @copyright
     * @return return_type
     */
    private  function AppInstanceId($appId){
        
        $processNo = getmypid();
        $fileName =   $this->getLogPath() . $processNo . '.instance.appid.pid';
        $appInstanceId = $this->getfilesText($fileName);
        if($appInstanceId !== '0' && empty($appInstanceId)){
     
            
            $rt = date('YmdHis');
            $ip = $this->_getCurrentMachineIp();
            $agentUuid  = self::$_appCode . '.' . $ip;
            $hostname = '';
            if(!empty( $_SERVER['HOSTNAME']) ){
                $hostname = $_SERVER['HOSTNAME'];
            }
            
            $param  = array(
                'ai' => (int)$appId,
                'au' => $agentUuid,
                'rt' => $rt,
                'oi' => array(
                    'osName'    => PHP_OS,
                    'hostname'  => $hostname,
                    'processNo' => $processNo,
                    'ipv4s'     => array($ip),
                ),
            );
            $appInstanceIds = $this->doRequest(json_encode($param), $this->getRegisterUrl() . '/instance/register');
            $appInstanceIds = json_decode($appInstanceIds, true);
            if(isset($appInstanceIds['ii'])){
                $appInstanceId  = (string)$appInstanceIds['ii'];
            }
            if(!empty($appInstanceId) || $appInstanceId === '0'){
                $this->fwriteFilesText($fileName, $appInstanceId);
            }
        }
        if($appInstanceId !== '0' &&  empty($appInstanceId)){
            return false;
        }

        return (int)$appInstanceId;
        
    }
    
    /**
     * http请求
     * @param unknown $params
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    private  function doRequest($params, $registerUrl) {
        
        if($this->_isClose || !$this->isSampling()){
            return ;
        }
        //不支持curl 关闭改功能
        if( !function_exists('curl_init') ){
            $this->_isClose = false;
        }
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $registerUrl);
        curl_setopt($ch, CURLOPT_AUTOREFERER, true);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_TIMEOUT,1);
        curl_setopt($ch, CURLOPT_POST, 1);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
        $result = curl_exec($ch);//运行curl    
        $info = curl_getinfo($ch);
        curl_close($ch);
        
        if( 0 == strlen($result)  || ($info['http_code'] != 200) ){
            return false;
        }
        return $result;
    }
    
    /**
     * 获取文件当前行内容
     * @param unknown $filename
     * @since 2017年11月28日
     * @copyright
     * @return return_type
     */
    private function getfilesText($filename){
        try {
            $file = new SplFileObject($filename);
            return $file->current();
           
        } catch (\RuntimeException $e) {
            return  null;
        }
    }    
    
    private function fwriteFilesText($filename, $text){
        try {
            $file = new SplFileObject($filename, "w");
            $file->fwrite($text);
        } catch (\RuntimeException $e) {
        }
        
    }
}
