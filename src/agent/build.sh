#!/bin/bash
GOOS=darwin GOARCH=amd64 go build -o sky_php_agent_darwin_x64 agent.go
GOOS=darwin GOARCH=386 go build -o sky_php_agent_darwin_x86 agent.go

GOOS=linux GOARCH=amd64 go build -o sky_php_agent_linux_x64 agent.go
GOOS=linux GOARCH=386 go build -o sky_php_agent_linux_x86 agent.go

GOOS=linux GOARCH=arm go build -o sky_php_agent_linux_arm86 agent.go
GOOS=linux GOARCH=arm64 go build -o sky_php_agent_linux_arm64 agent.go
