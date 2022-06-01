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

use anyhow::{anyhow, bail, Context};
use ipc_channel::ipc::{bytes_channel, IpcBytesReceiver, IpcBytesSender, IpcSharedMemory};
use std::{
    mem::{size_of, transmute},
    sync::{
        atomic::{AtomicUsize, Ordering},
        Mutex,
    },
};
use tokio::sync::OnceCell;

const MAX_MESSAGE_QUEUE_COUNT: usize = 100;

static MESSAGE_QUEUE_SENDER: OnceCell<Mutex<IpcBytesSender>> = OnceCell::const_new();
static MESSAGE_QUEUE_RECEIVER: OnceCell<Mutex<IpcBytesReceiver>> = OnceCell::const_new();
static MESSAGE_QUEUE_COUNT: OnceCell<IpcSharedMemory> = OnceCell::const_new();
static MESSAGE_QUEUE_MAX_LENGTH: AtomicUsize = AtomicUsize::new(0);

pub fn init(max_length: usize) -> anyhow::Result<()> {
    // init count
    let count: [u8; size_of::<AtomicUsize>()] = unsafe { transmute(AtomicUsize::new(0)) };
    MESSAGE_QUEUE_COUNT.set(IpcSharedMemory::from_bytes(&count))?;

    // init channel
    MESSAGE_QUEUE_MAX_LENGTH.store(max_length, Ordering::SeqCst);

    let (sender, receiver) = bytes_channel()?;
    MESSAGE_QUEUE_SENDER.set(Mutex::new(sender))?;
    MESSAGE_QUEUE_RECEIVER.set(Mutex::new(receiver))?;

    Ok(())
}

pub fn send(data: &[u8]) -> anyhow::Result<()> {
    let max_length = MESSAGE_QUEUE_MAX_LENGTH.load(Ordering::SeqCst);
    if data.len() > max_length {
        bail!("send data is too big");
    }

    let old_count = get_message_queue_count()?.fetch_add(1, Ordering::SeqCst);

    if old_count >= MAX_MESSAGE_QUEUE_COUNT {
        get_message_queue_count()?.fetch_min(MAX_MESSAGE_QUEUE_COUNT, Ordering::SeqCst);
        bail!("message queue is fulled");
    }

    let lock = MESSAGE_QUEUE_SENDER
        .get()
        .context("message queue sender: channel hasn't initialized or failed")?
        .try_lock()
        .map_err(|e| anyhow!("message queue sender is locked: {:?}", e))?;
    lock.send(data)?;
    Ok(())
}

pub fn receive() -> anyhow::Result<Vec<u8>> {
    let lock = MESSAGE_QUEUE_RECEIVER
        .get()
        .context("message queue receiver: channel hasn't initialized or failed")?
        .lock()
        .map_err(|e| anyhow!("message queue receiver get lock failed: {:?}", e))?;
    let buffer = lock
        .recv()
        .map_err(|e| anyhow!("message queue receive failed: {:?}", e))?;

    get_message_queue_count()?.fetch_sub(1, Ordering::SeqCst);

    Ok(buffer)
}

fn get_message_queue_count<'a>() -> anyhow::Result<&'a AtomicUsize> {
    let count = MESSAGE_QUEUE_COUNT
        .get()
        .context("message queue count: channel hasn't initialized or failed")?
        .as_ptr() as *const AtomicUsize;
    unsafe { count.as_ref().context("why message queue count is null") }
}