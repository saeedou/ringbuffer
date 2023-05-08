// Undefine previous definitions.
#undef RBSIZE
#undef SNAME
#undef T


// Size of ringbuffer
#define RBSIZE 9


// Structure name
#define SNAME rb


// Type of buffer
#define T char


// undef previous functions
#undef MKFNAME


// Make funcion name
#define MKFNAME(name) cq_ ## name


#include "ringbuffer.h"
#include "ringbuffer.c"


void
test_cq_reset() {
    struct rb rb;
    rb.w = 4;
    rb.r = 4;
    memset(rb.buff, 0, RBSIZE);
    strcpy(rb.buff, "hello");

    // Error
    eqint(-1, cq_reset(&rb));
}


void
test_cq_available() {
    struct rb rb;
    rb.w = 6;
    rb.r = 3;

    // Error
    eqint(-1, cq_available(&rb));
}
 
 
void
test_cq_put() {
    struct rb rb;
    rb.w = 4;
    rb.r = 4;
    memset(rb.buff, 0, RBSIZE);
    char towrite[6] = "abcdef";

    // Error
    eqint(-1, cq_put(&rb, towrite, sizeof(towrite)));
}
 
 
void
test_cq_pop() {
    struct rb rb;
    rb.w = 4;
    rb.r = 4;
    memset(rb.buff, 'a', RBSIZE);
    char toread[4];

    // Error
    eqint(-1, cq_pop(&rb, toread, 4));
}
