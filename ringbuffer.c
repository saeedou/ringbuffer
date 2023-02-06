#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "ringbuffer.h"


int
rb_init(struct rb *b, size_t size) {
    b->r = 0;
    b->w = 0;
    b->size = size;
    b->buff = (char *) malloc(size);

    return 0;
}

void
rb_deinit(struct rb *b) {
    if (b->buff != NULL) {
        free(b->buff);
    }
}

int
rb_available(struct rb *b) {
    int available;

    if (b->w >= b->r) {
        available = b->size - (b->w - b->r);
        return available;
    } else if (b->w < b->r) {
        available = b->r - b->w;
        return available;
    }
}

int
rb_used(struct rb *b) {
    int used;
    used = b->size - rb_available(b);
    return used;
}

int
rb_write(struct rb *b, char c) {
    if (rb_available(b) > 0) {
        b->buff[b->w] = c;
        b->w++;
        return EXIT_SUCCESS;
    } else {
        printf("Ring buffer is full.");
        return EXIT_FAILURE;
    }
}
