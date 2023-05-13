#include <cutest.h>


// Undefine previous definitions.
#undef RBSIZE
#undef SNAME
#undef T


// Size of ringbuffer
#define RBSIZE 8


// Structure name
#define SNAME rb


// Type of buffer
#define T char


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

    eqint(0, cq_reset(&rb));

    eqint(0, rb.w);
    eqint(0, rb.r);
    eqint(0, strlen(rb.buff));
}


void
test_cq_available() {
    struct rb rb;
    rb.w = 6;
    rb.r = 3;
    eqint(4, cq_available(&rb));

    rb.w = 3;
    rb.r = 6;
    eqint(2, cq_available(&rb));

    rb.w = 6;
    rb.r = 6;
    eqint(7, cq_available(&rb));
}


void
test_cq_put() {
    struct rb rb;
    char towrite[6] = "abcdef";
    char cq_sample = 's';
    cq_reset(&rb);
    rb.w = 4;
    rb.r = 4;

    eqint(0, cq_put(&rb, towrite, sizeof(towrite)));
    eqnstr("ef", rb.buff, 2);
    eqnstr("abcd", rb.buff + 4, 4);
    eqint(2, rb.w);

    cq_put(&rb, &cq_sample, 1);

    eqint(-1, cq_put(&rb, &cq_sample, 1));
}


void
test_cq_pop() {
    struct rb rb;
    char towrite[6] = "abcdef";
    char toread[4];
    cq_reset(&rb);

    cq_put(&rb, towrite, sizeof(towrite));
    eqint(-1, cq_pop(&rb, toread, 7));
    eqint(0, cq_pop(&rb, toread, 4));
    eqint(4, rb.r);
    eqnstr("abcd", toread, 4);
}


int
main() {
    test_cq_reset();
    test_cq_available();
    test_cq_put();
    test_cq_pop();


    // When RBSIZE is not power of two
    #include "test_not_power_of_two.c"
    test_cq_reset();
    test_cq_available();
    test_cq_put();
    test_cq_pop();
 
    return EXIT_SUCCESS;
}
