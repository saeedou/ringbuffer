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
    if (is_power_of_two() == -1) {
        return -1;
    }

    if (MKFNAME(available)(b) < size) {
        return -1;
    }
 
    for (int i = 0; i < size; i++) {
        b->buff[b->w] = data[i];
 
        b->w = (b->w + 1) & (RBSIZE - 1);
    }
    return 0;
}


int
MKFNAME(pop)(struct rb *b, T *data, int size) {
    if (is_power_of_two() == -1) {
        return -1;
    }

    int used;
    int buffindex;

    used = RBSIZE - MKFNAME(available)(b) - 1;
    if (size > used) {
        return -1;
    }

    for (int i = 0; i < size; i++) {
        data[i] = b->buff[b->r];
        b->r = (b->r + 1) & (RBSIZE - 1);
    }

    return 0;
}
