#include "cutest.h"
#include "ringbuffer.h"


void
test_rb_init() {
    struct rb rb;
    eqint(0, rb_init(&rb, 8));
    eqint(0, rb.r);
    eqint(0, rb.w);
    eqint(8, rb.size);
    rb_deinit(&rb);
}


//void
//test_deinit(){
//}


void
test_rb_available() {
    struct rb rb;
    rb_init(&rb, 8);
    rb.w = 6;
    rb.r = 3;
    eqint(5, rb_available(&rb));

    rb.w = 3;
    rb.r = 6;
    eqint(3, rb_available(&rb));

    rb.w = 6;
    rb.r = 6;
    eqint(8, rb_available(&rb));
    rb_deinit(&rb);
}


void
test_rb_used() {
    struct rb rb;
    rb_init(&rb, 8);
    rb.w = 6;
    rb.r = 3;
    eqint(3, rb_used(&rb));

    rb.w = 3;
    rb.r = 6;
    eqint(5, rb_used(&rb));

    rb.w = 6;
    rb.r = 6;
    eqint(0, rb_used(&rb));
    rb_deinit(&rb);
}


void
test_rb_write() {
    struct rb rb;
    char arr[2] = {'a', 'b'};
    char char_sample = 's';

    rb_init(&rb, 4);

    eqint(0, rb_write(&rb, arr, 2));
    eqchr('a', rb.buff[0]);
    eqint(2, rb.w);

    rb_write(&rb, &char_sample, 1);
    rb_write(&rb, &char_sample, 1);

    eqint(-1, rb_write(&rb, &char_sample, 1));
    rb_deinit(&rb);
}


void
test_rb_read() {
    struct rb rb;
    char char_sample = 'a';
    rb_init(&rb, 2);
    isnull(rb_read(&rb));

    rb_write(&rb, &char_sample, 1);
    eqchr('a', rb_read(&rb));

}


int main() {
    test_rb_init();
    test_rb_available();
    test_rb_used();
    test_rb_write();
    test_rb_read();
    return EXIT_SUCCESS;
}
