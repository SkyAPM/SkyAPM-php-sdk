#!/usr/bin/env bash

#
# Copyright 2021 SkyAPM
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

export BASEDIR=$(dirname "$0")/..
export TEMP_DIR="$BASEDIR"/temp
export PROTOCOL_DIR="$BASEDIR"/src/protocol
export COLLECT_DIR="$BASEDIR"/src/collect

if [[ ! -d "$TEMP_DIR" ]]; then
  mkdir "$TEMP_DIR"
else
  rm -rf "$TEMP_DIR"/*
fi

protoc --proto_path="$PROTOCOL_DIR" --go_out="$TEMP_DIR" --go-grpc_out="$TEMP_DIR" "$PROTOCOL_DIR"/*/*.proto "$PROTOCOL_DIR"/*/*/*.proto
rm -fr "$COLLECT_DIR"
mv "$TEMP_DIR"/skywalking.apache.org/repo/goapi/collect "$BASEDIR"/src/
rm -fr "$TEMP_DIR"

# fix
SED_REP='s:skywalking.apache.org/repo/goapi:github.com/SkyAPM/SkyAPM-php-sdk/src:g'
if uname -s | grep Darwin; then
  find "$COLLECT_DIR" -type f -name "*.go" | xargs sed -i '' $SED_REP
elif uname -s | grep Linux; then
  find "$COLLECT_DIR" -type f -name "*.go" | xargs sed -i $SED_REP
fi
find "$COLLECT_DIR" -name "*Compat.pb.go" -exec rm {} \;
find "$COLLECT_DIR" -name "*Compat_grpc.pb.go" -exec rm {} \;