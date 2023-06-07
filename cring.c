// Copyright 2023 Vahid Mardani
/*
 * This file is part of Carrow.
 *  Carrow is free software: you can redistribute it and/or modify it under 
 *  the terms of the GNU General Public License as published by the Free 
 *  Software Foundation, either version 3 of the License, or (at your option) 
 *  any later version.
 *  
 *  Carrow is distributed in the hope that it will be useful, but WITHOUT ANY 
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 *  details.
 *  
 *  You should have received a copy of the GNU General Public License along 
 *  with Carrow. If not, see <https://www.gnu.org/licenses/>. 
 *  
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
#include <stdlib.h>  // NOLINT(build/include)
#include <errno.h>
#include <unistd.h>
#include <string.h>


int
CRING_NAME(init) (CRING_T() *q, unsigned char bits) {
    q->size = _calcsize(bits);
    q->buffer = malloc(CRING_BYTES(q->size + 1));
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


int
CRING_NAME(put) (CRING_T() *q, const CRING_TYPE *data, size_t count) {
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


int
CRING_NAME(skip) (CRING_T() *q, size_t count) {
    if (count == 0) {
        return 0;
    }

    if (CRING_USED(q) < count) {
        return -1;
    }

    q->r = CRING_READER_CALC(q, count);
    return 0;
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
    firstbytes = read(fd, q->buffer + q->w, CRING_BYTES(toend));
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
        *count = firstbytes / CRING_BYTES(1);
    }
    q->w = CRING_WRITER_CALC(q, firstbytes / CRING_BYTES(1));
    avail = CRING_AVAILABLE(q);
    if (avail == 0) {
        /* Buffer is full */
        return CFS_BUFFERFULL;
    }

    if (firstbytes / CRING_BYTES(1) < toend) {
        return CFS_OK;
    }
    secondbytes = read(fd, q->buffer + q->w, CRING_BYTES(avail));
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
        *count = (firstbytes + secondbytes) / CRING_BYTES(1);
    }
    q->w = CRING_WRITER_CALC(q, secondbytes / CRING_BYTES(1));
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
    firstbytes = write(fd, q->buffer + q->r, CRING_BYTES(toend));
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
        *count = firstbytes / CRING_BYTES(1);
    }

    q->r = CRING_READER_CALC(q, firstbytes / CRING_BYTES(1));
    used = CRING_USED(q);
    if (used == 0) {
        /* Buffer is empty */
        return CFS_OK;
    }

    if (firstbytes < toend) {
        return CFS_AGAIN;
    }

    secondbytes = write(fd, q->buffer + q->r, CRING_BYTES(used));
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
        *count = (firstbytes + secondbytes) / CRING_BYTES(1);
    }
    q->w = CRING_READER_CALC(q, secondbytes / CRING_BYTES(1));
    return CFS_OK;
}
