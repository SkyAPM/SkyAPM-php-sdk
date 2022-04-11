//
// Created by Yanlong He on 8/4/22.
//

#include "sky_go_utils.h"

GoString NewGoString(const char *p, size_t n) {
    return {p, static_cast<ptrdiff_t>(n)};
}

std::string NewString(GoString s) {
    return std::string(s.p, s.n);
}
