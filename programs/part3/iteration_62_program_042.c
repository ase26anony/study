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
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_sched_1(int* restrict arr1, int* restrict arr2, 
                           float* restrict farr1, float* restrict farr2,
                           int size, volatile int* vflag) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    int sum_i = 0;
    
    /* Create complex data dependencies with mixed types */
    for (int i = 0; i < size; i++) {
        /* Flow dependency: sum_i depends on previous iteration */
        sum_i += arr1[i] * arr2[i];
        
        /* Anti dependency: arr1[i] read before write */
        arr1[i] = (arr1[i] + sum_i) & 0xFF;
        
        /* Output dependency: farr1[i] written twice in same iteration */
        farr1[i] = (float)sum_i * 0.5f;
        
        /* Data-dependent conditional branch - unpredictable */
        if (arr1[i] & 1) {
            /* Control dependency inside loop */
            farr1[i] = farr1[i] * 2.0f + farr2[i];
            sum_f += farr1[i];
            
            /* Inline assembly barrier - creates scheduling boundary */
            asm volatile("" ::: "memory");
        } else {
            sum_d += (double)farr1[i] * 0.25;
        }
        
        /* Manual unrolling (2 iterations) for more ILP */
        if (i + 1 < size) {
            int j = i + 1;
            sum_i -= arr2[j] >> 2;
            arr2[j] = arr1[j] ^ sum_i;
            
            /* Another conditional with different computation */
            if (arr2[j] > 1000) {
                farr2[j] = farr1[j] * 3.14f;
                asm volatile("" ::: "memory");
            }
            i = j;  /* Advance i */
        }
        
        /* Loop-carried dependency through volatile flag */
        if (*vflag & 0x1) {
            sum_i >>= 1;
        }
    }
    
    /* Prevent dead code elimination */
    arr1[0] = sum_i;
    farr1[0] = sum_f;
    farr2[0] = (float)sum_d;
}

/* Function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_sched_2(volatile int* data, int size) {
    volatile int counter = 0;
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    
    /* Nested loops with outer-loop carried state */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17 + 123) & 0xFF;
        
        /* Inner loop with multiple dependency types */
        for (int i = 0; i < size; i++) {
            /* Multiple accumulators for parallel operations */
            acc1 += data[i] * 0.1f;
            acc2 += data[size - i - 1] * 0.2f;
            acc3 += (float)(data[i] + base) * 0.3f;
            
            /* Resource conflict simulation: multiple FP operations */
            float temp1 = acc1 * acc2;
            float temp2 = acc2 * acc3;
            float temp3 = acc3 * acc1;
            
            /* Conditional store with anti-dependency */
            if (temp1 > temp2) {
                data[i] = (int)(temp1 - temp2);
                asm volatile("" ::: "memory");
            } else if (temp3 > 100.0f) {
                data[i] = (int)(temp3 * 0.5f);
            }
            
            /* Volatile counter update - can't be reordered */
            counter++;
            
            /* Another manual unrolling */
            if (i % 3 == 0 && i + 2 < size) {
                data[i + 1] ^= base;
                data[i + 2] += counter;
                i += 2;
            }
        }
        
        /* Outer loop dependency */
        base = (base * 3 + 7) & 0xFF;
    }
    
    /* Use results */
    data[0] += (int)(acc1 + acc2 + acc3);
}

/* Outer-loop carried state pattern */
void test_outer_loop_carried(int* arr, int size, int iterations) {
    int state = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Outer loop modifies state used in inner loop */
        state = (state * 13 + iter) & 0xFFF;
        float factor = 1.0f + (state % 100) * 0.01f;
        
        /* Inner loop with complex computations */
        for (int i = 0; i < size; i++) {
            /* Mix of integer and floating point */
            int val = arr[i];
            float fval = (float)val * factor;
            
            /* Data-dependent branching */
            if (val > 1000) {
                fval = fval * 2.0f - (float)state;
                arr[i] = (int)fval;
                asm volatile("" ::: "memory");
            } else if (val < 100) {
                fval = fval * 0.5f + (float)(state >> 4);
                arr[i] = (int)fval;
            } else {
                arr[i] = (int)(fval * 1.5f);
            }
            
            /* Loop-carried dependency through array */
            if (i > 0) {
                arr[i] ^= arr[i - 1] & 0xFF;
            }
        }
        
        /* Prevent over-optimization */
        asm volatile("" ::: "memory");
    }
}

int main(void) {
    const int SIZE = 1024;
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    volatile int vflag = 1;
    
    /* Initialize with pseudo-random but non-uniform data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (int)(lcg_rand() % 2000);
        arr2[i] = (int)(lcg_rand() % 2000);
        farr1[i] = (float)(lcg_rand() % 1000) * 0.01f;
        farr2[i] = (float)(lcg_rand() % 1000) * 0.02f;
    }
    
    /* Runtime variability based on checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i] ^ arr2[i];
    }
    
    vflag = checksum & 0xFF;
    
    /* Call test functions multiple times with runtime decisions */
    if (vflag & 0x1) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_sched_1(arr1, arr2, farr1, farr2, SIZE, &vflag);
            vflag = (vflag * 3 + 1) & 0xFF;
        }
    }
    
    if (vflag & 0x2) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_sched_2((volatile int*)arr1, SIZE / 2);
        }
    }
    
    if (vflag & 0x4) {
        test_outer_loop_carried(arr2, SIZE, 8);
    }
    
    /* Alternate between patterns */
    for (int alt = 0; alt < 4; alt++) {
        if (alt % 2 == 0) {
            test_selective_sched_1(arr1, arr2, farr1, farr2, SIZE / 4, &vflag);
        } else {
            test_outer_loop_carried(arr1, SIZE / 4, 2);
        }
        vflag = (vflag + 17) & 0xFF;
    }
    
    /* Final checksum to prevent dead code elimination */
    int final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += arr1[i];
        final_checksum ^= arr2[i];
        final_checksum += (int)farr1[i];
        final_checksum ^= (int)farr2[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    return 0;
}
