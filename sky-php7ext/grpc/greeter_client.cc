/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "ApplicationRegisterService.grpc.pb.h"
#include "DiscoveryService.grpc.pb.h"
#include "TraceSegmentService.grpc.pb.h"

#include "common_struct.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;


extern "C" int goSkyGrpc(char*host, int  method, ParamDataStruct* paramData, void (*callfuct)( AppInstance* ) ); 

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel) {
    channel_ = channel;
  }

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  AppInstance* goRegister(const std::string& appCode) {
    std::unique_ptr<ApplicationRegisterService::Stub> stub_;
    stub_ =  ApplicationRegisterService::NewStub(channel_) ;
    // Data we are sending to the server.
    Application request;
    request.set_applicationcode( appCode );
    // Container for the data we expect from the server.
    ApplicationMapping reply;
    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;


    KeyWithIntegerValue kwiv;
    // The actual RPC.
    Status status = stub_->applicationCodeRegister(&context, request, &reply);
    AppInstance* appInstance = (AppInstance*)malloc(sizeof(AppInstance));
    // Act upon its status.
    if (status.ok()) {
      kwiv = reply.application();
      appInstance->applicationId  = kwiv.value();
      return appInstance;
    }
    return NULL;
  }

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  AppInstance* goRegisterInstance(
            std::string agentUUID, 
            std::string osName, 
            std::string hostname, 
            std::string ipv4s, 
            int applicationId,  
            long registerTime,   
            int processNo
      ) {

    std::unique_ptr<InstanceDiscoveryService::Stub> stub_;
    stub_  = InstanceDiscoveryService::NewStub(channel_);
    // Data we are sending to the server.
    ApplicationInstance request;
    request.set_applicationid( applicationId );
    request.set_agentuuid( agentUUID );
    request.set_registertime( registerTime );

    OSInfo* osinfo = new OSInfo;
    request.set_allocated_osinfo( osinfo );
    osinfo->set_osname( osName );
    osinfo->set_hostname( hostname );
    osinfo->set_processno( processNo );
    osinfo->add_ipv4s( ipv4s );

    AppInstance* appInstancestruct = (AppInstance*)malloc(sizeof(AppInstance));

    // Container for the data we expect from the server.
    ApplicationInstanceMapping reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->registerInstance(&context, request, &reply);
    //free OSInfo
    //delete osinfo;
    // Act upon its status.
    if (status.ok()) {

      appInstancestruct->applicationId = reply.applicationid();
      appInstancestruct->applicationInstanceId = reply.applicationinstanceid();
      return appInstancestruct;
    }
    return NULL;

    
  }


  void  sendTrace( TraceSegmentObjectStruct* traceSegment  ) {


    std::unique_ptr<TraceSegmentService::Stub> stub_;

    stub_  = TraceSegmentService::NewStub(channel_);
    // Data we are sending to the server.
    UpstreamSegment request ;
    //UniqueId
    UniqueId* uniqueId;

    uniqueId = request.add_globaltraceids();
    comListNode* com_list = traceSegment->traceSegmentIdList->head;
    while(com_list != NULL)
    {
        uniqueId->add_idparts(com_list->data->idParts);
        com_list = com_list->next;
    }
    if( traceSegment->segment ){
      request.set_segment(traceSegment->segment);

      Downstream reply;

      ClientContext context;

      grpc_connectivity_state  state = channel_->GetState(false);
      
      if( state == GRPC_CHANNEL_TRANSIENT_FAILURE  ){
        return ;
      }

      std::unique_ptr<ClientWriter<UpstreamSegment> > writer( stub_->collect(&context, &reply));
      writer->Write(request);
      writer->WritesDone();
      writer->Finish();
     
    }
   
  }

 private:
  std::shared_ptr<Channel> channel_; 
};



int goSkyGrpc(char *host, int method, ParamDataStruct* paramData, void (*callfuct)( AppInstance* ) ) {

  std::shared_ptr<Channel> channel =  grpc::CreateChannel(
      host, grpc::InsecureChannelCredentials());
  static GreeterClient greeter(channel);

  switch (method) {
          case METHOD__GO_REGISTER:
            {
              std::string appCode( paramData->registerParam->appCode);
              callfuct(greeter.goRegister(appCode));
            }
            break;
          case METHOD__GO_REGISTER_INSTANCE:

            {
              std::string agentUUID(paramData->registerInstanceParam->agentUUID);
              std::string osName(paramData->registerInstanceParam->osName);
              std::string hostname(paramData->registerInstanceParam->hostname);
              std::string ipv4s(paramData->registerInstanceParam->ipv4s);
              int applicationId = paramData->registerInstanceParam->applicationId;
              long registerTime = paramData->registerInstanceParam->registerTime;
              int processNo = paramData->registerInstanceParam->processNo;
              callfuct(
                greeter.goRegisterInstance(
                    agentUUID, 
                    osName, 
                    hostname, 
                    ipv4s, 
                    applicationId,
                    registerTime,
                    processNo
                    )
              );
            }
            break;
          case METHOD__SEND_TRACE_SEGMENT:
            {
              greeter.sendTrace( paramData->sendTraceSegmentParam->traceSegment  );
            } 
            break;
          default:
              return 0;
          break;
  }
 


  return 1;
}
