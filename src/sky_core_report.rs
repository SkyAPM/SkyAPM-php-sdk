use std::os::raw::c_char;
use std::boxed::Box;
use std::ffi::{CStr, CString};
use reporter::grpc::Reporter;
use tokio;

pub mod skywalking_proto {
    pub mod v3 {
        tonic::include_proto!("skywalking.v3");
    }
}

pub mod reporter;

#[no_mangle]
extern "C" fn sky_core_report_new(c_address: *const c_char, c_service: *const c_char, c_service_instance: *const c_char) -> *const Reporter {
    let address = unsafe { CStr::from_ptr(c_address) };
    let service = unsafe { CStr::from_ptr(c_service) };
    let service_instance = unsafe { CStr::from_ptr(c_service_instance) };
    let report = Reporter::new(address.to_str().unwrap().to_owned(), service.to_str().unwrap().to_owned(), service_instance.to_str().unwrap().to_owned());
    println!("{}", unsafe{CStr::from_ptr(report.address).to_str().unwrap()});
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