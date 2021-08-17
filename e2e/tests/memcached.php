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
function testMemcached() {

    $memcached = new Memcached();
    $memcached->addServer('127.0.0.1', 11211);

    $memcached->setOption(Memcached::OPT_COMPRESSION, false);
    //配置存储不压缩，压缩value不利于递增递减

    $memcached->setOption(Memcached::OPT_LIBKETAMA_COMPATIBLE, true);

    $memcached->addServers(array( //添加多台服务器分布式
        array('127.0.0.1', 11211, 20),
        array('127.0.0.1', 11211, 20),
    ));

    $memcached->flush(1); //1秒内清除所有元素

    $memcached->set('name', '2333');

    $memcached->setByKey('server_master_db', 'mage', '28');
    # 指定 server_key server_master_db 存储键mage

    $memcached->setMulti(array('salary' => '3000', 'email' => '2333'));
    // 存储多个元素

    $memcached->setMultiByKey('server_master_db',
        array('salary' => '3000', 'email' => '2333')
    );
    //  'server_master_db'服务器 存储多个元素

    $memcached->add('name', 'TK'); // 键name不存在添加value 否则添加失败

    $memcached->addByKey('server_master_db', 'mname', 'MTK');

    $memcached->append('key', '-816'); // 键key的value后追加字符串 -816

    $memcached->appendByKey('server_master_db', 'mname', '-923');

    $memcached->prepend('name', 'pre-'); #向一个已存在的元素前面追加数据

    $memcached->prependByKey('server_master_db', 'name', 'pre-');
    # 使用server_key自由的将key映射到指定服务器 向一个已存在的元素前面追加数据

    $memcached->get('name');


    $memcached->getByKey('server_master_db', 'mname');  # 从特定的服务器检索元素

    $memcached->getAllKeys(); // bug 我一致返回是false

    $memcached->increment('age', '1');
    #增加数值元素的值  如果元素的值不是数值类型，将其作为0处理


    $memcached->incrementByKey('server_master_db', 'age', '1');
    # 用于识别储存和读取值的服务器

    $memcached->decrement('age', '1');
    #减少数值元素的值  如果元素的值不是数值类型，将其作为0处理

    $memcached->decrementByKey('server_master_db', 'age', '1');
    # 用于识别储存和读取值的服务器

    $memcached->getDelayed(array('name', 'age'), true, null);
    # 请求多个元素， 如果with_cas设置为true，会同时请求每个元素的CAS标记
    # 指定一个result callback来替代明确的抓取结果

    $memcached->getDelayedByKey('server_master_db', array('name', 'age'), true, null);

    $memcached->fetch();
    # 搭配 $memcached->getDelayed()使用, 从最后一次请求中抓取下一个结果

    $memcached->fetchAll();
    #抓取最后一次请求的结果集中剩余的所有结果

    $memcached->getMulti(array('name', 'age')); #检索多个元素

    $memcached->getMultiByKey('server_master_db', array('mname', 'mage'));
    # 从特定服务器检索多个元素
    # 与 $this->memcached->fetchAll() 搭配使用

    $memcached->getOption(Memcached::OPT_COMPRESSION);
    # 获取Memcached的选项值

    $memcached->getResultCode();
    # 返回最后一次操作的结果代码   Memcached::RES_NOTSTORED

    $memcached->getResultMessage();
    # 返回最后一次操作的结果描述消息

    $memcached->getServerByKey('server_master_db'); # 获取一个key所映射的服务器信息

    $memcached->getServerList(); #  获取服务器池中的服务器列表

    $memcached->getStats(); #  获取服务器池的统计信息

    $memcached->getVersion();  #  获取服务器池中所有服务器的版本信息

    $memcached->isPersistent(); #判断当前连接是否是长连接

    $memcached->replace('name', 'pre-2333');
    #set()类似，但是如果 服务端不存在key， 操作将失败

    $memcached->replaceByKey('server_master_db', 'name', 'pre-2333');
    #setBykey()类似，但是如果 服务端不存在key， 操作将失败

    $memcached->resetServerList(); //清楚服务器池信息

    $memcached->setOption(Memcached::OPT_PREFIX_KEY, "widgets");
    #设置一个memcached选项

    $memcached->setOptions(array());
    #设置一个memcached选项

    //$memcached->setSaslAuthData('', '');

    $memcached->touch('name', 10);
    #设置键name 10秒后过期(只适用30天之内的秒数) ，30天以后请设置时间戳

    $memcached->touchByKey('server_master_db', 'name', 10);

    $memcached->delete('age', 10);
    #10秒(秒数/时间戳)内删除一个元素  这个键已经存在删除队列
    #该键对应的get、add、replace命令都不可用，直到删除

    $memcached->deleteByKey('server_master_db', 'age');

    $memcached->deleteMulti(array('age', 'name')); #传入array删除多个key

    $memcached->deleteMultiByKey('server_master_db', array('age', 'name'));

    $memcached->quit(); # 关闭所有打开的链接

}
