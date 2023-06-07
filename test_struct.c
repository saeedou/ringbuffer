#include <cutest.h>
#include <clog.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>


typedef struct foo {
    int a;
    char b;
} foo;

#undef CRING_TYPE
#define CRING_TYPE foo

#include "cring.h"
#include "cring.c"


void
test_foobuff_init_deinit() {
    struct foobuff q;
    
    eqint(0, foobuff_init(&q, 2));
    eqint(0, q.r);
    eqint(0, q.w);
    eqint(3, q.size);

    eqint(0, foobuff_deinit(&q));
    eqint(0, q.r);
    eqint(0, q.w);
    eqint(0, q.size);
    isnull(q.buffer);
}


void
test_foobuff_put_pop() {
    /* Setup */
    struct foobuff q;
    foobuff_init(&q, 3);
    size_t size = q.size;
    foo in[size];
    foo out[size];

    struct foo bar;
    bar.a = 1;
    bar.b = 'a';

    struct foo baz;
    baz.a = 2;
    baz.b = 'b';
    
    struct foo qux;
    qux.a = 3;
    qux.b = 'c';

    struct foo data[] = {bar, baz, qux};

    /* Put 3 struct sdata */
    eqint(0, foobuff_put(&q, data, 3));
    eqint(3, q.w);
    eqint(3, CRING_USED(&q));
    eqint(size - 3, CRING_AVAILABLE(&q));

    /* Get 3 struct from buffer */
    eqint(3, foobuff_pop(&q, out, 3));
    eqint(3, out[2].a);
    eqchr('c', out[2].b);
    eqint(3, q.w);
    eqint(3, q.r);

    /* Populate the buffer */
    int i;
    
    for (i = 0; i < 7; i++) {
        foobuff_put(&q, &bar, 1);
    }

    eqint(2, q.w);
    eqint(3, q.r);

    /* Get all available data */
    eqint(size, foobuff_pop(&q, out, size));
    eqint(2, q.w);
    eqint(2, q.r);
    eqint(7, CRING_AVAILABLE(&q));
    istrue(CRING_ISEMPTY(&q));

    /* Teardown */
    foobuff_deinit(&q);
}
 
 
void
test_foobuff_isfull_isempty() {
    /* Setup */
    struct foobuff q;
    foobuff_init(&q, 3);
    size_t size = q.size;
    struct foo out[size];

    struct foo bar;
    bar.a = 1;
    bar.b = 'a';

    istrue(CRING_ISEMPTY(&q));
 
    /* Fill buffer */
    int i;

    for (i = 0; i < 7; i++) {
        eqint(0, foobuff_put(&q, &bar, 1));
    }

    eqint(7, q.w);
    eqint(0, q.r);
    istrue(CRING_ISFULL(&q));
    eqint(7, CRING_USED(&q));

    /* Get all available data */
    eqint(size, foobuff_pop(&q, out, size));
    istrue(CRING_ISEMPTY(&q));
    eqint(7, q.w);
    eqint(7, q.r);
    eqint(7, CRING_AVAILABLE(&q));

    /* Teardown */
    foobuff_deinit(&q);
}


void
test_foobuff_put() {
    /* Setup */
    struct foobuff q;
    foobuff_init(&q, 3);
    size_t size = q.size;
    struct foo in[size];

    /* Provide some data and put them */
    eqint(-1, foobuff_put(&q, in, size + 1));
    eqint(0, foobuff_put(&q, in, size));

    /* Teardown */
    foobuff_deinit(&q);
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
test_foobuff_readput_popwrite() {
    /* Setup */
    struct foobuff q;
    foobuff_init(&q, 3);
    size_t size = q.size;
    struct foo in[size];
    struct foo out[size];
    struct tfile infile = tmpfile_open();
    struct tfile outfile = tmpfile_open();
    size_t bytes;
    int i;

    struct foo bar = {1, 'c'};
    struct foo baz = {2, 'd'};
    struct foo qux = {3, 'e'};
    struct foo quux = {4, 'f'};
    struct foo corge = {5, 'g'};
    struct foo grault = {6, 'h'};
    struct foo graply = {7, 'i'};
    struct foo all[7] = {bar, baz, qux, quux, corge, grault, graply};

    /* Empty buffer to file */
    istrue(foobuff_popwrite(&q, outfile.fd, NULL) == CFS_BUFFEREMPTY);
    istrue(foobuff_readput(&q, outfile.fd, NULL) == CFS_EOF);

    /* One chunk data */
    foobuff_put(&q, all, 5);
    istrue(foobuff_popwrite(&q, outfile.fd, &bytes) == CFS_OK);
    eqint(5, bytes);

    /* Two chunk data */
    foobuff_reset(&q);
    q.w = 6;
    q.r = 6;
    foobuff_put(&q, all, 4);
    istrue(foobuff_popwrite(&q, outfile.fd, &bytes) == CFS_OK);
    eqint(4, bytes);

    foobuff_reset(&q);
    foobuff_put(&q, all, 7);
    istrue(foobuff_readput(&q, infile.fd, &bytes) == CFS_BUFFERFULL);

    foobuff_popwrite(&q, infile.fd, NULL);
    lseek(infile.fd, 0, SEEK_SET);
    foobuff_reset(&q);
    istrue(foobuff_readput(&q, infile.fd, &bytes) == CFS_BUFFERFULL);
    eqint(5, q.buffer[4].a);
    eqint(7, bytes);

    lseek(infile.fd, 0, SEEK_SET);
    ftruncate(infile.fd, 0);
    foobuff_reset(&q);
    foobuff_put(&q, all, 3);
    foobuff_popwrite(&q, infile.fd, NULL);
    lseek(infile.fd, 0, SEEK_SET);
    istrue(foobuff_readput(&q, infile.fd, &bytes) == CFS_OK);
    eqint(3, bytes);
    eqint(3, q.buffer[2].a);

    lseek(infile.fd, 0, SEEK_SET);
    ftruncate(infile.fd, 0);
    foobuff_reset(&q);
    foobuff_put(&q, all, 4);
    foobuff_popwrite(&q, infile.fd, NULL);
    lseek(infile.fd, 0, SEEK_SET);
    istrue(foobuff_readput(&q, infile.fd, &bytes) == CFS_EOF);

    lseek(infile.fd, 0, SEEK_SET);
    ftruncate(infile.fd, 0);
    foobuff_reset(&q);
    foobuff_put(&q, all, 5);
    foobuff_popwrite(&q, infile.fd, NULL);
    foobuff_put(&q, all, 2);
    lseek(infile.fd, 0, SEEK_SET);
    istrue(foobuff_readput(&q, infile.fd, &bytes) == CFS_OK);
    eqint(4, q.w);

    /* Teardown */
    foobuff_deinit(&q);
}


int main() {
    test_foobuff_init_deinit();
    test_foobuff_put_pop();
    test_foobuff_isfull_isempty();
    test_foobuff_put();
    test_foobuff_readput_popwrite();

    return EXIT_SUCCESS;
}
