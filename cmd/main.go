package main

import (
	"github.com/SkyAPM/SkyAPM-php-sdk/reporter/logger"
	"github.com/SkyAPM/SkyAPM-php-sdk/reporter/service"
	cli "github.com/urfave/cli/v2"
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
	app.Version = "3.3.1"
	app.Flags = []cli.Flag{
		&cli.StringSliceFlag{Name: "grpc", Usage: "SkyWalking collector address", Value: cli.NewStringSlice("127.0.0.1:11800")},
		&cli.StringFlag{Name: "socket", Usage: "Pipeline for communicating with PHP", Value: "/tmp/sky-agent.sock"},
		&cli.IntFlag{Name: "send-rate", Usage: "Send trace 1 second by default", Value: 1},
		&cli.IntFlag{Name: "sky-version", Usage: "SkyWalking version", Value: 8},
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
