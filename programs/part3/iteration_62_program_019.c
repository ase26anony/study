/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with optimization attributes */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_complex_schedule(int* arr1, int* arr2, float* farr, int size, volatile int* control) {
    volatile int vlimit = *control % 256 + 128;  /* Prevent constant propagation */
    float sum_f = 0.0f;
    double acc_d = 0.0;
    int threshold = 1000;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 37) & 0xFF;
        
        /* Inner loop with high ILP and data-dependent branches */
        for (int i = 0; i < vlimit; i++) {
            /* Multiple dependency types with mixed operations */
            int idx = (i + base) % size;
            int val1 = arr1[idx];
            int val2 = arr2[idx];
            float fval = farr[idx];
            
            /* Flow dependency chain */
            sum_f = sum_f + fval * (val1 & 0xFF);
            
            /* Anti dependency */
            arr1[idx] = val1 ^ val2;
            
            /* Output dependency with control flow */
            if (sum_f > threshold) {
                farr[idx] = sum_f;
                sum_f = 0.0f;
                /* Inline asm barrier to create scheduling boundary */
                asm volatile("" ::: "memory");
            }
            
            /* Parallel computation with different data types */
            acc_d += (double)val2 * 0.5;
            
            /* Data-dependent branch with unpredictable pattern */
            if (val1 & 0x1) {
                arr2[idx] = val1 + (int)acc_d;
                /* Another scheduling barrier */
                asm volatile("" ::: "memory");
            } else {
                arr2[idx] = val1 - (int)(acc_d * 0.3);
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < vlimit) {
                int idx2 = (i + 1 + base) % size;
                int val1_2 = arr1[idx2];
                float fval2 = farr[idx2];
                
                sum_f = sum_f + fval2 * (val1_2 & 0x7F);
                arr1[idx2] = val1_2 ^ (val2 >> 1);
                
                if ((val1_2 & 0x3) == 0) {
                    farr[idx2] = sum_f * 0.5f;
                    asm volatile("" ::: "memory");
                }
                
                i++;  /* Increment counter for unrolled iteration */
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        threshold += (int)sum_f;
    }
}

/* Function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(int* data, int size, volatile int iter_count) {
    volatile int v_counter = iter_count;
    float temp[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    
    while (v_counter > 0) {
        /* Nested loops with resource conflicts */
        for (int j = 0; j < 8; j++) {
            int base = (j * 29) % size;
            
            for (int i = 0; i < 64; i++) {
                int idx = (base + i) % size;
                
                /* Multiple FP operations that could compete for FPU */
                float f1 = (float)data[idx];
                float f2 = (float)data[(idx + 1) % size];
                double d1 = (double)data[(idx + 2) % size];
                
                temp[j % 4] = temp[j % 4] + f1 * f2;
                
                /* Complex conditional with side effects */
                if ((data[idx] ^ data[(idx + 3) % size]) > 0) {
                    data[idx] = (int)(temp[j % 4] + d1);
                    /* Scheduling barrier in conditional path */
                    asm volatile("" ::: "memory");
                }
                
                /* More mixed operations */
                temp[(j + 1) % 4] -= f2 * 0.7f;
                d1 = d1 * 1.1 + (double)temp[j % 4];
                
                /* Another barrier to split scheduling regions */
                asm volatile("" ::: "memory");
                
                data[(idx + 1) % size] = (int)d1;
            }
        }
        
        v_counter--;
    }
}

/* Outer loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_outer_carried_state(int* arr, float* farr, int size) {
    int state = 0;
    
    for (int outer = 0; outer < 16; outer++) {
        /* Compute state based on outer iteration */
        state = (state * 131 + outer * 17) & 0x3FF;
        int factor = (state >> 2) + 1;
        
        /* Inner loop using outer loop state */
        for (int i = 0; i < size; i++) {
            /* Loop-carried dependency through 'state' */
            int val = arr[i];
            float fval = farr[i];
            
            /* Complex transformation with outer state */
            arr[i] = (val + state) * factor;
            farr[i] = fval * factor + (float)state;
            
            /* Data-dependent update of state */
            if (val > 0x7FFF) {
                state = (state + 1) & 0x3FF;
                asm volatile("" ::: "memory");
            }
            
            /* Additional computation to increase ILP */
            float tmp = farr[(i + 1) % size];
            farr[(i + 1) % size] = tmp * 0.99f + fval * 0.01f;
            
            /* Manual unrolling (3 iterations) */
            if (i + 2 < size) {
                int i2 = i + 1;
                int i3 = i + 2;
                
                arr[i2] = (arr[i2] + (state >> 1)) * (factor >> 1);
                arr[i3] = (arr[i3] + (state >> 2)) * (factor >> 2);
                
                farr[i2] = farr[i2] * 1.01f;
                farr[i3] = farr[i3] * 0.99f;
                
                i += 2;  /* Account for unrolled iterations */
            }
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    int array1[SIZE];
    int array2[SIZE];
    float farray[SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)lcg_rand() % 65536;
        array2[i] = (int)lcg_rand() % 65536;
        farray[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    volatile int control = lcg_rand() % 100;
    volatile int flag1 = (control > 30);
    volatile int flag2 = (control > 60);
    volatile int flag3 = (control > 90);
    
    /* Variable execution paths based on runtime values */
    if (flag1) {
        for (int rep = 0; rep < 5; rep++) {
            test_complex_schedule(array1, array2, farray, SIZE, &control);
            control = (control * 3 + 1) & 0xFF;
        }
    }
    
    if (flag2) {
        volatile int iterations = (control % 8) + 2;
        for (int rep = 0; rep < 3; rep++) {
            test_volatile_barriers(array1, SIZE, iterations);
            iterations = (iterations + 1) % 10;
        }
    }
    
    if (flag3) {
        for (int rep = 0; rep < 4; rep++) {
            test_outer_carried_state(array2, farray, SIZE);
        }
    } else {
        /* Always run at least once */
        test_outer_carried_state(array1, farray, SIZE / 2);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (long long)farray[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}
