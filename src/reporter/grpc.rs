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
use std::io::Write;
use crate::skywalking_proto::v3::SegmentObject;
use crate::skywalking_proto::v3::InstanceProperties;
use crate::skywalking_proto::v3::KeyStringValuePair;
use crate::skywalking_proto::v3::InstancePingPkg;
use crate::skywalking_proto::v3::management_service_client::ManagementServiceClient;
use crate::skywalking_proto::v3::trace_segment_report_service_client::TraceSegmentReportServiceClient;
use tonic::{
    transport::{Channel, Endpoint},
    Request,
};
use std::os::raw::c_char;
use std::thread;
use std::time::Duration;
use std::env;
use std::ffi::{CStr, CString};
use gethostname::gethostname;
use std::process;
use local_ip_address::local_ip;
use uuid::Uuid;
use tokio::{
    spawn,
    sync::{mpsc, OnceCell},
    task::spawn_blocking,
    time::sleep,
};
use serde_json;
use futures_util::stream;
use anyhow::{anyhow, bail, Context};
use futures_util::future::ok;
use std::{
    str,
    sync::{
        atomic::{AtomicBool, AtomicI32, Ordering},
        Arc,
    },
    fs::{
        File, OpenOptions,
    },
    path::{
        Path
    },
};
use crate::ipc;


#[derive(Clone)]
struct CountedSender<T> {
    sender: mpsc::Sender<T>,
    // For batch receive for Receiver.
    count: Arc<AtomicI32>,
}

impl<T> CountedSender<T> {
    fn try_send(&self, message: T) -> anyhow::Result<()> {
        self.sender
            .try_send(message)
            .map_err(|_| anyhow!("send failed or fulled"))?;
        self.count.fetch_add(1, Ordering::SeqCst);
        Ok(())
    }
}

struct CountedReceiver<T> {
    receiver: mpsc::Receiver<T>,
    // For batch receive.
    count: Arc<AtomicI32>,
}

impl<T> CountedReceiver<T> {
    async fn recv_all(&mut self) -> anyhow::Result<Vec<T>> {
        let count = self.count.load(Ordering::SeqCst);
        if count <= 0 {
            return Ok(Vec::new());
        }
        let mut buffered = Vec::with_capacity(count as usize);
        for _ in 0..count {
            let data = self.receiver.recv().await.context("closed")?;
            self.count.fetch_sub(1, Ordering::SeqCst);
            buffered.push(data);
        }
        Ok(buffered)
    }
}

static GRPC_CHANNEL: OnceCell<Channel> = OnceCell::const_new();
static REGISTER: OnceCell<bool> = OnceCell::const_new();

pub type ManagementClient = ManagementServiceClient<Channel>;
pub type SegmentReportClient = TraceSegmentReportServiceClient<Channel>;

#[derive(Debug)]
#[repr(C)]
pub struct Reporter {
    pub address: *const c_char,
    pub service: *const c_char,
    pub service_instance: *const c_char,
}

pub fn init(address: String, service: String, mut service_instance: String, log_level: String, log_path: String) -> anyhow::Result<()> {
    let mut level = simplelog::LevelFilter::Debug;

    if log_level == "disable" {
        level = simplelog::LevelFilter::Off;
    } else if log_level == "error" {
        level = simplelog::LevelFilter::Error;
    } else if log_level == "warn" {
        level = simplelog::LevelFilter::Warn;
    } else if log_level == "info" {
        level = simplelog::LevelFilter::Info;
    } else if log_level == "debug" {
        level = simplelog::LevelFilter::Debug;
    } else if log_level == "trace" {
        level = simplelog::LevelFilter::Trace;
    }

    if log_level != "disable" {
        if !Path::new(log_path.clone().as_str()).exists() {
            File::create(log_path.clone()).unwrap();
        }

        simplelog::WriteLogger::init(
            level,
            simplelog::Config::default(),
            OpenOptions::new().append(true).write(true).open(log_path).unwrap(),
        ).unwrap();
    }


    let rt = tokio::runtime::Builder::new_multi_thread()
        .worker_threads(4)
        .enable_all()
        .build()?;
    rt.block_on(worker(address, service, service_instance));
    Ok(())
}

pub async fn worker(address: String, service: String, service_instance: String) {
    let (segment_sender, segment_receiver) = counted_channel(5000);
    spawn(do_connect(address.to_owned()));
    spawn(login(service.to_owned(), service_instance.to_owned()));
    spawn(keep_alive(service.to_owned(), service_instance.to_owned()));
    spawn(sender(segment_receiver));
    receive(segment_sender).await;
}


pub async fn connect(address: &str) -> anyhow::Result<Channel> {
    let channel = Endpoint::from_shared(format!("http://{}", address))?
        .timeout(Duration::from_secs(5))
        .keep_alive_timeout(Duration::from_secs(5))
        .http2_keep_alive_interval(Duration::from_secs(10))
        .connect()
        .await?;

    Ok(channel)
}

pub async fn do_connect(address: String) {
    let channel = loop {
        match connect(&address).await {
            Ok(channel) => {
                break channel;
            }
            Err(e) => {
                log::error!("do connect err {}", e.to_string())
            }
        }
        sleep(Duration::from_secs(1)).await;
    };

    GRPC_CHANNEL.set(channel);
}

pub async fn login(service: String, service_instance: String) {
    let props = vec![
        KeyStringValuePair {
            key: "os_name".to_owned(),
            value: env::consts::OS.to_string(),
        },
        KeyStringValuePair {
            key: "host_name".to_owned(),
            value: gethostname().to_str().unwrap().to_string(),
        },
        KeyStringValuePair {
            key: "process_no".to_owned(),
            value: process::id().to_string(),
        },
        KeyStringValuePair {
            key: "language".to_owned(),
            value: "php".to_owned(),
        },
        KeyStringValuePair {
            key: "ipv4".to_owned(),
            value: local_ip().unwrap().to_string(),
        },
    ];

    let instance = InstanceProperties {
        service: service.clone(),
        service_instance: service_instance.clone(),
        properties: props,
        layer: "".to_owned(),
    };

    loop {
        sleep(Duration::from_secs(1)).await;
        let channel = match GRPC_CHANNEL.get() {
            Some(channel) => channel,
            None => continue,
        };
        log::debug!("login instance {:?}", instance);
        match do_login(channel.clone(), instance.clone()).await {
            Ok(r) => {
                REGISTER.set(true);
                break;
            },
            Err(e) => {
                log::error!("login err {}", e.to_string());
                continue;
            }
        };
    }
}

pub async fn do_login(channel: Channel, instance: InstanceProperties) -> anyhow::Result<()> {
    let mut client = ManagementServiceClient::new(channel);
    client.report_instance_properties(Request::new(instance)).await?;
    Ok(())
}

pub async fn keep_alive(service: String, service_instance: String) {
    let instance = InstancePingPkg {
        service: service.clone(),
        service_instance: service_instance.clone(),
        layer: "".to_string(),
    };

    loop {
        sleep(Duration::from_secs(5)).await;
        let register = match REGISTER.get() {
            Some(register) => register,
            None => continue,
        };
        log::debug!("keep alive register {}", register);
        let channel = match GRPC_CHANNEL.get() {
            Some(channel) => channel,
            None => continue,
        };
        log::debug!("keep alive instance {:?}", instance);
        match do_keep_alive(channel.clone(), instance.clone()).await {
            Ok(r) => continue,
            Err(e) => continue
        };
    }
}

pub async fn do_keep_alive(channel: Channel, instance: InstancePingPkg) -> anyhow::Result<()> {
    let mut client = ManagementServiceClient::new(channel);
    client.keep_alive(Request::new(instance)).await?;
    Ok(())
}

async fn receive(segment_sender: CountedSender<SegmentObject>) {
    loop {
        if let Err(e) = receive_once(segment_sender.clone()).await {
            log::error!("receive err {}", e.to_string())
        }
    }
}

async fn receive_once(segment_sender: CountedSender<SegmentObject>) -> anyhow::Result<()> {
    let content = spawn_blocking(ipc::receive).await??;
    if content.is_empty() {
        bail!("receive content is empty");
    }

    let s = str::from_utf8(&content)?;
    debug!("receive_once segment {}", s);
    if !s.is_empty() {
        let segment: SegmentObject = serde_json::from_str(s)?;
        segment_sender.try_send(segment)?;
    }

    Ok(())
}

async fn sender(mut segment_receiver: CountedReceiver<SegmentObject>) {
    loop {
        let register = match REGISTER.get() {
            Some(register) => register,
            None => {
                sleep(Duration::from_secs(1)).await;
                continue;
            },
        };
        let channel = match GRPC_CHANNEL.get() {
            Some(channel) => channel,
            None => {
                sleep(Duration::from_secs(1)).await;
                continue;
            }
        };

        if let Err(e) = send_once(channel.clone(), &mut segment_receiver).await {
            log::error!("sender err {}", e.to_string())
        }
        sleep(Duration::from_micros(100)).await;
    }
}

async fn send_once(channel: Channel, segment_receiver: &mut CountedReceiver<SegmentObject>) -> anyhow::Result<()> {
    let segment = segment_receiver.recv_all().await?;
    if segment.is_empty() {
        return Ok(());
    }

    log::info!("send once segment {:?}", segment);
    do_collect(channel, segment).await?;
    Ok(())
}

async fn do_collect(channel: Channel, segment: Vec<SegmentObject>) -> anyhow::Result<()> {
    let mut client = TraceSegmentReportServiceClient::new(channel);
    client.collect(stream::iter(segment)).await?;
    Ok(())
}

pub fn get_channel() -> anyhow::Result<Channel> {
    match GRPC_CHANNEL.get() {
        Some(channel) => Ok(channel.clone()),
        None => {
            bail!("waiting for connect to grpc success");
        }
    }
}

fn counted_channel<T>(buffer: usize) -> (CountedSender<T>, CountedReceiver<T>) {
    let (sender, receiver) = mpsc::channel(buffer);
    let count = Arc::new(AtomicI32::new(0));
    (
        CountedSender {
            sender,
            count: count.clone(),
        },
        CountedReceiver { receiver, count },
    )
}