#ifndef RINGBUFFER_H
#define RINGBUFFER_H


#include <stddef.h>


struct rb {
    int w;
    int r;
    size_t size;
    char *buff;
};

int
rb_init(struct rb *rb, size_t size);

void
rb_deinit(struct rb *rb);

int
rb_available(struct rb *rb);

int
rb_used(struct rb *rb);


#endif
