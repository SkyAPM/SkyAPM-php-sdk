#!/bin/bash
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

VERSION=5.0.0
OS=`uname`

echo "OS: $OS"
echo "version: $VERSION"

if [ "$OS" = 'Darwin' ]; then
    sed -i '' "s/\"[0-9].[0-9].[0-9]\"/\"$VERSION\"/" php_skywalking.h
    sed -i '' "s/v[0-9].[0-9].[0-9].tar.gz/v$VERSION.tar.gz/g" docs/BUILDING.md
    sed -i '' "s/k-[0-9].[0-9].[0-9]/k-$VERSION/" docs/BUILDING.md
fi