package service

import (
	"io"
	"net"
	"strings"
)

type Conn struct {
	agent *Agent
	c     net.Conn
}

func NewConn(a *Agent, c net.Conn) *Conn {
	var conn = &Conn{
		agent: a,
		c:     c,
	}

	return conn
}

func (c *Conn) Handle() {

	defer func() {
		c.c.Close()
	}()

	buf := make([]byte, 4096)
	var json string
	var endIndex int
	for {
		n, err := c.c.Read(buf)
		if err != nil {
			if err != io.EOF {
				log.Warn("conn read error:", err)
			}
			return
		}
		json += string(buf[0:n])
		for {
			endIndex = strings.IndexAny(json, "\n")
			if endIndex >= 0 {
				body := json[0:endIndex]
				if body[:1] == "0" {
					c.agent.register <- &register{
						c:    c.c,
						body: body[1:],
					}
				} else if body[:1] == "1" {
					c.agent.trace <- body[1:]
				}
				json = json[endIndex+1:]
			} else {
				break
			}
		}
	}
}
