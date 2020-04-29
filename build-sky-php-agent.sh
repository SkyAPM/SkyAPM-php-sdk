#!/bin/bash
GOOS=darwin GOARCH=amd64 go build -o sky-php-agent-darwin-x64 ./cmd/main.go
GOOS=darwin GOARCH=386 go build -o sky-php-agent-darwin-x86 ./cmd/main.go

GOOS=linux GOARCH=amd64 go build -o sky-php-agent-linux-x64 ./cmd/main.go
GOOS=linux GOARCH=386 go build -o sky-php-agent-linux-x86 ./cmd/main.go

GOOS=linux GOARCH=arm go build -o sky-php-agent-linux-arm86 ./cmd/main.go
GOOS=linux GOARCH=arm64 go build -o sky-php-agent-linux-arm64 ./cmd/main.go
