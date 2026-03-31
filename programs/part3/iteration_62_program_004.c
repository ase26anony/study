/* sel-sched-trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_sched_1(int *arr_a, int *arr_b, float *arr_c, int size, int threshold) {
    volatile int vol_size = size; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF; /* Outer loop carried state */
        
        for (int i = 0; i < vol_size; i++) {
            /* Multiple dependency types with control flow */
            int val_a = arr_a[i];
            int val_b = arr_b[i];
            
            /* Flow dependency: sum depends on previous iteration */
            sum = sum + val_a * val_b;
            
            /* Anti dependency: arr_b[i] read before write */
            arr_b[i] = (arr_b[i] + base) * 3;
            
            /* Control dependency with unpredictable branch */
            if (sum > threshold) {
                /* Output dependency: arr_c[i] written multiple times */
                arr_c[i] = (float)sum * 0.5f;
                sum = 0;
                fsum += arr_c[i];
            } else {
                arr_c[i] = (float)(val_a + val_b) * 0.25f;
                fsum -= arr_c[i];
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < vol_size) {
                int next_a = arr_a[i + 1];
                int next_b = arr_b[i + 1];
                sum = sum - next_a + next_b;
                arr_c[i + 1] = (float)sum * 0.3f;
                
                /* Inline assembly barrier to create scheduling complexity */
                asm volatile("" ::: "memory");
            }
            
            /* Mixed floating point operations */
            double dtemp = (double)arr_c[i] * 1.5;
            fsum += (float)dtemp;
            
            /* Another unpredictable branch */
            if ((val_a & 0x3) == 0) {
                arr_a[i] = val_b ^ 0x55AA;
                asm volatile("" ::: "memory"); /* Another barrier */
            }
        }
    }
    
    /* Prevent dead code elimination */
    arr_a[0] = sum;
    arr_c[0] = fsum;
}

/* Second test with volatile counters and more barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_sched_2(double *arr_d, int *arr_e, int size) {
    volatile int vol_iter = size;
    double acc = 0.0;
    
    for (int j = 0; j < 3; j++) {
        int mod_base = j * 7;
        
        for (volatile int i = 0; i < vol_iter; i++) {
            /* Complex dependency chain */
            double d1 = arr_d[i];
            int e1 = arr_e[i];
            
            /* Multiple FP operations with dependencies */
            d1 = d1 * 1.6180339887 + (double)e1;
            acc += d1;
            
            /* Resource conflict simulation */
            arr_d[i] = acc * 0.70710678118;
            arr_e[i] = (int)acc ^ mod_base;
            
            /* Frequent barriers increase scheduling difficulty */
            asm volatile("" ::: "memory");
            
            /* Unrolled section with different operations */
            if (i % 2 == 0) {
                double temp = arr_d[i] * arr_d[i];
                arr_e[i] += (int)temp;
                asm volatile("" ::: "memory");
            }
            
            /* Data-dependent array access */
            int idx = (e1 & 0xF) % size;
            arr_d[idx] = arr_d[idx] * 0.9;
        }
        
        /* Outer loop modifies inner loop's data */
        arr_e[j] = (int)acc;
        asm volatile("" ::: "memory");
    }
}

/* Third test: outer-loop carried state pattern */
void test_outer_carried_state(int *arr, int size) {
    int state = 0;
    
    for (int phase = 0; phase < 8; phase++) {
        /* Outer loop computation affects inner loop */
        int phase_factor = (phase * 13 + 7) & 0x3F;
        volatile int inner_bound = size - (phase % 3);
        
        for (int i = 0; i < inner_bound; i++) {
            /* Loop-carried dependency through 'state' */
            int old_state = state;
            state = (state * 31 + arr[i]) & 0xFFFF;
            
            /* Complex update with phase_factor */
            arr[i] = (arr[i] + phase_factor) * (old_state % 16 + 1);
            
            /* Conditional with phase-dependent behavior */
            if ((phase + i) & 1) {
                arr[i] = arr[i] ^ 0x00FF00FF;
                asm volatile("" ::: "memory");
            }
            
            /* Additional computation every 4th iteration */
            if (i % 4 == 0) {
                state = state + phase * 256;
            }
        }
        
        /* Cross-iteration dependency */
        arr[phase % size] = state;
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d[SIZE];
    int array_e[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000) - 500;
        array_b[i] = (int)(lcg_rand() % 1000) - 500;
        array_c[i] = (float)(lcg_rand() % 100) / 10.0f;
        array_d[i] = (double)(lcg_rand() % 200) / 20.0;
        array_e[i] = (int)(lcg_rand() % 500);
    }
    
    volatile int threshold = 2500;
    volatile int use_test1 = 1;
    volatile int use_test2 = 1;
    volatile int use_test3 = 1;
    
    /* Runtime-variable control flow */
    if (use_test1) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_sched_1(array_a, array_b, array_c, SIZE, threshold + rep * 100);
        }
    }
    
    if (use_test2) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_sched_2(array_d, array_e, SIZE);
        }
    }
    
    if (use_test3) {
        for (int rep = 0; rep < 4; rep++) {
            test_outer_carried_state(array_b, SIZE);
        }
    }
    
    /* Compute final checksum to prevent elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i];
        checksum += (int64_t)array_b[i];
        checksum += (int64_t)(array_c[i] * 100);
        checksum += (int64_t)(array_d[i] * 100);
        checksum += array_e[i];
        
        /* Mix in some non-linear operations */
        if (i % 7 == 0) {
            checksum ^= (checksum << 13);
            checksum ^= (checksum >> 17);
            checksum ^= (checksum << 5);
        }
    }
    
    printf("Final checksum: %ld\n", (long)checksum);
    return 0;
}
