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
test_u8queue_init_deinit() {
    struct u8queue q;
    
    eqint(0, u8queue_init(&q, 2));
    eqint(0, q.r);
    eqint(0, q.w);
    eqint(3, q.size);

    eqint(0, u8queue_deinit(&q));
    eqint(0, q.r);
    eqint(0, q.w);
    eqint(0, q.size);
}


void
test_u8queue_enqueue_get() {
    /* Setup */
    struct u8queue q;
    u8queue_init(&q, 8);
    size_t size = q.size;
    char in[size];
    char out[size];
    int ufd = rand_open();

    /* Put 3 chars */
    eqint(0, u8queue_enqueue(&q, "foo", 3));
    eqint(3, q.w);
    eqint(3, CRING_USED(&q));
    eqint(size - 3, CRING_AVAILABLE(&q));

    // /* Ger 3 chars from buffer */
    // eqint(3, u8queue_get(b, out, 3));
    // eqnstr("foo", out, 3);
    // eqint(3, b->w);
    // eqint(3, b->r);

    // /* Provide some random data and put them */
    // read(ufd, in, size);
    // eqint(size - 2, u8queue_enqueue(b, in, size - 2));
    // eqint(1, b->w);
    // eqint(3, b->r);

    // /* Get all available data */
    // eqint(size - 2, u8queue_get(b, out, size - 2));
    // eqint(1, b->w);
    // eqint(1, b->r);
    // eqint(255, u8queue_available(b));
    // istrue(u8queue_isempty(b));
    // eqnstr(in, out, size - 2);

    /* Teardown */
    close(ufd);
    u8queue_deinit(&q);
}


/// void
/// test_u8queue_isfull_isempty() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     char out[size];
///     u8queue_t b = u8queue_create(size);
///     int ufd = rand_open();
///     istrue(u8queue_isempty(b));
/// 
///     /* Provide some random data and put them */
///     read(ufd, in, size);
///     eqint(size - 1, u8queue_enqueue(b, in, size));
///     eqint(4095, b->writer);
///     eqint(0, b->reader);
///     istrue(u8queue_isfull(b));
///     eqint(4095, u8queue_used(b));
/// 
///     /* Get all available data */
///     eqint(size - 1, u8queue_get(b, out, size));
///     istrue(u8queue_isempty(b));
///     eqint(4095, b->writer);
///     eqint(4095, b->reader);
///     eqint(4095, u8queue_available(b));
///     istrue(u8queue_isempty(b));
///     eqnstr(in, out, size - 1);
/// 
///     /* Teardown */
///     close(ufd);
///     u8queue_destroy(b);
/// }
/// 
/// 
/// void
/// test_u8queue_enqueueall() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     u8queue_t b = u8queue_create(size);
///     int ufd = rand_open();
/// 
///     /* Provide some random data and put them */
///     read(ufd, in, size);
///     eqint(-1, u8queue_enqueueall(b, in, size));
///     eqint(0, u8queue_enqueueall(b, in, size - 1));
/// 
///     /* Teardown */
///     close(ufd);
///     u8queue_destroy(b);
/// }
/// 
/// 
/// void
/// test_u8queue_enqueue_getmin() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     char out[size];
///     u8queue_t b = u8queue_create(size);
/// 
///     /* Put 3 chars */
///     eqint(3, u8queue_enqueue(b, "foo", 3));
///     eqint(3, b->writer);
/// 
///     /* Try to read at least 4 bytes */
///     eqint(-1, u8queue_getmin(b, out, 4, 10));
///     eqint(3, u8queue_getmin(b, out, 3, 10));
/// 
///     /* Teardown */
///     u8queue_destroy(b);
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
/// test_u8queue_readin_writeout() {
///     /* Setup */
///     size_t size = getpagesize();
///     char in[size];
///     char out[size];
///     u8queue_t b = u8queue_create(size);
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
///     eqint(size - 1, u8queue_readin(b, infile.fd, size));
///     eqint(size - 1, u8queue_used(b));
/// 
///     /* Write out */
///     eqint(size -1, u8queue_writeout(b, outfile.fd, size));
///     eqint(0, u8queue_used(b));
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
///     u8queue_destroy(b);
/// }


int main() {
    test_u8queue_init_deinit();
    test_u8queue_enqueue_get();
    // test_u8queue_isfull_isempty();
    // test_u8queue_enqueueall();
    // test_u8queue_enqueue_getmin();
    // test_u8queue_readin_writeout();
    return EXIT_SUCCESS;
}
