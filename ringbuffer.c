#include <stddef.h>
#include <stdlib.h>

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
