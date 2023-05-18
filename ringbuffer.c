#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ringbuffer.h"


int
MKFNAME(reset) (struct rb *b) {
    if (is_power_of_two() == -1) {
        return -1;
    }

    b->r = 0;
    b->w = 0;
    memset(b->buff, 0, RBSIZE);

    return 0;
}


int
MKFNAME(available) (struct rb *b) {
    if (is_power_of_two() == -1) {
        return -1;
    }

    int available;
 
    if (b->w >= b->r) {
        available = (RBSIZE - (b->w - b->r)) - 1;
        return available;
    }
    available = b->r - b->w - 1;
    return available;
}
