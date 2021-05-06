#!/bin/bash

VERSION=4.1.2
OS=`uname`

echo "OS: $OS"
echo "version: $VERSION"

if [ "$OS" = 'Darwin' ]; then
    sed -i '' "s/\"[0-9].[0-9].[0-9]\"/\"$VERSION\"/" php_skywalking.h
    sed -i '' "s/v[0-9].[0-9].[0-9].tar.gz/v$VERSION.tar.gz/g" docs/install.md
    sed -i '' "s/k-[0-9].[0-9].[0-9]/k-$VERSION/" docs/install.md
fi