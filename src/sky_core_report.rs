pub mod skywalking_proto {
    pub mod v3 {
        tonic::include_proto!("skywalking.v3");
    }
}

pub mod reporter;

#[no_mangle]
pub extern "C" fn greet() {
    println!("Hello, world!")
}