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

void test_rb_available() {
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

int main() {
    test_rb_init();
    test_rb_available();
    return EXIT_SUCCESS;
}
