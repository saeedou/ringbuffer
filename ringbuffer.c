#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ringbuffer.h"


int
is_power_of_two() {
    if ((RBSIZE & (RBSIZE - 1)) == 0) {
        return 0;
    }

    return -1;
}


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


int
MKFNAME(put) (struct rb *b, T *data, int size) {
    int linear_empty_size;

    if (is_power_of_two() == -1) {
        return -1;
    }

    if (MKFNAME(available)(b) < size) {
        return -1;
    }

    linear_empty_size = RBSIZE - b->w;
    memcpy(b->buff + b->w, data, linear_empty_size);

    if (size > linear_empty_size) {
        memcpy(b->buff, data + linear_empty_size, linear_empty_size);
    }

    b->w = (b->w + size) & (RBSIZE - 1);
    return 0;
}


int
MKFNAME(pop)(struct rb *b, T *data, int size) {
    if (is_power_of_two() == -1) {
        return -1;
    }

    int used;
    int linear_used_size;

    used = RBSIZE - MKFNAME(available)(b) - 1;
    if (size > used) {
        return -1;
    }

    linear_used_size = RBSIZE - b->r;
    memcpy(data, b->buff + b->r, linear_used_size);

    if (size > linear_used_size) {
        memcpy(data + linear_used_size, b->buff, linear_used_size);
    }

    b->r = (b->r + size) & (RBSIZE - 1);
    return 0;
}
