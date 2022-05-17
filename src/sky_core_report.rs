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
#[macro_use]
extern crate log;
extern crate simplelog;

use std::os::raw::{c_char, c_void};
use std::boxed::Box;
use std::cmp::max;
use std::ffi::{CStr, CString};
use reporter::grpc::Reporter;
use reporter::{grpc, ipc};
use tokio;
use local_ip_address::local_ip;
use uuid::Uuid;
use std::process;
use rand::Rng;
use rand;
use std::thread;
use std::ptr::null_mut;
use std::{
    slice::from_raw_parts,
};

pub mod skywalking_proto {
    pub mod v3 {
        tonic::include_proto!("skywalking.v3");
    }
}

pub mod reporter;

#[no_mangle]
extern "C" fn sky_core_report_ipc_init(max_length: usize) -> bool {
    return match ipc::init(max_length) {
        Ok(_) => {
            log::debug!("sky_core_report_ipc_init ok");
            true
        }
        Err(e) => {
            log::error!("sky_core_report_ipc_init err {}", e.to_string());
            false
        }
    };
}

#[no_mangle]
extern "C" fn sky_core_report_ipc_send(data: *const c_char, len: usize) -> bool {
    let data = unsafe { from_raw_parts(data.cast(), len) };
    return match ipc::send(data) {
        Ok(_) => {
            log::debug!("sky_core_report_ipc_send ok");
            true
        }
        Err(e) => {
            log::error!("sky_core_report_ipc_send err {}", e.to_string());
            false
        }
    };
}

#[no_mangle]
extern "C" fn sky_core_report_trace_id() -> *const c_char {
    let mut rng: i32 = rand::thread_rng().gen_range(100000..999999);
    let mut trace_id = format!("{}.{}.{}", Uuid::new_v4().to_string(), process::id().to_string(), rng.to_string());
    trace_id = trace_id.replace("-", "");
    return CString::new(trace_id).expect("").into_raw();
}


#[no_mangle]
extern "C" fn sky_core_service_instance_id() -> *const c_char {
    let service_instance = Uuid::new_v4().to_string() + "@" + &local_ip().unwrap().to_string();
    return CString::new(service_instance).expect("").into_raw();
}

#[no_mangle]
extern "C" fn sky_core_report_new(
    address: *const c_char,
    service: *const c_char,
    service_instance: *const c_char,
    log_level: *const c_char,
    log_path: *const c_char,
) -> bool {
    let f = || unsafe {
        let address = CStr::from_ptr(address).to_str()?;
        let service = CStr::from_ptr(service).to_str()?;
        let service_instance = CStr::from_ptr(service_instance).to_str()?;
        let log_level = CStr::from_ptr(log_level).to_str()?;
        let log_path = CStr::from_ptr(log_path).to_str()?;
        grpc::init(
            address.to_string(),
            service.to_string(),
            service_instance.to_string(),
            log_level.to_string(),
            log_path.to_string(),
        )?;
        Ok::<_, anyhow::Error>(())
    };
    match f() {
        Ok(_) => true,
        Err(e) => false
    }
}