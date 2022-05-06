//
// Created by Yanlong He on 29/4/22.
//

#ifndef SKYWALKING_SKY_CORE_REPORT_H
#define SKYWALKING_SKY_CORE_REPORT_H

typedef struct sky_core_report_t {
    char *address;
    char *service;
    char *service_instance;
} sky_core_report_t;

sky_core_report_t *sky_core_report_new(char *address, char *service, char *service_instance);

void sky_core_report_push(sky_core_report_t *report, char *json);

#endif //SKYWALKING_SKY_CORE_REPORT_H
