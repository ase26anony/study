/* sel-sched-trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = (lcg_state * 1103515245 + 12345) & 0x7fffffff;
    return lcg_state;
}

/* Function 1: Complex nested loops with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_complex_schedule(int *a, int *b, float *c, double *d, int n, int threshold) {
    volatile int vol_n = n;  /* Prevent constant propagation */
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 17) & 0xFF;
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < vol_n; i++) {
            /* Multiple dependency types */
            int idx = (i + base) % n;
            
            /* Flow dependency: sum_int depends on previous iteration */
            sum_int += a[idx] * b[idx];
            
            /* Anti dependency: a[idx] read before write */
            a[idx] = (a[idx] + base) & 0xFFFF;
            
            /* Output dependency: b[idx] written multiple times */
            b[idx] = sum_int >> 3;
            
            /* Control dependency with unpredictable branch */
            if (sum_int > threshold) {
                /* Mixed floating-point operations */
                sum_float += c[idx] * 1.5f;
                c[idx] = sum_float;
                sum_int = sum_int >> 1;  /* Reset partially */
            }
            
            /* More arithmetic with different types */
            sum_double += d[idx] * 0.75;
            d[idx] = sum_double / (i + 1);
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < vol_n) {
                int idx2 = (i + 1 + base) % n;
                sum_int -= b[idx2];
                a[idx2] = (a[idx2] * 3) & 0xFFFF;
                
                /* Inline assembly barrier - creates scheduling boundary */
                asm volatile("" ::: "memory");
                
                if (a[idx2] & 1) {  /* Data-dependent branch */
                    sum_float -= c[idx2] * 0.5f;
                }
            }
        }
    }
    
    /* Prevent dead code elimination */
    a[0] = sum_int;
    c[0] = sum_float;
    d[0] = sum_double;
}

/* Function 2: Volatile counters with assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(int *arr, float *farr, int size) {
    volatile int vol_i, vol_j;
    int temp1 = 0, temp2 = 0;
    float ftemp1 = 0.0f, ftemp2 = 0.0f;
    
    /* Nested loops with volatile counters */
    for (vol_j = 0; vol_j < 8; vol_j++) {
        int outer_state = vol_j * 13;
        
        for (vol_i = 0; vol_i < size; vol_i++) {
            /* Multiple parallel computations */
            int idx = (vol_i + outer_state) % size;
            
            /* Independent chains with resource conflicts */
            temp1 = arr[idx] * 3 + temp1;
            temp2 = arr[idx] / 2 + temp2;
            ftemp1 = farr[idx] * 1.7f + ftemp1;
            ftemp2 = farr[idx] / 1.3f + ftemp2;
            
            /* Assembly barrier every 4 iterations */
            if ((vol_i & 3) == 0) {
                asm volatile("" ::: "memory");
            }
            
            /* Conditional store with anti-dependency */
            if (temp1 > temp2) {
                arr[idx] = temp1 - temp2;
                farr[idx] = ftemp1 - ftemp2;
            } else {
                arr[idx] = temp2 - temp1;
                farr[idx] = ftemp2 - ftemp1;
            }
            
            /* More unrolled operations */
            int idx2 = (idx + 1) % size;
            temp1 ^= arr[idx2];
            temp2 |= arr[idx2];
            ftemp1 += farr[idx2] * 0.3f;
            ftemp2 -= farr[idx2] * 0.7f;
        }
    }
    
    /* Ensure results are used */
    arr[0] = temp1 + temp2;
    farr[0] = ftemp1 + ftemp2;
}

/* Function 3: Outer-loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_outer_carried_state(int *data, int n) {
    int state = 0;
    volatile int vol_limit = n;
    
    /* Outer loop modifies state used in inner loop */
    for (int outer = 0; outer < 16; outer++) {
        /* Complex state calculation */
        state = (state * 31 + outer * 7) & 0x3FF;
        int factor = (state >> 2) + 1;
        
        /* Inner loop with state-dependent computation */
        for (int i = 0; i < vol_limit; i++) {
            /* Loop-carried dependency through state */
            int val = data[i];
            
            /* Multiple operations with different latencies */
            val = (val + state) * factor;
            val = val ^ (val >> 4);
            val = val * 3 - factor;
            
            /* Data-dependent array access */
            int dep_idx = val % n;
            if (dep_idx >= 0 && dep_idx < n) {
                data[i] = val + data[dep_idx];
            }
            
            /* Periodic barrier */
            if ((i & 7) == 0) {
                asm volatile("" ::: "memory");
            }
            
            /* Update state for next iteration */
            state = (state + val) & 0x3FF;
        }
        
        /* Additional computation between outer iterations */
        factor = (factor * 2) & 0xFF;
    }
    
    data[0] = state;
}

int main(void) {
    const int SIZE = 1024;
    int array1[SIZE], array2[SIZE];
    float farray[SIZE];
    double darray[SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)(lcg_rand() & 0xFFFF);
        array2[i] = (int)(lcg_rand() & 0xFFFF);
        farray[i] = (float)(lcg_rand() / 65536.0f);
        darray[i] = (double)(lcg_rand() / 65536.0);
    }
    
    volatile int checksum = 0;
    volatile int flag = 1;
    
    /* Runtime-variable execution pattern */
    if (flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_complex_schedule(array1, array2, farray, darray, SIZE, 10000);
        }
    }
    
    /* Compute intermediate checksum */
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i] ^ array2[i];
    }
    
    /* Conditional execution based on checksum */
    if (checksum & 1) {
        for (int rep = 0; rep < 3; rep++) {
            test_volatile_barriers(array1, farray, SIZE);
        }
    } else {
        test_outer_carried_state(array2, SIZE);
    }
    
    /* More iterations with different parameters */
    for (int iter = 0; iter < 2; iter++) {
        test_complex_schedule(array1, array2, farray, darray, SIZE, 
                             (iter * 5000) + 5000);
    }
    
    /* Final checksum computation */
    uint64_t final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += array1[i];
        final_checksum += array2[i];
        final_checksum += (uint64_t)(farray[i] * 1000);
        final_checksum += (uint64_t)(darray[i] * 1000);
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)final_checksum);
    return 0;
}
