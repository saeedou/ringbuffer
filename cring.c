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


/** Read from fd into the buffer utill EAGAIN.
 * count argument contains the count of inserted items.
 * Return Value: 
 *   On success, the number of bytes read is returned.
 *   Zero indicates end of file.
 *   On error, -1 is returned, and errno is set appropriately.  
 */
ssize_t
CRING_NAME(readput) (CRING_T() *q, int fd) {
    int avail;
    int toend;
    ssize_t firstbytes;
    ssize_t secondbytes;

    if (CRING_ISFULL(q)) {
        /* Buffer is full */
        errno = ENOBUFS;
        return -1;
    }

    toend = CRING_FREE_TOEND(q);
    firstbytes = read(fd, q->buffer + q->w, toend);
    if (firstbytes == 0) {
        /* EOF */
        return 0;
    }

    if (firstbytes < 0) {
        /* error */
        return -1;
    }

    q->w = CRING_WRITER_CALC(q, firstbytes);;
    avail = CRING_AVAILABLE(q);
    if (avail == 0) {
        /* Buffer is full */
        return firstbytes;
    }

    secondbytes = read(fd, q->buffer + q->w, avail);
    if (secondbytes == 0) {
        /* EOF */
        return firstbytes;
    }

    if (secondbytes < 0) {
        if (EVMUSTWAIT()) {
            /* Must wait */
            errno = 0;
            return firstbytes;
        }
        /* error */
        return -1;
    }

    q->w = CRING_WRITER_CALC(q, secondbytes);
    return firstbytes + secondbytes;
}


/** Write ro fd from the buffer utill EAGAIN.
 * count argument contains the count of inserted items.
 * Return Value: 
 *   On success, the number of items write is returned.
 *   Zero indicates end of file.
 *   On error, -1 is returned, and errno is set appropriately.  
 */
ssize_t
CRING_NAME(popwrite) (CRING_T() *q, int fd) {
    int used;
    int toend;
    ssize_t firstbytes;
    ssize_t secondbytes;

    if (CRING_ISEMPTY(q)) {
        /* Buffer is empty */
        errno = ENOBUFS;
        return -1;
    }

    toend = CRING_USED_TOEND(q);
    firstbytes = write(fd, q->buffer + q->r, toend);
    if (firstbytes == 0) {
        /* EOF */
        return 0;
    }

    if (firstbytes < 0) {
        /* error */
        return -1;
    }

    q->r = CRING_READER_CALC(q, firstbytes);;
    used = CRING_USED(q);
    if (used == 0) {
        /* Buffer is empty */
        return firstbytes;
    }

    secondbytes = write(fd, q->buffer + q->r, used);
    if (secondbytes == 0) {
        /* EOF */
        return firstbytes;
    }

    if (secondbytes < 0) {
        if (EVMUSTWAIT()) {
            /* Must wait */
            errno = 0;
            return firstbytes;
        }
        /* error */
        return -1;
    }

    q->w = CRING_READER_CALC(q, secondbytes);
    return firstbytes + secondbytes;
}
