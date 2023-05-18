/* Generic stuff */
#define CRING_PASTER(x, y) x ## y
#define CRING_EVALUATOR(x, y) CRING_PASTER(x, y)
#define CRING_STRUCT() CRING_EVALUATOR(CRING_TYPE, queue)
#define CRING_T() CRING_EVALUATOR(CRING_TYPE, queue_t)

#define CRING_NAME_PASTER(x, y) x ## _ ## y
#define CRING_NAME_EVALUATOR(x, y) CRING_NAME_PASTER(x, y)
#define CRING_NAME(n) CRING_NAME_EVALUATOR(CRING_STRUCT(), n)


typedef struct CRING_STRUCT() {
    uint8_t bits;

    /* Read/Tail*/
    int r;

    /* Write/Head */
    int w;
    
    CRING_TYPE *buffer;
} CRING_T();
