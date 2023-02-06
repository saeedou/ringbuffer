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

int main() {
    test_rb_init();
    return EXIT_SUCCESS;
}
