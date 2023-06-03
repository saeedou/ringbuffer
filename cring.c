#include <stdlib.h>
#include <errno.h>
#include <unistd.h>


int
CRING_NAME(init) (CRING_T() *q, unsigned char bits) {
    q->size = _calcsize(bits);
    q->buffer = malloc(sizeof(CRING_TYPE) * (q->size + 1));
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
CRING_NAME(reset) (CRING_T() *q) {
    q->r = 0;
    q->w = 0;
    return 0;
}


#include <clog.h>
int
CRING_NAME(put) (CRING_T() *q, const CRING_TYPE *data, size_t count) {
    if (CRING_AVAILABLE(q) < count) {
        return -1;
    }
    
    size_t toend = CRING_FREE_TOEND(q);
    size_t chunklen = CRING_MIN(toend, count);
    DEBUG("Chunklen: %d %p %p", CRING_BYTES(chunklen), q->buffer, q->buffer + q->w);
    memcpy(q->buffer + q->w, data, CRING_BYTES(chunklen));
    q->w = CRING_WRITER_CALC(q, chunklen);

    if (count > chunklen) {
        count -= chunklen;
        DEBUG("Chunklen: %d %p %p", CRING_BYTES(count), q->buffer, q->buffer + q->w);
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


ssize_t
CRING_NAME(popuntil) (CRING_T() *q, CRING_TYPE c, CRING_TYPE *data, 
        size_t count) {
    size_t topop = CRING_MIN(count, CRING_USED(q));
    size_t r = 0;

    while (r < topop) {
        if (memcmp(&(q->buffer[CRING_READER_CALC(q, r)]), &c, 
                    sizeof(CRING_TYPE)) == 0) {
            return CRING_NAME(pop)(q, data, r+1);
        }

        r++;
    }

    return -1;
}


enum cring_filestatus
CRING_NAME(readput) (CRING_T() *q, int fd, size_t *count) {
    int avail;
    int toend;
    ssize_t firstbytes;
    ssize_t secondbytes;

    if (CRING_ISFULL(q)) {
        /* Buffer is full */
        return CFS_BUFFERFULL;
    }

    toend = CRING_FREE_TOEND(q);
    firstbytes = read(fd, q->buffer + q->w, toend);
    if (firstbytes == 0) {
        /* EOF */
        return CFS_EOF;
    }

    if (firstbytes < 0) {
        if (EVMUSTWAIT()) {
            /* Must wait */
            return CFS_AGAIN;
        }
        /* error */
        return CFS_ERROR;
    }
    
    if (count) {
        *count = firstbytes;
    }
    q->w = CRING_WRITER_CALC(q, firstbytes);;
    avail = CRING_AVAILABLE(q);
    if (avail == 0) {
        /* Buffer is full */
        return CFS_BUFFERFULL;
    }

    if (firstbytes < toend) {
        return CFS_OK;
    }
    secondbytes = read(fd, q->buffer + q->w, avail);
    if (secondbytes == 0) {
        /* EOF */
        return CFS_EOF;
    }

    if (secondbytes < 0) {
        if (EVMUSTWAIT()) {
            /* Must wait */
            return CFS_AGAIN;
        }
        /* error */
        return CFS_ERROR;
    }

    if (count) {
        *count = firstbytes + secondbytes;
    }
    q->w = CRING_WRITER_CALC(q, secondbytes);
    return CFS_OK;
}


enum cring_filestatus
CRING_NAME(popwrite) (CRING_T() *q, int fd, size_t *count) {
    int used;
    int toend;
    ssize_t firstbytes;
    ssize_t secondbytes;

    if (CRING_ISEMPTY(q)) {
        /* Buffer is empty */
        return CFS_BUFFEREMPTY;
    }

    toend = CRING_USED_TOEND(q);
    firstbytes = write(fd, q->buffer + q->r, toend);
    if (firstbytes == 0) {
        /* EOF */
        return CFS_EOF;;
    }

    if (firstbytes < 0) {
        if (EVMUSTWAIT()) {
            /* Must wait */
            return CFS_AGAIN;
        }
        /* error */
        return CFS_ERROR;;
    }

    if (count) {
        *count = firstbytes;
    }

    q->r = CRING_READER_CALC(q, firstbytes);;
    used = CRING_USED(q);
    if (used == 0) {
        /* Buffer is empty */
        return CFS_BUFFEREMPTY;
    }

    if (firstbytes < toend) {
        return CFS_OK;
    }

    secondbytes = write(fd, q->buffer + q->r, used);
    if (secondbytes == 0) {
        /* EOF */
        return CFS_EOF;;
    }

    if (secondbytes < 0) {
        if (EVMUSTWAIT()) {
            /* Must wait */
            return CFS_AGAIN;
        }
        /* error */
        return CFS_ERROR;;
    }

    if (count) {
        *count = firstbytes + secondbytes;
    }
    q->w = CRING_READER_CALC(q, secondbytes);
    return CFS_OK;
}
