#!/bin/bash
GOOS=darwin GOARCH=amd64 go build -o sky-php-agent-darwin-x64 .
GOOS=darwin GOARCH=386 go build -o sky-php-agent-darwin-x86 .

GOOS=linux GOARCH=amd64 go build -o sky-php-agent-linux-x64 .
GOOS=linux GOARCH=386 go build -o sky-php-agent-linux-x86 .

GOOS=linux GOARCH=arm go build -o sky-php-agent-linux-arm86 .
GOOS=linux GOARCH=arm64 go build -o sky-php-agent-linux-arm64 .
