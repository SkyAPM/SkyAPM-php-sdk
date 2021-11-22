# Get started quickly with docker

See [Dockerfile](../docker/Dockerfile)

## How to use this image
```
$ docker run --restart always -d -e SW_OAP_ADDRESS=oap:11800 skyapm/skywalking-php
```

## Configuration

### SW_OAP_ADDRESS

The address of OAP server. Default value is oap:11800.
