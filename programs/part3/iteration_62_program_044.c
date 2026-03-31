/* sel-sched-trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with debug dumps for this function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_sched_1(int* arr_a, int* arr_b, float* arr_c, int size, int threshold) {
    volatile int vol_size = size;  /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;  /* Outer loop carried state */
        
        for (int i = 0; i < vol_size; i++) {
            /* Create multiple dependency types with manual unrolling */
            
            /* Flow dependency: sum depends on previous iteration */
            sum = sum + arr_a[i] * arr_b[i];
            
            /* Anti dependency: arr_a[i] read before write */
            arr_a[i] = (arr_a[i] + base) * 2;
            
            /* Output dependency: arr_b[i] written multiple times */
            arr_b[i] = arr_b[i] + (sum & 0xF);
            
            /* Control dependency with unpredictable branch */
            if (sum > threshold) {
                arr_c[i] = (float)sum * 0.5f;
                sum = sum / 2;  /* Modify carried state */
            } else {
                arr_c[i] = (float)arr_b[i] * 0.25f;
            }
            
            /* Floating-point operations create different RTL patterns */
            fsum = fsum + arr_c[i];
            
            /* Inline assembly barrier creates scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Second iteration of manual unrolling */
            if (i + 1 < vol_size) {
                int idx = i + 1;
                sum = sum + arr_a[idx] * arr_b[idx];
                arr_a[idx] = (arr_a[idx] + base) * 3;
                arr_b[idx] = arr_b[idx] + (sum & 0x7);
                
                if (sum > threshold * 2) {
                    arr_c[idx] = (float)sum * 0.3f;
                    sum = sum / 3;
                }
                
                fsum = fsum + arr_c[idx] * 2.0f;
                asm volatile("" ::: "memory");
            }
        }
        
        /* Modify threshold based on outer loop state */
        threshold = (threshold + base) & 0x3FF;
    }
    
    /* Prevent dead code elimination */
    arr_a[0] = (int)fsum;
}

/* Another function with volatile counters and complex patterns */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_sched_2(double* arr_d, int* arr_e, int size) {
    volatile int vol_counter = size;
    double dsum = 0.0;
    int carry = 0;
    
    /* Outer loop with carried state */
    for (int block = 0; block < 3; block++) {
        int block_base = (block * 13) ^ 0xAA;
        
        /* Inner loop with mixed integer/double operations */
        for (int i = 0; i < vol_counter; i++) {
            /* Multiple parallel computation chains */
            double temp1 = arr_d[i] * 1.5 + (double)block_base;
            double temp2 = arr_d[i] * 0.75 - (double)carry;
            
            /* Data-dependent conditional with side effects */
            if ((arr_e[i] & 1) == 0) {
                arr_d[i] = temp1 * temp2;
                carry = (carry + 1) & 0xF;
            } else {
                arr_d[i] = temp2 - temp1;
                carry = (carry - 1) & 0xF;
            }
            
            /* Complex expression with multiple operations */
            dsum = dsum + arr_d[i] * (double)((i & 7) + 1);
            
            /* More manual unrolling */
            if (i % 2 == 0 && i + 2 < vol_counter) {
                arr_e[i] = arr_e[i] ^ (arr_e[i + 1] + block_base);
                asm volatile("" ::: "memory");
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        vol_counter = vol_counter > 10 ? vol_counter - 1 : size;
    }
    
    /* Use result to prevent elimination */
    arr_e[0] = (int)(dsum * 100.0);
}

/* Function simulating outer-loop carried state pattern */
void test_outer_carried_state(int* arr, int size, int iterations) {
    volatile int vol_iter = iterations;
    
    for (int iter = 0; iter < vol_iter; iter++) {
        /* Compute base that depends on outer iteration */
        int base = (iter * 19 + 7) & 0xFF;
        float factor = 1.0f + (iter % 10) * 0.1f;
        
        for (int i = 0; i < size; i++) {
            /* Loop with outer-loop carried dependencies */
            int val = arr[i];
            
            /* Multiple operations creating complex RTL */
            val = (val + base) * (int)(factor * 10);
            val = val ^ (base << (i & 3));
            val = val + (iter & 0xF) * 256;
            
            /* Conditional store with side effect */
            if (val > 1000) {
                arr[i] = val % 1000;
                base = (base + 1) & 0xFF;  /* Modify outer state */
            } else {
                arr[i] = val;
            }
            
            /* Scheduling barrier every 8 iterations */
            if ((i & 7) == 0) {
                asm volatile("" ::: "memory");
            }
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d[SIZE];
    int array_e[SIZE];
    
    /* Initialize with pseudo-random but non-uniform data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
        array_d[i] = (double)(lcg_rand() % 1000) * 0.01;
        array_e[i] = (int)(lcg_rand() % 256);
    }
    
    volatile int checksum = 0;
    volatile int flag = 1;
    
    /* Call test functions multiple times with runtime variability */
    for (int run = 0; run < 5; run++) {
        if (flag) {
            test_selective_sched_1(array_a, array_b, array_c, SIZE, 5000);
        }
        
        /* Modify flag based on array content */
        flag = (array_a[0] + array_b[0]) & 1;
        
        test_selective_sched_2(array_d, array_e, SIZE / 2);
        
        if (run % 2 == 0) {
            test_outer_carried_state(array_a, SIZE, 3);
        }
    }
    
    /* Compute final checksum to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i] + array_b[i] + (int)array_c[i] 
                  + (int)array_d[i] + array_e[i];
        checksum &= 0xFFFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
