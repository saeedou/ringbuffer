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
#include <fcntl.h>
#include <cutest.h>
#include <clog.h>


typedef uint8_t u8;
#undef CRING_TYPE
#define CRING_TYPE u8

#include "cring.h"
#include "cring.c"


static int
rand_open() {
    int fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        return -1;
    }
    return fd;
}


void
test_u8buff_init_deinit() {
    u8buff_t q;

    eqint(0, u8buff_init(&q, 2));
    eqint(0, q.r);
    eqint(0, q.w);
    eqint(3, q.size);

    eqint(0, u8buff_deinit(&q));
    eqint(0, q.r);
    eqint(0, q.w);
    eqint(0, q.size);
}


void
test_u8buff_skip() {
    struct u8buff q;
    u8buff_init(&q, 8);
    size_t size = q.size;
    char out[size];

    /* Put 9 chars */
    eqint(0, u8buff_put(&q, "foobarbaz", 9));

    /* Skip 3 chars */
    eqint(0, u8buff_skip(&q, 3));

    /* Ger 3 chars from buffer */
    eqint(3, u8buff_pop(&q, out, 3));
    eqnstr("bar", out, 3);

    /* Skip another 3 chars */
    eqint(0, u8buff_skip(&q, 3));
    eqint(0, CRING_USED(&q));

    /* Skip when buffer is empty */
    eqint(-1, u8buff_skip(&q, 1));
}


void
test_u8buff_put_pop() {
    /* Setup */
    struct u8buff q;
    u8buff_init(&q, 8);
    size_t size = q.size;
    char in[size];
    char out[size];
    int ufd = rand_open();

    /* Put 3 chars */
    eqint(0, u8buff_put(&q, "foo", 3));
    eqint(3, q.w);
    eqint(3, CRING_USED(&q));
    eqint(size - 3, CRING_AVAILABLE(&q));

    /* Ger 3 chars from buffer */
    eqint(3, u8buff_pop(&q, out, 3));
    eqnstr("foo", out, 3);
    eqint(3, q.w);
    eqint(3, q.r);

    /* Provide some random data and put them */
    read(ufd, in, size);
    eqint(0, u8buff_put(&q, in, size - 1));
    eqint(1, q.w);
    eqint(3, q.r);

    /* Get all available data */
    eqint(size - 1, u8buff_pop(&q, out, size - 1));
    eqint(1, q.w);
    eqint(1, q.r);
    eqint(255, CRING_AVAILABLE(&q));
    istrue(CRING_ISEMPTY(&q));
    eqnstr(in, out, size - 1);

    /* Teardown */
    close(ufd);
    u8buff_deinit(&q);
}


void
test_u8buff_isfull_isempty() {
    /* Setup */
    struct u8buff q;
    u8buff_init(&q, 12);
    size_t size = q.size;
    char in[size];
    char out[size];
    int ufd = rand_open();
    istrue(CRING_ISEMPTY(&q));

    /* Provide some random data and put them */
    read(ufd, in, size);
    eqint(0, u8buff_put(&q, in, size));
    eqint(4095, q.w);
    eqint(0, q.r);
    istrue(CRING_ISFULL(&q));
    eqint(4095, CRING_USED(&q));

    /* Get all available data */
    eqint(size, u8buff_pop(&q, out, size));
    istrue(CRING_ISEMPTY(&q));
    eqint(4095, q.w);
    eqint(4095, q.r);
    eqint(4095, CRING_AVAILABLE(&q));
    istrue(CRING_ISEMPTY(&q));
    eqnstr(in, out, size - 1);

    /* Teardown */
    close(ufd);
    u8buff_deinit(&q);
}


void
test_u8buff_put() {
    /* Setup */
    struct u8buff q;
    u8buff_init(&q, 12);
    size_t size = q.size;
    char in[size];
    int ufd = rand_open();

    /* Provide some random data and put them */
    read(ufd, in, size + 1);
    eqint(-1, u8buff_put(&q, in, size + 1));
    eqint(0, u8buff_put(&q, in, size));

    /* Teardown */
    close(ufd);
    u8buff_deinit(&q);
}


struct tfile {
    FILE *file;
    int fd;
};


static struct tfile
tmpfile_open() {
    struct tfile t = {
        .file = tmpfile(),
    };

    t.fd = fileno(t.file);
    return t;
}


void
test_u8buff_readput_popwrite() {
    /* Setup */
    struct u8buff q;
    u8buff_init(&q, 12);
    size_t size = q.size;
    char in[size];
    char out[size];
    int ufd = rand_open();
    struct tfile infile = tmpfile_open();
    struct tfile outfile = tmpfile_open();
    size_t bytes = 0;

    /* Provide some random data and put them */
    read(ufd, in, size);
    write(infile.fd, in, size);
    lseek(infile.fd, 0, SEEK_SET);

    /* Read some data from fd into the buffer */
    isfalse(u8buff_readput(&q, infile.fd, &bytes) == CFS_ERROR);
    eqint(size, bytes);
    eqint(size, CRING_USED(&q));

    /* Write out */
    isfalse(u8buff_popwrite(&q, outfile.fd, &bytes) == CFS_ERROR);
    eqint(size, bytes);
    eqint(0, CRING_USED(&q));

    /* Compare */
    lseek(outfile.fd, 0, SEEK_SET);
    read(outfile.fd, out, size);
    eqnstr(in, out, size);

    /* Teardown */
    close(ufd);
    fclose(infile.file);
    fclose(outfile.file);
    u8buff_deinit(&q);
}


int main() {
    test_u8buff_init_deinit();
    test_u8buff_skip();
    test_u8buff_put_pop();
    test_u8buff_isfull_isempty();
    test_u8buff_put();
    test_u8buff_readput_popwrite();

    return EXIT_SUCCESS;
}
