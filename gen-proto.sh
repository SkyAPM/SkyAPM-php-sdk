#!/usr/bin/env bash

rm -r /tmp/protoc-tmp
mkdir /tmp/protoc-tmp

# gen v1
protoc -I reporter/protocol/v1 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v1/common/common.proto
protoc -I reporter/protocol/v1 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v1/common/CLR.proto reporter/protocol/v1/common/JVM.proto reporter/protocol/v1/common/trace-common.proto
protoc -I reporter/protocol/v1 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v1/language-agent/*.proto

# gen v2
protoc -I reporter/protocol/v2 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v2/common/common.proto
protoc -I reporter/protocol/v2 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v2/common/CLR.proto reporter/protocol/v2/common/JVM.proto reporter/protocol/v2/common/trace-common.proto
protoc -I reporter/protocol/v2 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v2/language-agent-v2/*.proto
protoc -I reporter/protocol/v2 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v2/profile/*.proto
protoc -I reporter/protocol/v2 --go_out=plugins=grpc:/tmp/protoc-tmp reporter/protocol/v2/register/*.proto

# gen v3
protoc -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` --cpp_out=src/network/v3 src/protocol/v3/common/Common.proto
protoc -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` --cpp_out=src/network/v3 src/protocol/v3/language-agent/*.proto
protoc -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` --cpp_out=src/network/v3 src/protocol/v3/profile/*.proto
protoc -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` --cpp_out=src/network/v3 src/protocol/v3/management/*.proto
find src -name "*.grpc.pb.cc" | while read id; do mv $id ${id/.grpc/_grpc}; done



find /tmp/protoc-tmp -type f -print0 | xargs -0 sed -i "" "s/skywalking\/network/github.com\/SkyAPM\/SkyAPM-php-sdk\/reporter\/network/g"
rm -r reporter/network
mv /tmp/protoc-tmp/skywalking/* reporter
rm -r /tmp/protoc-tmp
echo "v3 gen success"

