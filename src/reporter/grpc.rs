use crate::skywalking_proto::v3::SegmentObject;
use crate::skywalking_proto::v3::management_service_client::ManagementServiceClient;
use tonic::transport::Channel;

pub type ManagementClient = ManagementServiceClient<Channel>;

pub struct Reporter {
    
}

impl Reporter {
    pub async fn start(address: impl Into<String>) -> Self {
        let mut reporter = ManagementClient::connect(address.into()).await.unwrap();
    }
}