/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */

#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_complex_schedule(int* restrict arr_a, int* restrict arr_b, 
                          float* restrict arr_f, double* restrict arr_d, 
                          int size, int threshold) {
    volatile int vol_size = size;  /* Prevent constant propagation */
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < 3; outer++) {
        int base = (outer * 17) & 0xFF;
        float f_base = (float)base * 0.1f;
        double d_base = (double)base * 0.01;
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < vol_size; i++) {
            /* Create multiple dependency types */
            int temp1 = arr_a[i] + base;
            float temp2 = arr_f[i] * f_base;
            double temp3 = arr_d[i] + d_base;
            
            /* Flow dependency chain */
            sum_int = sum_int + temp1;
            sum_float = sum_float + temp2;
            sum_double = sum_double + temp3;
            
            /* Data-dependent conditional branch - unpredictable */
            if (sum_int & 0x1) {  /* Check LSB */
                /* Anti-dependency: read then write same location */
                int old_val = arr_b[i];
                arr_b[i] = old_val + sum_int;
                
                /* Output dependency in floating point */
                arr_f[i] = sum_float;
                asm volatile("" ::: "memory");  /* Scheduling barrier */
            }
            
            /* Another conditional with different check */
            if ((sum_int > threshold) && (i & 0x3)) {
                /* Complex operation mix */
                arr_d[i] = sum_double * 0.5;
                sum_int = sum_int / 2;  /* Modify carried variable */
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling - creates more ILP opportunities */
            if (i + 1 < vol_size) {
                int temp4 = arr_a[i+1] * base;
                float temp5 = arr_f[i+1] / (f_base + 1.0f);
                sum_int += temp4;
                sum_float += temp5;
                
                /* Another unpredictable branch */
                if (temp4 & 0x2) {
                    arr_b[i+1] = temp4 ^ base;
                }
            }
        }
    }
    
    /* Store final sums to prevent elimination */
    arr_a[0] = sum_int;
    arr_f[0] = sum_float;
    arr_d[0] = sum_double;
}

/* Second test with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
void test_volatile_barriers(int* arr, int size) {
    volatile int vol_i, vol_j;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Nested loops with volatile counters */
    for (vol_j = 0; vol_j < 4; vol_j++) {
        int mod = (vol_j * 13 + 7) & 0xF;
        
        for (vol_i = 0; vol_i < size; vol_i++) {
            /* Multiple accumulators with different operations */
            acc1 = acc1 + arr[vol_i] * mod;
            acc2 = acc2 ^ (arr[vol_i] + mod);
            acc3 = acc3 | (arr[vol_i] - mod);
            
            /* Frequent assembly barriers create scheduling challenges */
            asm volatile("" ::: "memory");
            
            /* Data-dependent store */
            if (acc1 > acc2) {
                arr[vol_i] = acc3;
                asm volatile("" ::: "memory");
            }
            
            /* More complex dependency web */
            int temp = arr[vol_i] * 2;
            acc1 = acc1 - temp;
            acc2 = acc2 + (temp >> 3);
            
            /* Conditional with side effect */
            if ((vol_i & 0x7) == 0) {
                acc3 = arr[vol_i] ^ acc3;
                asm volatile("" ::: "memory");
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        arr[0] = acc1 + acc2 + acc3;
    }
}

/* Third test: outer-loop carried state pattern */
void test_outer_carried_state(float* arr, int size, int iterations) {
    float state = 1.0f;
    volatile int vol_iter = iterations;
    
    for (int iter = 0; iter < vol_iter; iter++) {
        /* State modified in outer loop, used in inner */
        state = state * 1.1f + (float)iter;
        if (state > 100.0f) state = 1.0f;
        
        float factor = state * 0.01f;
        int limit = size - (iter % 8);
        
        /* Inner loop with state-dependent computation */
        for (int i = 0; i < limit; i++) {
            /* Flow dependency on 'state' from outer loop */
            arr[i] = (arr[i] + state) * factor;
            
            /* Anti-dependency chain */
            float old = arr[i];
            arr[i] = old * old + factor;
            
            /* Control dependency */
            if (arr[i] > 50.0f) {
                factor = factor * 0.9f;
                asm volatile("" ::: "memory");
            }
            
            /* Partial unrolling */
            if (i + 2 < limit) {
                arr[i+1] = arr[i+1] * factor + state;
                arr[i+2] = arr[i+2] / factor - state;
            }
        }
        
        /* Modify state based on inner loop results */
        state = arr[0] * 0.5f;
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    float array_f[SIZE];
    double array_d[SIZE];
    
    /* Initialize with pseudo-random but non-uniform data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_f[i] = (float)(lcg_rand() % 1000) * 0.01f;
        array_d[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
    
    volatile int checksum = 0;
    volatile int flag = array_a[0] & 1;  /* Runtime-dependent flag */
    
    /* Call test functions multiple times with runtime variability */
    for (int repeat = 0; repeat < 5; repeat++) {
        if (flag || (repeat & 1)) {
            test_complex_schedule(array_a, array_b, array_f, array_d, 
                                 SIZE, 500 + repeat * 100);
        }
        
        test_volatile_barriers(array_b, SIZE - (repeat * 32));
        
        if (!flag || (repeat & 2)) {
            test_outer_carried_state(array_f, SIZE, 3 + (repeat % 3));
        }
        
        /* Modify flag based on array contents */
        flag = (array_a[repeat % SIZE] + array_b[repeat % SIZE]) & 1;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i] + (int)array_f[i] + (int)array_d[i];
        checksum ^= array_b[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
