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
#ifndef CRING_COMMON  // NOLINT(build/header_guard)
#define CRING_COMMON


#include <unistd.h>


#ifndef CRING_COMMON_NAME
#define CRING_COMMON_NAME   buff
#endif


/* Generic stuff */
#define CRING_PASTER(x, y) x ## y
#define CRING_EVALUATOR(x, y) CRING_PASTER(x, y)
#define CRING_STRUCT() CRING_EVALUATOR(CRING_TYPE, CRING_COMMON_NAME)
#define CRING_T() \
    CRING_EVALUATOR(CRING_TYPE, CRING_EVALUATOR(CRING_COMMON_NAME, _t))

#define CRING_NAME_PASTER(x, y) x ## _ ## y
#define CRING_NAME_EVALUATOR(x, y) CRING_NAME_PASTER(x, y)
#define CRING_NAME(n) CRING_NAME_EVALUATOR(CRING_STRUCT(), n)


#define CRING_CALC(b, n)         ((n) & (b)->size)
#define CRING_USED(b)            CRING_CALC(b, (b)->w - (b)->r)
#define CRING_AVAILABLE(b)       CRING_CALC(b, (b)->r - ((b)->w + 1))
#define CRING_WRITER_CALC(b, n)  CRING_CALC(b, (b)->w + (n))
#define CRING_READER_CALC(b, n)  CRING_CALC(b, (b)->r + (n))
#define CRING_READER_SKIP(b, n)  (b)->reader = CRING_READER_CALC((b), (n))
#define CRING_RESET(b) ({ \
    (b)->writecounter = 0; \
    (b)->reader = 0; \
    (b)->writer = 0; \
})
#define CRING_ISEMPTY(b)          (CRING_USED(b) == 0)
#define CRING_ISFULL(b)           (CRING_AVAILABLE(b) == 0)


#define CRING_USED_TOEND(b) \
    ({int end = ((b)->size + 1) - (b)->r; \
      int n = ((b)->w + end) & (b)->size; \
      n < end ? n : end;})


/*
01234567
  w r
*/
#define CRING_FREE_TOEND(b) \
    ({int end = (b)->size - (b)->w; \
      int n = (end + (b)->r) & (b)->size; \
      n <= end ? n : end + 1;})


#define CRING_BYTES(n) ((n) * sizeof(CRING_TYPE))
#define CRING_MIN(x, y) (((x) > (y))? (y): (x))


#define EVMUSTWAIT() ((errno == EAGAIN) || (errno == EWOULDBLOCK) \
        || (errno == EINPROGRESS))


static size_t
_calcsize(unsigned char bits) {
    size_t s = 0;

    for (; bits > 0; bits--) {
        s = (s << 1) | 1;
    }

    return s;
}


enum cring_filestatus {
    CFS_OK,
    CFS_EOF,
    CFS_ERROR,
    CFS_AGAIN,
    CFS_BUFFERFULL,
    CFS_BUFFEREMPTY,
};


#endif


typedef struct CRING_STRUCT() {
    size_t size;

    /* Read/Tail*/
    int r;

    /* Write/Head */
    int w;

    CRING_TYPE *buffer;
} CRING_T();


int
CRING_NAME(init) (CRING_T() *q, unsigned char bits);


int
CRING_NAME(deinit) (CRING_T() *q);


int
CRING_NAME(put) (CRING_T() *q, const CRING_TYPE *data, size_t count);


ssize_t
CRING_NAME(pop) (CRING_T() *q, CRING_TYPE *data, size_t count);


int
CRING_NAME(skip) (CRING_T() *q, size_t count);


ssize_t
CRING_NAME(popuntil) (CRING_T() *q, CRING_TYPE c, CRING_TYPE *data,
        size_t count);


enum cring_filestatus
CRING_NAME(readput) (CRING_T() *q, int fd, size_t *count);


enum cring_filestatus
CRING_NAME(popwrite) (CRING_T() *q, int fd, size_t *count);


int
CRING_NAME(reset) (CRING_T() *q);
