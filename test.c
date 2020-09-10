#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>

int main() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("test time %ld", tv.tv_sec*1000 + tv.tv_usec/1000);
}