// Copyright 2022 SkyAPM
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
use std::fs::OpenOptions;
use std::io::Write;
use crate::skywalking_proto::v3::SegmentObject;
use crate::skywalking_proto::v3::InstanceProperties;
use crate::skywalking_proto::v3::KeyStringValuePair;
use crate::skywalking_proto::v3::InstancePingPkg;
use crate::skywalking_proto::v3::management_service_client::ManagementServiceClient;
use crate::skywalking_proto::v3::trace_segment_report_service_client::TraceSegmentReportServiceClient;
use tonic::transport::Channel;
use std::os::raw::c_char;
use std::thread;
use std::time::Duration;
use std::env;
use std::ffi::{CStr, CString};
use gethostname::gethostname;
use std::process;
use local_ip_address::local_ip;
use uuid::Uuid;
use tokio;
use serde_json;
use futures_util::stream;

pub type ManagementClient = ManagementServiceClient<Channel>;
pub type SegmentReportClient = TraceSegmentReportServiceClient<Channel>;

#[derive(Debug)]
#[repr(C)]
pub struct Reporter {
    pub address: *const c_char,
    pub service: *const c_char,
    pub service_instance: *const c_char,
}

impl Reporter {
    pub fn new(address: String, service: String, mut service_instance: String) -> Self {
        if service_instance == "" {
            service_instance = Uuid::new_v4().to_string() + "@" + &local_ip().unwrap().to_string();
        }

        let new_service = service.clone();
        let new_service_instance = service_instance.clone();
        thread::spawn(move || {
            let k_service = service.clone();
            let k_service_instance = service_instance.clone();
            tokio::runtime::Builder::new_current_thread()
                .worker_threads(4)
                .enable_all()
                .build()
                .unwrap()
                .block_on(async move {
                    let mut management = ManagementClient::connect("http://10.122.94.115:11800").await.unwrap();
                    let mut properties = Vec::<KeyStringValuePair>::new();
                    properties.push(KeyStringValuePair {
                        key: "os_name".to_string(),
                        value: env::consts::OS.to_string(),
                    });
                    properties.push(KeyStringValuePair {
                        key: "host_name".to_string(),
                        value: gethostname().to_str().unwrap().to_string(),
                    });
                    properties.push(KeyStringValuePair {
                        key: "process_no".to_string(),
                        value: process::id().to_string(),
                    });
                    properties.push(KeyStringValuePair {
                        key: "language".to_string(),
                        value: "php".to_string(),
                    });
                    properties.push(KeyStringValuePair {
                        key: "ipv4".to_string(),
                        value: local_ip().unwrap().to_string(),
                    });

                    let response = management.report_instance_properties(tonic::Request::new(InstanceProperties {
                        service: k_service,
                        service_instance: k_service_instance,
                        properties,
                        layer: "".to_string(),
                    })).await.unwrap();
                    println!("RESPONSE={:?}", response);
                });
            loop {
                let r_service = service.clone();
                let r_service_instance = service_instance.clone();
                tokio::runtime::Builder::new_current_thread()
                    .worker_threads(4)
                    .enable_all()
                    .build()
                    .unwrap()
                    .block_on(async move {
                        let mut management = ManagementClient::connect("http://10.122.94.115:11800").await.unwrap();
                        let response = management.keep_alive(tonic::Request::new(InstancePingPkg {
                            service: r_service,
                            service_instance: r_service_instance,
                            layer: "".to_string(),
                        })).await.unwrap();
                        println!("RESPONSE={:?}", response);
                    });
                thread::sleep(Duration::from_secs(1));
            }
        });

        return Self {
            address: CString::new(address).expect("").into_raw(),
            service: CString::new(new_service).expect("").into_raw(),
            service_instance: CString::new(new_service_instance).expect("").into_raw(),
        };
    }
    pub fn push(&self, json: String) {
        println!("{}", json);
        let segment: SegmentObject = serde_json::from_str(json.as_str()).unwrap();

        tokio::runtime::Builder::new_current_thread()
            .worker_threads(4)
            .enable_all()
            .build()
            .unwrap()
            .block_on(async move {
                let mut segment_report = SegmentReportClient::connect("http://10.122.94.115:11800").await.unwrap();
                let response = segment_report.collect(stream::iter(vec![segment])).await.unwrap();
                println!("RESPONSE={:?}", response);
            });

        // println!("{:?}", segment);
    }
}