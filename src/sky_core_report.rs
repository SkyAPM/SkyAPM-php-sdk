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
use std::os::raw::c_char;
use std::boxed::Box;
use std::ffi::{CStr, CString};
use reporter::grpc::Reporter;
use tokio;
use uuid::Uuid;
use std::process;
use rand::Rng;
use rand;

pub mod skywalking_proto {
    pub mod v3 {
        tonic::include_proto!("skywalking.v3");
    }
}

pub mod reporter;

#[no_mangle]
extern "C" fn sky_core_report_trace_id() -> *const c_char {
    let mut rng: i32 = rand::thread_rng().gen_range(100000..999999);
    let mut trace_id = format!("{}.{}.{}", Uuid::new_v4().to_string(), process::id().to_string(), rng.to_string());
    trace_id = trace_id.replace("-", "");
    return CString::new(trace_id).expect("").into_raw();
}

#[no_mangle]
extern "C" fn sky_core_report_new(c_address: *const c_char, c_service: *const c_char, c_service_instance: *const c_char) -> *const Reporter {
    let address = unsafe { CStr::from_ptr(c_address) };
    let service = unsafe { CStr::from_ptr(c_service) };
    let service_instance = unsafe { CStr::from_ptr(c_service_instance) };
    let report = Reporter::new(address.to_str().unwrap().to_owned(), service.to_str().unwrap().to_owned(), service_instance.to_str().unwrap().to_owned());
    println!("{}", unsafe { CStr::from_ptr(report.address).to_str().unwrap() });
    Box::into_raw(Box::new(report)) as *const Reporter
}

#[no_mangle]
extern "C" fn sky_core_report_push(ptr: *mut Reporter, json: *const c_char) {
    if ptr.is_null() {
        return;
    }

    let report = unsafe {
        &mut *(ptr as *mut Reporter)
    };

    let c_json = unsafe { CStr::from_ptr(json) };
    report.push(c_json.to_str().unwrap().to_owned());
}