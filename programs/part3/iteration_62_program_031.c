/* sel-sched-trigger.c
 * Program to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_LOOP 8
#define INNER_LOOP 128

/* Simple LCG pseudo-random generator */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *a, int32_t *b, float *fa, double *db) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int32_t)(lcg_rand() % 1000) - 500;
        b[i] = (int32_t)(lcg_rand() % 1000) - 500;
        fa[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
        db[i] = (double)(lcg_rand() % 1000) / 100.0 - 5.0;
    }
}

/* Test function 1: Complex nested loops with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_function_1(int32_t *a, int32_t *b, float *fa, double *db) {
    volatile int volatile_flag = 1; /* Prevent constant propagation */
    int32_t sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP && volatile_flag; j++) {
        int base = (j * 37) & 0xFF; /* Computation with outer loop index */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (j * INNER_LOOP + i) % ARRAY_SIZE;
            
            /* Mixed arithmetic creating flow dependencies */
            sum_int += a[idx] * b[idx];
            sum_float += fa[idx] * 2.5f;
            sum_double += db[idx] * 1.5;
            
            /* Data-dependent conditional branch - unpredictable pattern */
            if (sum_int & 0x1) { /* Check LSB */
                /* Create anti-dependency by reusing variables */
                a[idx] = sum_int >> 1;
                b[idx] = (sum_int * 3) & 0xFFF;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
            }
            
            /* Another conditional with floating point */
            if (sum_float > 100.0f) {
                fa[idx] = sum_float / 2.0f;
                sum_float = sum_float - 50.0f;
                
                /* Manual unrolling creates more scheduling opportunities */
                if (i + 1 < INNER_LOOP) {
                    int idx2 = (j * INNER_LOOP + i + 1) % ARRAY_SIZE;
                    sum_double += db[idx2] * base; /* Use outer loop carried state */
                    db[idx2] = sum_double * 0.9;
                }
            }
            
            /* Output dependency creation */
            sum_int = sum_int + base;
        }
        
        /* Loop-carried dependency across outer iterations */
        sum_int = (sum_int * 11) & 0xFFFF;
    }
    
    /* Prevent dead code elimination */
    a[0] = sum_int;
    fa[0] = sum_float;
    db[0] = sum_double;
}

/* Test function 2: Volatile counters and inline assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void test_function_2(int32_t *a, int32_t *b) {
    volatile int volatile_counter = INNER_LOOP / 2;
    int32_t acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Loop with volatile condition and scheduling barriers */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Multiple accumulators for instruction-level parallelism */
        acc1 = acc1 + a[i] * 3;
        acc2 = acc2 + b[i] * 7;
        acc3 = acc3 + (a[i] - b[i]);
        
        /* Inline assembly creates scheduling barriers */
        asm volatile("" : "+r"(acc1), "+r"(acc2) : : "memory");
        
        /* Data-dependent operation with volatile */
        if (i > volatile_counter) {
            a[i] = acc1 ^ acc2;
            b[i] = acc3 & 0xFF;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Small unrolled section */
            if (i + 2 < ARRAY_SIZE) {
                a[i+1] = (acc1 + acc2) >> 1;
                b[i+1] = (acc3 * 2) & 0xFF;
                i++; /* Manual advance */
            }
        }
        
        /* Complex condition with multiple dependencies */
        if ((acc1 > 1000) && (acc2 < 5000) && (acc3 & 0x1)) {
            volatile_counter = volatile_counter + 1;
        }
    }
    
    /* Cross-iteration dependency */
    for (int i = 1; i < ARRAY_SIZE; i++) {
        a[i] = a[i] + a[i-1]; /* Flow dependency */
        b[i] = b[i] ^ b[i-1];
    }
}

/* Test function 3: Outer loop carried state pattern */
static void test_function_3(int32_t *arr, float *farr) {
    int32_t outer_state = 0;
    float float_state = 1.0f;
    
    for (int j = 0; j < OUTER_LOOP * 2; j++) {
        /* Compute base from outer loop state */
        int base = (outer_state * 13 + j * 17) & 0xFF;
        float fbase = float_state * 0.75f;
        
        /* Inner loop using outer loop carried state */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (j * INNER_LOOP + i) % ARRAY_SIZE;
            
            /* Mixed operations with different latencies */
            arr[idx] = (arr[idx] + base) * ((i & 0x1) ? 3 : 5);
            farr[idx] = (farr[idx] + fbase) * ((i & 0x2) ? 1.2f : 0.8f);
            
            /* Create control dependency */
            if (arr[idx] > 10000) {
                farr[idx] = farr[idx] / 2.0f;
                base = base + 1; /* Modify outer state within inner loop */
            }
            
            /* Potential resource conflict: multiple FP operations */
            float_state = float_state * 1.01f;
            farr[idx] = farr[idx] + float_state;
        }
        
        /* Update outer loop carried state */
        outer_state = outer_state + base;
        float_state = float_state * 0.99f;
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
}

/* Compute checksum to prevent dead code elimination */
static int32_t compute_checksum(int32_t *a, int32_t *b, float *fa, double *db) {
    int32_t checksum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum = (checksum + a[i]) ^ b[i];
        fsum += fa[i];
        
        /* Mix in double values */
        if (i % 4 == 0) {
            checksum += (int32_t)(db[i] * 100.0);
        }
    }
    
    checksum += (int32_t)fsum;
    return checksum;
}

int main(void) {
    /* Allocate and initialize arrays */
    int32_t array_a[ARRAY_SIZE];
    int32_t array_b[ARRAY_SIZE];
    float array_fa[ARRAY_SIZE];
    double array_db[ARRAY_SIZE];
    
    init_arrays(array_a, array_b, array_fa, array_db);
    
    /* Volatile flag for runtime control flow variability */
    volatile int run_test_1 = 1;
    volatile int run_test_2 = 1;
    volatile int run_test_3 = 1;
    
    /* Main test sequence with conditional repetition */
    int32_t total_checksum = 0;
    
    /* Run test 1 multiple times if flag is set */
    if (run_test_1) {
        for (int rep = 0; rep < 5; rep++) {
            test_function_1(array_a, array_b, array_fa, array_db);
            
            /* Modify volatile flag based on array content */
            run_test_1 = (array_a[0] & 0x1) ? 1 : 0;
        }
    }
    
    /* Run test 2 with volatile control */
    if (run_test_2) {
        for (int rep = 0; rep < 3; rep++) {
            test_function_2(array_a, array_b);
            run_test_2 = (array_b[ARRAY_SIZE/2] > 0) ? 1 : 0;
        }
    }
    
    /* Always run test 3 at least once */
    test_function_3(array_a, array_fa);
    
    /* Additional runs based on checksum */
    int32_t interim_checksum = compute_checksum(array_a, array_b, array_fa, array_db);
    if (run_test_3 && (interim_checksum & 0x1)) {
        test_function_3(array_b, array_fa); /* Swap arrays for different pattern */
    }
    
    /* Final checksum computation and output */
    total_checksum = compute_checksum(array_a, array_b, array_fa, array_db);
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Array samples: a[0]=%d, b[0]=%d, fa[0]=%.2f, db[0]=%.2f\n",
           array_a[0], array_b[0], array_fa[0], array_db[0]);
    
    return 0;
}
