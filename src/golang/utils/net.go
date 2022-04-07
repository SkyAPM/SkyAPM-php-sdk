package utils

import "net"

func GetLocalIP() ([]string, error) {
	var ips []string
	adders, err := net.InterfaceAddrs()
	if err != nil {
		return ips, err
	}
	for _, addr := range adders {
		n, ok := addr.(*net.IPNet)
		if !ok {
			continue
		}
		if n.IP.IsLoopback() {
			continue
		}
		if !n.IP.IsGlobalUnicast() {
			continue
		}
		ips = append(ips, n.IP.String())
	}
	return ips, nil
}
