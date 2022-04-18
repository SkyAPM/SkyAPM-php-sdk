//
// Created by Yanlong He on 18/4/22.
//

#include "sky_util_php.h"

void *sky_util_find_obj_func(const char *obj, const char *name) {
    zend_class_entry *ce = zend_hash_str_find_ptr(CG(class_table), obj, strlen(obj));
    if (ce != NULL) {
        return zend_hash_str_find_ptr(&ce->function_table, name, strlen(name));
    }
    return NULL;
}