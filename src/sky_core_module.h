//
// Created by Yanlong He on 12/4/22.
//

#ifndef SKYWALKING_SKY_CORE_MODULE_H
#define SKYWALKING_SKY_CORE_MODULE_H

#include "php.h"
#include "pthread.h"

#define SKY_FIND_FUNC(name) zend_hash_str_find_ptr(CG(function_table), name, sizeof(name) - 1)

typedef struct skywalking_t {
    char service[1024];
    char serviceInstance[1024];
    char *connect;
} skywalking_t;

int sky_core_module_init(INIT_FUNC_ARGS);

static ZEND_RSRC_DTOR_FUNC(skywalking_dtor);

void sky_core_module_free();

void sky_core_request_init(zval *request, u_int64_t request_id);

void sky_core_request_free(zval *response, u_int64_t request_id);

#endif //SKYWALKING_SKY_CORE_MODULE_H
