#pragma once

#include <stdio.h>

#include <sys/time.h>

#include "config.h"

typedef struct {
    size_t id;
    int time;
    struct timeval sys_time;
    double channels[CHANNELS];
} msg_t;

void print_msg(msg_t &msg);