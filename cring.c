#include <stdlib.h>


size_t
CRING_NAME(size) (CRING_T() *q) {
    size_t s = 0;
    uint8_t bits;
    
    for (bits = q->bits; bits > 0; bits--) {
        s <<= 1;
        s |= 1;
    }

    return s;
}


int
CRING_NAME(init) (CRING_T() *q, uint8_t bits) {
    size_t size;

    q->bits = bits;
    size = CRING_NAME(size) (q);

    q->buffer = malloc(size + 1);
    if (q->buffer == NULL) {
        return -1;
    }

    q->r = 0;
    q->w = 0;
    return 0;
}


int
CRING_NAME(deinit) (CRING_T() *q) {
    if (q == NULL) {
        return -1;
    }

    q->bits = 0;
    q->r = 0;
    q->w = 0;
    if (q->buffer != NULL) {
        free(q->buffer);
        q->buffer = NULL;
    }
}
