//
// Created by Yanlong He on 8/4/22.
//

#ifndef SKYWALKING_SKY_GO_UTILS_H
#define SKYWALKING_SKY_GO_UTILS_H

#include <string>
#include "sky_go_wrapper.h"

GoString NewGoString(const char *p, size_t n);

std::string NewString(GoString s);

#endif //SKYWALKING_SKY_GO_UTILS_H
