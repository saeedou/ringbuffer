#include <cutest.h>
#include <clog.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


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
    struct u8buff q;
    
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


/// void
/// test_u8buffueue_isfull_isempty() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     char out[size];
///     u8buffueue_t b = u8buffueue_create(size);
///     int ufd = rand_open();
///     istrue(u8buffueue_isempty(b));
/// 
///     /* Provide some random data and put them */
///     read(ufd, in, size);
///     eqint(size - 1, u8buffueue_put(b, in, size));
///     eqint(4095, b->writer);
///     eqint(0, b->reader);
///     istrue(u8buffueue_isfull(b));
///     eqint(4095, u8buffueue_used(b));
/// 
///     /* Get all available data */
///     eqint(size - 1, u8buffueue_get(b, out, size));
///     istrue(u8buffueue_isempty(b));
///     eqint(4095, b->writer);
///     eqint(4095, b->reader);
///     eqint(4095, u8buffueue_available(b));
///     istrue(u8buffueue_isempty(b));
///     eqnstr(in, out, size - 1);
/// 
///     /* Teardown */
///     close(ufd);
///     u8buffueue_destroy(b);
/// }
/// 
/// 
/// void
/// test_u8buffueue_put() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     u8buffueue_t b = u8buffueue_create(size);
///     int ufd = rand_open();
/// 
///     /* Provide some random data and put them */
///     read(ufd, in, size);
///     eqint(-1, u8buffueue_put(b, in, size));
///     eqint(0, u8buffueue_put(b, in, size - 1));
/// 
///     /* Teardown */
///     close(ufd);
///     u8buffueue_destroy(b);
/// }
/// 
/// 
/// void
/// test_u8buffueue_put() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     char out[size];
///     u8buffueue_t b = u8buffueue_create(size);
/// 
///     /* Put 3 chars */
///     eqint(3, u8buffueue_put(b, "foo", 3));
///     eqint(3, b->writer);
/// 
///     /* Try to read at least 4 bytes */
///     eqint(-1, u8buffueue_getmin(b, out, 4, 10));
///     eqint(3, u8buffueue_getmin(b, out, 3, 10));
/// 
///     /* Teardown */
///     u8buffueue_destroy(b);
/// }
/// 
/// 
/// struct tfile {
///     FILE *file;
///     int fd;
/// };
/// 
/// 
/// static struct tfile
/// tmpfile_open() {
///     struct tfile t = {
///         .file = tmpfile(),
///     };
/// 
///     t.fd = fileno(t.file);
///     return t;
/// }
/// 
/// 
/// void
/// test_u8buffueue_readin_writeout() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     char out[size];
///     u8buffueue_t b = u8buffueue_create(size);
///     int ufd = rand_open();
///     struct tfile infile = tmpfile_open();
///     struct tfile outfile = tmpfile_open();
/// 
///     /* Provide some random data and put them */
///     read(ufd, in, size);
///     write(infile.fd, in, size);
///     lseek(infile.fd, 0, SEEK_SET);
/// 
///     /* Read some data from fd into the buffer */
///     eqint(size - 1, u8buffueue_readin(b, infile.fd, size));
///     eqint(size - 1, u8buffueue_used(b));
/// 
///     /* Write out */
///     eqint(size -1, u8buffueue_writeout(b, outfile.fd, size));
///     eqint(0, u8buffueue_used(b));
/// 
///     /* Compare */
///     lseek(outfile.fd, 0, SEEK_SET);
///     read(outfile.fd, out, size);
///     eqnstr(in, out, size);
/// 
///     /* Teardown */
///     close(ufd);
///     fclose(infile.file);
///     fclose(outfile.file);
///     u8buffueue_destroy(b);
/// }


int main() {
    test_u8buff_init_deinit();
    test_u8buff_put_pop();
    // test_u8buffueue_isfull_isempty();
    // test_u8buffueue_put();
    // test_u8buffueue_put();
    // test_u8buffueue_readin_writeout();
    return EXIT_SUCCESS;
}
