package main

import (
	"agent/agent/logger"
	"agent/agent/service"
	"github.com/urfave/cli"
	"os"
)

var log = logger.Log

func main() {

	defer func() {
		if err := recover(); err != nil {
			log.Error(err)
		}
	}()

	app := cli.NewApp()
	app.Name = "sky_php_agent"
	app.Usage = "the skywalking trace sending agent"
	app.Flags = []cli.Flag{
		cli.StringFlag{Name: "grpc", Usage: "SkyWalking collector grpc address", Value: "127.0.0.1:11800"},
		cli.StringFlag{Name: "socket", Usage: "Pipeline for communicating with PHP", Value: "/var/run/sky-agent.sock"},
		cli.IntFlag{Name: "send-rate", Usage: "Send trace 1 second by default", Value: 1},
	}

	app.Action = func(c *cli.Context) error {

		a := service.NewAgent(c)
		a.Run()
		return nil
	}

	err := app.Run(os.Args)
	if err != nil {
		log.Errorln(err)
	}
}
