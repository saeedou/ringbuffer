#include <stdlib.h>


int
CRING_NAME(init) (CRING_T() *q, uint8_t bits) {
    q->size = _calcsize(bits);
    q->buffer = malloc(q->size + 1);
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

    q->size = 0;
    q->r = 0;
    q->w = 0;
    if (q->buffer != NULL) {
        free(q->buffer);
        q->buffer = NULL;
    }

    return 0;
}


int
CRING_NAME(put) (CRING_T() *q, CRING_TYPE *data, size_t count) {
    if (CRING_AVAILABLE(q) < count) {
        return -1;
    }
    
    size_t toend = CRING_FREE_TOEND(q);
    size_t chunklen = CRING_MIN(toend, count);
    memcpy(q->buffer + q->w, data, CRING_BYTES(chunklen));
    q->w = CRING_WRITER_CALC(q, chunklen);

    if (count > chunklen) {
        count -= chunklen;
        memcpy(q->buffer, data + chunklen, CRING_BYTES(count));
        q->w += count;
    }

    return 0;
}


ssize_t
CRING_NAME(pop) (CRING_T() *q, CRING_TYPE *data, size_t count) {
    if (CRING_USED(q) < count) {
        return -1;
    }

    size_t toend = CRING_USED_TOEND(q);
    ssize_t total = CRING_MIN(toend, count);

    memcpy(data, q->buffer + q->r, CRING_BYTES(total));
    q->r = CRING_READER_CALC(q, total);
    count -= total;

    if (count) {
        count = CRING_MIN(CRING_USED_TOEND(q), count);
        memcpy(data + total, q->buffer, CRING_BYTES(count));
        q->r += count;
        total += count;
    }
   
    return total;
}
