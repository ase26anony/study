/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_LOOPS 8
#define INNER_LOOPS 128
#define UNROLL_FACTOR 4

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loops with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_complex_schedule(int *arr_a, int *arr_b, float *arr_c, volatile int *trigger) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    int sum_i = 0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOPS; j++) {
        int base = (j * 7919) & 0xFF;  /* Prime multiplier for variability */
        float threshold = (j % 3 == 0) ? 100.0f : 50.0f;
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < INNER_LOOPS; i += UNROLL_FACTOR) {
            /* Manually unrolled with different operations */
            for (int u = 0; u < UNROLL_FACTOR; u++) {
                int idx = i + u;
                if (idx >= INNER_LOOPS) break;
                
                /* Multiple dependency types */
                int val_a = arr_a[idx];
                int val_b = arr_b[idx];
                
                /* Flow dependency on sum_i */
                sum_i = sum_i + val_a * base;
                
                /* Anti dependency through arr_b modification */
                arr_b[idx] = (val_b ^ base) + sum_i;
                
                /* Control dependency with floating point */
                if (sum_i > (base * 100)) {
                    sum_f = sum_f + (float)val_a * 1.5f;
                    /* Memory barrier to complicate scheduling */
                    asm volatile("" ::: "memory");
                    
                    if (sum_f > threshold) {
                        arr_c[idx] = sum_f;
                        sum_f = sum_f * 0.5f;
                    }
                }
                
                /* Output dependency through multiple assignments */
                sum_d = sum_d + (double)val_a * (double)val_b;
                if ((val_a & 1) && (val_b & 2)) {
                    sum_d = sum_d * 1.1;
                }
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
            }
            
            /* Cross-iteration dependency */
            if (*trigger > 0) {
                sum_i = sum_i >> 1;
                *trigger = *trigger - 1;
            }
        }
        
        /* Loop-carried dependency to next outer iteration */
        arr_a[j % ARRAY_SIZE] = sum_i & 0xFFFF;
    }
    
    /* Final store to prevent elimination */
    arr_b[0] = sum_i;
    arr_c[0] = sum_f;
}

/* Function 2: Volatile counters with assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(int *arr, volatile int *counter) {
    volatile int v_limit = *counter;
    float acc[4] = {0.1f, 0.2f, 0.3f, 0.4f};
    
    for (volatile int v = 0; v < v_limit; v++) {
        int idx = v % (ARRAY_SIZE - 4);
        
        /* Mixed FP operations with explicit barriers */
        acc[0] = acc[0] + (float)arr[idx] * 1.23f;
        asm volatile("" ::: "memory");
        
        acc[1] = acc[1] - (float)arr[idx + 1] * 0.87f;
        if (acc[0] > acc[1]) {
            acc[2] = acc[2] * 1.05f;
        }
        asm volatile("" ::: "memory");
        
        acc[3] = (acc[0] + acc[1]) / (acc[2] + 0.001f);
        
        /* Data-dependent store */
        if ((arr[idx] & 3) == 0) {
            arr[idx] = (int)(acc[0] + acc[1] + acc[2] + acc[3]);
        }
        
        /* Complex integer computation */
        int temp = arr[idx] ^ arr[idx + 1];
        temp = temp * 1103515245 + 12345;
        arr[idx + 2] = temp & 0x7FFF;
        
        /* Another barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Store results */
    for (int i = 0; i < 4; i++) {
        arr[i] += (int)acc[i];
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_outer_carried_state(int *arr, int outer_iters, int inner_iters) {
    int carried_state = 0;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Compute base from outer loop state */
        int base = (carried_state * 63689 + outer * 378551) & 0xFFF;
        float factor = 1.0f + (outer % 10) * 0.1f;
        
        /* Inner loop with carried state dependency */
        for (int inner = 0; inner < inner_iters; inner++) {
            int idx = (outer * inner_iters + inner) % ARRAY_SIZE;
            
            /* Complex transformation with carried state */
            int old_val = arr[idx];
            arr[idx] = (old_val + base) * factor;
            
            /* Update carried state based on result */
            if (arr[idx] > 1000000) {
                carried_state = carried_state ^ (old_val >> 4);
                arr[idx] = arr[idx] % 1000000;
            }
            
            /* Additional computation to increase ILP */
            float f_val = (float)arr[idx] * 0.001f;
            if (f_val > 500.0f) {
                arr[(idx + 1) % ARRAY_SIZE] += (int)(f_val * 0.5f);
            }
        }
        
        /* Outer-loop update */
        carried_state = (carried_state + base) & 0xFF;
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
    }
    
    /* Volatile trigger to prevent optimization */
    volatile int trigger = lcg_rand() % 100 + 50;
    volatile int counter = lcg_rand() % 80 + 20;
    volatile int outer_iters = 5;
    volatile int inner_iters = 150;
    
    /* Variable control flow to introduce runtime decisions */
    volatile int checksum_a = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum_a ^= array_a[i];
    }
    
    /* Conditional repeated execution based on checksum */
    if (checksum_a & 1) {
        for (int rep = 0; rep < 10; rep++) {
            test_complex_schedule(array_a, array_b, array_c, &trigger);
        }
    }
    
    if (checksum_a & 2) {
        for (int rep = 0; rep < 8; rep++) {
            test_volatile_barriers(array_b, &counter);
        }
    }
    
    if (checksum_a & 4) {
        for (int rep = 0; rep < 6; rep++) {
            test_outer_carried_state(array_a, outer_iters, inner_iters);
        }
    }
    
    /* Additional mixed execution */
    for (int mix = 0; mix < 3; mix++) {
        test_complex_schedule(array_a, array_b, array_c, &trigger);
        test_volatile_barriers(array_b, &counter);
        test_outer_carried_state(array_a, 3, 100);
    }
    
    /* Final checksum computation to prevent dead code elimination */
    int final_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array_a[i];
        final_checksum ^= array_b[i];
        final_checksum += (int)array_c[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    return 0;
}
