/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
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
static void init_arrays(int32_t *a, int32_t *b, float *fa, double *da) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int32_t)(lcg_rand() % 1000) - 500;
        b[i] = (int32_t)(lcg_rand() % 1000) - 500;
        fa[i] = (float)(lcg_rand() % 1000) / 100.0f;
        da[i] = (double)(lcg_rand() % 1000) / 100.0;
    }
}

/* Function 1: Annotated to force selective scheduling with complex patterns */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_function_1(int32_t *a, int32_t *b, float *fa, double *da) {
    volatile int threshold = 1000;  /* Prevent constant propagation */
    int32_t sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Nested loops with data-dependent branches and mixed arithmetic */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int base = (j * 37) & 0xFF;  /* Outer loop carried state */
        
        /* Manually unrolled inner loop with multiple dependency types */
        for (int i = 0; i < INNER_LOOP; i += 4) {
            /* Flow dependency: sum depends on previous sum */
            sum = sum + a[i] * b[i];
            
            /* Anti dependency: a[i] read before write */
            if (sum > threshold) {
                a[i] = sum;
                sum = 0;
            }
            
            /* Output dependency: fsum written multiple times */
            fsum = fsum + fa[i] * 1.5f;
            if (fsum > 50.0f) {
                fsum = fsum * 0.5f;
                fa[i] = fsum;
            }
            
            /* Control dependency with volatile */
            volatile int flag = (sum & 1);
            if (flag) {
                dsum = dsum + da[i] * 2.0;
            }
            
            /* Second iteration of unrolled loop */
            sum = sum + a[i+1] * b[i+1];
            if (sum < -threshold) {
                b[i+1] = -sum;
                sum = sum / 2;
            }
            
            fsum = fsum + fa[i+1] * 2.0f;
            dsum = dsum + da[i+1] * (base / 100.0);  /* Use outer loop state */
            
            /* Third iteration with inline assembly barrier */
            sum = sum + a[i+2] * b[i+2];
            asm volatile("" ::: "memory");  /* Scheduling barrier */
            
            if ((a[i+2] ^ b[i+2]) & 1) {  /* Data-dependent branch */
                sum = sum ^ 0x5555;
            }
            
            /* Fourth iteration with mixed operations */
            sum = sum + a[i+3] * b[i+3];
            fsum = fsum - fa[i+3];
            dsum = dsum * 1.01 + da[i+3];
            
            /* Complex condition with multiple dependencies */
            if ((sum > 0 && fsum < 0) || (dsum > 100.0)) {
                a[i+3] = (int32_t)fsum;
                b[i+3] = (int32_t)dsum;
            }
        }
        
        /* Loop-carried dependency to next outer iteration */
        threshold = threshold + (base % 10);
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(sum), "+r"(fsum), "+r"(dsum) : : "memory");
}

/* Function 2: Uses volatile counters and assembly barriers extensively */
static void test_function_2(int32_t *a, int32_t *b) {
    volatile int counter = INNER_LOOP;  /* Volatile prevents optimization */
    int32_t acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    while (counter-- > 0) {
        int idx = INNER_LOOP - counter - 1;
        
        /* Multiple accumulators with artificial dependencies */
        acc1 = acc1 + a[idx];
        asm volatile("" ::: "memory");  /* Barrier 1 */
        
        acc2 = acc2 + b[idx];
        asm volatile("" ::: "memory");  /* Barrier 2 */
        
        /* Cross-dependency between accumulators */
        if (acc1 > acc2) {
            acc3 = acc3 + (acc1 - acc2);
        } else {
            acc4 = acc4 + (acc2 - acc1);
        }
        
        /* Data-dependent array update */
        if ((a[idx] & 3) == 0) {
            a[idx] = acc3;
            asm volatile("" ::: "memory");  /* Barrier 3 */
        } else if ((a[idx] & 3) == 1) {
            b[idx] = acc4;
            asm volatile("" ::: "memory");  /* Barrier 4 */
        }
        
        /* Complex chain of operations */
        acc1 = acc1 ^ (acc2 << 1);
        acc2 = acc2 ^ (acc1 >> 1);
        acc3 = acc3 * 3 - acc4;
        acc4 = acc4 * 5 - acc3;
    }
    
    /* Merge results with memory barrier */
    asm volatile("" ::: "memory");
    a[0] = acc1 + acc2 + acc3 + acc4;
}

/* Function 3: Outer-loop carried state pattern */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static void test_function_3(int32_t *arr, float *farr) {
    int state = 0;
    float fstate = 0.0f;
    
    for (int j = 0; j < OUTER_LOOP; j++) {
        /* Outer loop modifies state used in inner loop */
        int base = (state * j + 7919) & 0xFFF;  /* Prime number for variability */
        float factor = 1.0f + (j % 10) * 0.1f;
        
        /* Inner loop with dependencies on outer loop state */
        for (int i = 0; i < INNER_LOOP; i++) {
            /* Multiple interleaved operations */
            arr[i] = (arr[i] + base) * (i % 8 + 1);
            farr[i] = (farr[i] + fstate) * factor;
            
            /* Update states with feedback */
            state = state ^ arr[i];
            fstate = fstate + farr[i] * 0.5f;
            
            /* Conditional update based on complex expression */
            if ((arr[i] & (1 << (j % 5))) != 0) {
                farr[i] = fstate;
                state = state + i;
            }
        }
        
        /* Outer loop update with memory barrier */
        asm volatile("" ::: "memory");
        state = (state * 13 + 7) & 0xFFFF;
        fstate = fstate * 0.9f;
    }
}

/* Compute checksum to prevent dead code elimination */
static int32_t compute_checksum(int32_t *a, int32_t *b, float *fa, double *da) {
    int32_t checksum = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum = checksum ^ a[i];
        checksum = checksum + b[i];
        fchecksum = fchecksum + fa[i];
        checksum = checksum + (int32_t)(da[i] * 100);
    }
    
    checksum = checksum + (int32_t)fchecksum;
    return checksum;
}

int main(void) {
    /* Declare arrays with different types to increase RTL complexity */
    int32_t array_a[ARRAY_SIZE];
    int32_t array_b[ARRAY_SIZE];
    float array_fa[ARRAY_SIZE];
    double array_da[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    init_arrays(array_a, array_b, array_fa, array_da);
    
    /* Volatile flag to introduce runtime variability */
    volatile int run_alternate = array_a[0] > 0;
    
    /* Call test functions multiple times with runtime decision */
    for (int iteration = 0; iteration < 5; iteration++) {
        if (run_alternate) {
            /* Force selective scheduler activation */
            test_function_1(array_a, array_b, array_fa, array_da);
        } else {
            test_function_2(array_a, array_b);
        }
        
        /* Always run function 3 */
        test_function_3(array_a, array_fa);
        
        /* Toggle flag based on array content */
        run_alternate = (array_b[iteration % ARRAY_SIZE] & 1);
    }
    
    /* Additional repetitions to increase scheduling opportunities */
    for (int rep = 0; rep < 3; rep++) {
        test_function_1(array_a, array_b, array_fa, array_da);
        test_function_2(array_a, array_b);
    }
    
    /* Compute and print final checksum */
    int32_t final_checksum = compute_checksum(array_a, array_b, array_fa, array_da);
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
