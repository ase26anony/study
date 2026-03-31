/* sel-sched-trigger.c
 * Designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling on this function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_scheduler_1(int* arr_a, int* arr_b, float* arr_c, 
                               int size, int threshold) {
    volatile int vol_size = size;  /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;  /* Outer loop carried state */
        
        for (int i = 0; i < vol_size; i++) {
            /* Create multiple dependency types */
            int val_a = arr_a[i];
            int val_b = arr_b[i];
            
            /* Flow dependency chain */
            sum = sum + val_a * val_b;
            
            /* Anti dependency: reuse same register */
            int temp = sum;
            
            /* Control dependency with unpredictable branch */
            if (sum > threshold) {
                arr_c[i] = (float)sum * 0.5f;
                sum = 0;  /* Reset creates output dependency */
            } else {
                arr_c[i] = (float)temp * 0.25f;
            }
            
            /* Floating point operations with different precision */
            fsum = fsum + arr_c[i];
            
            /* Inline assembly barrier to create scheduling boundaries */
            asm volatile("" ::: "memory");
            
            /* Manually unrolled iteration 1 */
            if (i + 1 < vol_size) {
                int val_a2 = arr_a[i + 1];
                int val_b2 = arr_b[i + 1];
                sum = sum + val_a2 * (val_b2 + base);  /* Use outer loop state */
                
                /* Complex conditional with bit operations */
                if ((val_a2 & 0x1) && (sum & 0x2)) {
                    arr_c[i + 1] = fsum;
                    fsum = fsum * 0.9f;
                }
                asm volatile("" ::: "memory");
            }
            
            /* Resource conflict: multiple FP operations */
            double dtemp = (double)fsum * 1.1;
            fsum = (float)(dtemp * 0.95);
        }
    }
}

/* Second test with volatile counters and more barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void test_selective_scheduler_2(double* arr_d, int* arr_idx, int size) {
    volatile int vol_iter = size;
    double acc[4] = {0.0, 0.0, 0.0, 0.0};  /* Multiple accumulators */
    
    /* Outer loop with carried state */
    for (int block = 0; block < 8; block++) {
        double block_factor = 1.0 + (block * 0.1);
        int start_idx = (block * 3) % size;
        
        /* Inner loop with complex addressing */
        for (int i = 0; i < vol_iter; i++) {
            int idx = (start_idx + i) % size;
            
            /* Four parallel dependency chains */
            acc[0] = acc[0] + arr_d[idx] * block_factor;
            asm volatile("" ::: "memory");
            
            acc[1] = acc[1] - arr_d[idx] * (block_factor * 0.5);
            asm volatile("" ::: "memory");
            
            /* Data-dependent array index */
            int dep_idx = arr_idx[idx] % 4;
            acc[dep_idx] = acc[dep_idx] * 1.01;
            
            /* Conditional with side effect */
            if (acc[0] > acc[1]) {
                arr_d[idx] = acc[0] - acc[1];
                acc[2] = acc[2] + 1.0;
            } else {
                arr_d[idx] = acc[1] - acc[0];
                acc[3] = acc[3] + 1.0;
            }
            
            /* More unrolling */
            if (i % 2 == 0 && i + 2 < vol_iter) {
                double tmp = arr_d[(idx + 1) % size];
                arr_d[(idx + 1) % size] = tmp * 0.99 + acc[i % 4];
                asm volatile("" ::: "memory");
            }
        }
    }
}

/* Third test: outer-loop carried state pattern */
void test_outer_loop_carried(int* arr, int size, int reps) {
    volatile int vol_reps = reps;
    
    for (int r = 0; r < vol_reps; r++) {
        int base = (r * 13 + 7) & 0x3F;  /* Varies with outer loop */
        int factor = 1 + (r % 5);
        
        /* Inner loop uses outer loop state */
        for (int i = 0; i < size; i++) {
            /* Multiple operations with dependencies */
            int old = arr[i];
            arr[i] = (old + base) * factor;
            
            /* Create loop-carried dependency */
            base = (base + (old & 0x1)) & 0x3F;
            
            /* Branch with data-dependent condition */
            if ((arr[i] & 0xF) == 0) {
                factor = factor ^ 0x1F;
                asm volatile("" ::: "memory");
            }
            
            /* Additional unrolled iteration */
            if (i % 3 == 0 && i + 1 < size) {
                arr[i + 1] = arr[i + 1] + (base >> 2);
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
    int array_idx[SIZE];
    int array_work[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000) - 500;
        array_b[i] = (int)(lcg_rand() % 1000) - 500;
        array_c[i] = (float)(lcg_rand() % 1000) / 10.0f;
        array_d[i] = (double)(lcg_rand() % 1000) / 5.0;
        array_idx[i] = lcg_rand() % SIZE;
        array_work[i] = i;
    }
    
    volatile int checksum_a = 0;
    volatile int checksum_b = 0;
    
    /* Runtime-variable control flow */
    for (int iter = 0; iter < 3; iter++) {
        /* Variable threshold based on iteration */
        int threshold = 1000 + (iter * 500);
        
        /* Call test functions conditionally */
        if (checksum_a < 1000000 || iter == 0) {
            test_selective_scheduler_1(array_a, array_b, array_c, SIZE, threshold);
        }
        
        if (checksum_b == 0 || (iter & 0x1)) {
            test_selective_scheduler_2(array_d, array_idx, SIZE / 2);
        }
        
        /* Update volatile checksums to prevent optimization */
        for (int i = 0; i < 10; i++) {
            checksum_a += array_a[i % SIZE];
            checksum_b += array_b[i % SIZE];
        }
    }
    
    /* Outer loop carried state test */
    test_outer_loop_carried(array_work, SIZE, 5);
    
    /* Final checksum computation to prevent dead code elimination */
    long long final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += array_a[i] + array_b[i] + (int)array_c[i] 
                        + (int)array_d[i] + array_work[i];
    }
    
    printf("Final checksum: %lld\n", final_checksum);
    printf("Volatile checksums: %d, %d\n", checksum_a, checksum_b);
    
    return 0;
}
