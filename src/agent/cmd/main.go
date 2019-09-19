package main

import (
	"agent/agent/logger"
	"agent/agent/service"
	"github.com/urfave/cli"
	"os"
)

var log = logger.Log

func main() {
	app := cli.NewApp()
	app.Name = "sky_php_agent"
	app.Usage = "the skywalking trace sending agent"
	app.Flags = []cli.Flag{
		cli.StringFlag{Name: "grpc", Usage: "--grpc", Value: "127.0.0.1:10800"},
		cli.StringFlag{Name: "socket", Usage: "--socket", Value: "/tmp/sky_agent.sock"},
		cli.IntFlag{Name: "queue", Usage: "--queue", Value: 100},
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
