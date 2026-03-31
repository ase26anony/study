/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling on this function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_sched_1(int* arr_a, int* arr_b, float* arr_c, int size, int threshold) {
    volatile int vol_size = size; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 17) & 0xFF; /* Simple computation for base */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < vol_size; i++) {
            /* Multiple dependency types */
            int temp = arr_a[i] + base;
            arr_a[i] = temp * 3;
            
            /* Flow dependency */
            sum += arr_b[i];
            
            /* Anti dependency */
            int old_b = arr_b[i];
            arr_b[i] = temp ^ old_b;
            
            /* Output dependency + control dependency */
            if (sum > threshold) {
                arr_c[i] = fsum + (float)sum * 0.5f;
                sum = sum / 2; /* Reset partially */
            }
            
            /* Mixed floating point operations */
            fsum = fsum + arr_c[i] * 1.5f;
            
            /* Inline assembly barrier to create scheduling complexity */
            asm volatile("" ::: "memory");
            
            /* Data-dependent branch with unpredictable pattern */
            if (arr_a[i] & 0x1) {
                fsum = fsum - (float)(arr_b[i] & 0xF) * 0.25f;
                asm volatile("" ::: "memory"); /* Another barrier */
            }
        }
        
        /* Manual unrolling for more scheduling opportunities */
        for (int i = 0; i < vol_size && i+1 < size; i += 2) {
            /* Unrolled operations with different data types */
            double dtemp = (double)arr_a[i] * 0.33;
            arr_c[i] = (float)dtemp + arr_c[i+1];
            
            int tmp1 = arr_b[i] * arr_a[i+1];
            int tmp2 = arr_b[i+1] * arr_a[i];
            arr_b[i] = tmp1 ^ tmp2;
            arr_b[i+1] = tmp1 & tmp2;
            
            /* Complex conditional */
            if ((arr_a[i] + arr_a[i+1]) > (threshold * 2)) {
                arr_c[i] = arr_c[i] * 2.0f;
                asm volatile("" ::: "memory");
            }
        }
    }
}

/* Second test with volatile counters and more barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_sched_2(double* arr_d, int* arr_idx, int size) {
    volatile int vol_start = 0;
    volatile int vol_end = size;
    
    double acc[4] = {0.0, 0.0, 0.0, 0.0};
    
    /* Nested loops with outer-loop carried state */
    for (int phase = vol_start; phase < 3; phase++) {
        int phase_factor = (phase * 7 + 3) & 0xF;
        
        for (int i = 0; i < vol_end; i++) {
            /* Multiple accumulators for ILP */
            acc[0] += (double)arr_idx[i] * 0.1;
            acc[1] += acc[0] * 0.5;
            
            /* Resource conflict simulation */
            arr_d[i] = arr_d[i] + acc[1] - acc[0];
            
            /* Memory barrier between dependent operations */
            asm volatile("" ::: "memory");
            
            acc[2] = acc[2] * 0.9 + (double)(arr_idx[i] & phase_factor);
            acc[3] = acc[0] + acc[1] + acc[2];
            
            /* Data-dependent array access */
            int idx = arr_idx[i] % size;
            arr_d[idx] = arr_d[idx] + acc[3];
            
            /* Unpredictable branch */
            if ((arr_idx[i] + phase) & 0x3) {
                acc[0] = acc[0] * 0.8;
                asm volatile("" ::: "memory");
            } else {
                acc[1] = acc[1] * 1.2;
            }
        }
        
        /* Small fully unrolled inner loop */
        for (int i = 0; i < vol_end && i+3 < size; i += 4) {
            /* 4-way unrolled with different operations */
            arr_d[i]   = arr_d[i]   * 1.01 + (double)phase;
            arr_d[i+1] = arr_d[i+1] * 0.99 - (double)phase_factor;
            arr_d[i+2] = arr_d[i+2] + arr_d[i] - arr_d[i+1];
            arr_d[i+3] = arr_d[i+3] * arr_d[i+2];
            
            /* Barrier in unrolled loop */
            asm volatile("" ::: "memory");
            
            arr_idx[i]   = arr_idx[i]   ^ (int)arr_d[i];
            arr_idx[i+1] = arr_idx[i+1] ^ (int)arr_d[i+1];
            arr_idx[i+2] = arr_idx[i+2] + (int)arr_d[i+2];
            arr_idx[i+3] = arr_idx[i+3] - (int)arr_d[i+3];
        }
    }
}

/* Third test with irregular access patterns */
void test_selective_sched_3(int* arr, float* farr, int size, int iter) {
    volatile int vol_iter = iter;
    
    for (int t = 0; t < vol_iter; t++) {
        int stride = (t % 7) + 1; /* Varying stride */
        
        /* Loop with non-unit stride and mixed operations */
        for (int i = 0; i < size; i += stride) {
            /* Polynomial calculation with dependencies */
            int x = arr[i];
            int x2 = x * x;
            int x3 = x2 * x;
            arr[i] = (x3 + 2*x2 + 3*x + 4) & 0xFFF;
            
            /* Floating point chain */
            float fx = (float)x * 0.1f;
            float fy = fx * fx + 1.0f;
            farr[i] = farr[i] * 0.9f + fy * 0.1f;
            
            /* Conditional with side effects */
            if (arr[i] > 1000) {
                farr[i] = farr[i] * 2.0f;
                arr[i] = arr[i] >> 1;
                asm volatile("" ::: "memory");
            }
            
            /* Another dependent operation */
            if (i > 0) {
                farr[i] = farr[i] + farr[i-1] * 0.5f;
            }
        }
        
        /* Reverse traversal for more complexity */
        for (int i = size-1; i >= 0; i--) {
            arr[i] = arr[i] ^ arr[(i+1) % size];
            asm volatile("" ::: "memory");
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
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
        array_d[i] = (double)(lcg_rand() % 100) * 0.01;
        array_idx[i] = lcg_rand() % SIZE;
    }
    
    volatile int threshold = 5000;
    volatile int checksum_a = 0;
    volatile int checksum_b = 0;
    
    /* Runtime decision to call functions multiple times */
    for (int outer = 0; outer < 3; outer++) {
        /* Call selective-scheduled function */
        test_selective_sched_1(array_a, array_b, array_c, SIZE, threshold);
        
        /* Volatile flag based on array state */
        volatile int flag = (array_a[0] + array_b[0]) & 0x1;
        if (flag) {
            for (int rep = 0; rep < 2; rep++) {
                test_selective_sched_2(array_d, array_idx, SIZE);
            }
        }
        
        test_selective_sched_3(array_b, array_c, SIZE, 2);
    }
    
    /* Compute final checksums to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum_a += array_a[i] ^ array_b[i];
        checksum_b += (int)array_c[i] + (int)array_d[i];
    }
    
    printf("Final checksums: %d, %d\n", checksum_a, checksum_b);
    return 0;
}
