package utils

import (
	"fmt"
	"github.com/google/uuid"
	"math/rand"
	"os"
	"strings"
	"time"
)

func GenerateTraceId() string {
	rand.Seed(time.Now().UnixNano())
	traceId := fmt.Sprintf("%s.%d.%d", uuid.New().String(), os.Getpid(), rand.Intn(999999)+5000)
	return strings.Replace(traceId, "-", "", -1)
}
