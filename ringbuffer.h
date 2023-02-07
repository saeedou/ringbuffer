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

int
rb_write(struct rb *b, char *char_arr, int arr_length);

char
rb_read(struct rb *rb);


#endif
