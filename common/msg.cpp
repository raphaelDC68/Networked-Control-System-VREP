#include "msg.h"

void print_msg(msg_t &msg)
{
    printf("msg_t:\n");
    printf("\tid: %d\n", (int) msg.id);
    printf("\ttime: %d\n", msg.time);
    for(int i = 0; i < CHANNELS; i++) {
        printf("\t[%d]: %6.4lf\n", i, msg.channels[i]);
    }
    printf("\n");
}