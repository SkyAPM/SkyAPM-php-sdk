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
#include "src/logging/logging_manager.h"
#include <boost/interprocess/ipc/message_queue.hpp>
#include "logging/Logging.grpc.pb.h"
#include "src/common.h"
#include "grpc/grpc.h"
#include "grpc++/grpc++.h"
#include "src/logging/logging_hander_yii.h"
#include "src/logging/logging_hander_thinkphp.h"
#include "sky_log.h"
#include <vector>
#include "sky_plugin_logging.h"
extern struct service_info *s_info;

void LogginManager::init(const ManagerOptions &options, struct service_info *info) {
    if (!SKYWALKING_G(logging_enable)) {
        sky_log("logging plugin not enabled");
        return;
    }
    sprintf(info->log_mq_name, "skywalking_logging_queue_%d", getpid());
    try {
        boost::interprocess::message_queue::remove(info->log_mq_name);
        boost::interprocess::message_queue(
                boost::interprocess::open_or_create,
                info->log_mq_name,
                SKYWALKING_G(logging_mq_length),
                SKYWALKING_G(logging_mq_max_message_length),
                boost::interprocess::permissions(0666)
        );
    } catch (boost::interprocess::interprocess_exception &ex) {
        php_error(E_WARNING, "%s %s", "[skywalking] create logging queue fail ", ex.what());
    }
    std::thread logth(logConsumer, options, info);
    logth.detach();
    if (SKYWALKING_G(logging_yii_enable)) {
        sky_log("logging yii plugin enable, target_name:" + std::string(SKYWALKING_G(logging_yii_target_name)));
        register_logging_hander(new YiiLoggingHander(SKYWALKING_G(logging_yii_target_name)));
    }

    if (SKYWALKING_G(logging_thinkphp_enable)) {
        sky_log("logging thinkphp plugin enable, target_name:" + std::string(SKYWALKING_G(logging_thinkphp_target_name)));
        register_logging_hander(new ThinkphpLoggingHander(SKYWALKING_G(logging_thinkphp_target_name)));
    }

    sky_log("logging plugin init success, mq_name: [" + std::string(info->log_mq_name) 
    + "],logging_mq_length: [" + std::to_string(SKYWALKING_G(logging_mq_length))
    + "],logging_mq_max_message_length: " +  std::to_string(SKYWALKING_G(logging_mq_max_message_length))
    + "]");
}

void LogginManager::logConsumer(const ManagerOptions &options, struct service_info *info) {
    sky_log("logConsumer begin : grpc address->" + options.grpc + ", receive mq name->" + info->log_mq_name);
    while(true) {
        std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(options.grpc, getCredentials(options)));
        std::unique_ptr<skywalking::v3::LogReportService::Stub> stub(skywalking::v3::LogReportService::NewStub(channel));
        grpc::ClientContext context;
        Commands commands;
        if (!options.authentication.empty()) {
            context.AddMetadata("authentication", options.authentication);
        }
        auto writer = stub->collect(&context,&commands);

        try {
            boost::interprocess::message_queue mq(boost::interprocess::open_only, info->log_mq_name);

            while (true) {
                std::string data;
                data.resize(SKYWALKING_G(logging_mq_max_message_length));
                size_t msg_size;
                unsigned msg_priority;
                mq.receive(&data[0], data.size(), msg_size, msg_priority);
                data.resize(msg_size);

                skywalking::v3::LogData logData;
                logData.ParseFromString(data);
                bool status = writer->Write(logData);
                if (SKYWALKING_G(log_enable)) {
                    std::string json_str;
                    std::string result = status ? "success" : "fail";
                    google::protobuf::util::JsonPrintOptions opt;
                    opt.always_print_primitive_fields = true;
                    opt.preserve_proto_field_names = true;
                    google::protobuf::util::MessageToJsonString(logData, &json_str, opt);
                    sky_log("logging write " + result + ", data:" + json_str);
                }
                
                // retry oap server
                if (!status){
                    break;
                }
            }
        } catch (boost::interprocess::interprocess_exception &ex) {
            sky_log("logConsumer error: " + std::string(ex.what()));
            php_error(E_WARNING, "%s %s", "[skywalking] logConsumer open queue fail ", ex.what());
        }
    }
}

void LogginManager::cleanup() {
    boost::interprocess::message_queue::remove(s_info->log_mq_name);
}

std::shared_ptr<grpc::ChannelCredentials> LogginManager::getCredentials(const ManagerOptions &options) {
    std::shared_ptr<grpc::ChannelCredentials> creds;
    if (options.grpc_tls == true) {
        if (options.cert_chain.empty() && options.private_key.empty()) {
            creds = grpc::SslCredentials(grpc::SslCredentialsOptions());
        } else {
            grpc::SslCredentialsOptions opts;
            opts.pem_cert_chain = options.cert_chain;
            opts.pem_root_certs = options.root_certs;
            opts.pem_private_key = options.private_key;
            creds = grpc::SslCredentials(opts);
        }
    } else {
        creds = grpc::InsecureChannelCredentials();
    }
    return creds;
}