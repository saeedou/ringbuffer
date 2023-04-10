#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ringbuffer.h"


void
MKFNAME(reset) (struct rb *b) {
    b->r = 0;
    b->w = 0;
    memset(b->buff, 0, RBSIZE);
}


int
MKFNAME(available) (struct rb *b) {
   int available;

   if (b->w >= b->r) {
       available = (RBSIZE - (b->w - b->r)) - 1;
       return available;
   }
   available = b->r - b->w - 1;
   return available;
}


int
MKFNAME(put) (struct rb *b, T *date, int size) {
   if (MKFNAME(available)(b) == 0 || MKFNAME(available)(b) < size) {
       return -1;
   }

   for (int i = 0; i < size; i++) {
       b->buff[b->w] = date[i];
       b->w++;
   }
   return 0;
}


int
MKFNAME(pop)(struct rb *b, T *date, int size) {
    int used;
    int buffindex;

    used = RBSIZE - MKFNAME(available)(b) - 1;
    if (size > used) {
        return -1;
    }

    for (int i = 0; i < size; i++) {
        buffindex = (b->r + i) % RBSIZE;
        date[i] = b->buff[buffindex];
    }
    b->r = (b->r + size) % RBSIZE;
    return 0;
}
