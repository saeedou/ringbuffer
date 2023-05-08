#ifndef RINGBUFFER_H
#define RINGBUFFER_H


#include <stddef.h>


struct SNAME {
    int w;
    int r;
    T buff[RBSIZE];
};


int
MKFNAME(reset) (struct rb *rb);


int
MKFNAME(available) (struct rb *rb);


int
MKFNAME(put) (struct rb *b, T *data, int size);


int
MKFNAME(pop) (struct rb *b, T *data, int size);


int
is_power_of_two();


#endif
