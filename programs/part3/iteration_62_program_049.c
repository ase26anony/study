/* sel-sched-trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int *arr1, int *arr2, float *farr1, float *farr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 1000) - 500;
        arr2[i] = (int)(lcg_rand() % 1000) - 500;
        farr1[i] = (float)(lcg_rand() % 1000) / 100.0f;
        farr2[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
}

/* Test function 1: Complex nested loops with data-dependent branches and mixed operations */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_function_1(int *arr1, int *arr2, float *farr1, float *farr2, int size) {
    volatile int outer_volatile = size / 4;  /* Prevent constant propagation */
    float accumulator = 0.0f;
    int int_acc = 0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < outer_volatile; j++) {
        int base = (j * 73) & 0xFF;  /* Non-trivial base computation */
        
        /* Inner loop with high ILP and unpredictable control flow */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types and mixed operations */
            float temp_f = farr1[i] * farr2[i] + (float)base;
            int temp_i = arr1[i] + arr2[i] - base;
            
            /* Data-dependent branch creating control dependency */
            if (arr1[i] & 1) {  /* Unpredictable branch */
                accumulator += temp_f * 2.0f;
                int_acc += temp_i * 3;
                arr1[i] = (arr1[i] << 1) | (arr1[i] >> 31);  /* Rotation */
            } else {
                accumulator -= temp_f * 0.5f;
                int_acc -= temp_i / 2;
                arr2[i] = (arr2[i] ^ base) + i;
            }
            
            /* Additional floating-point operation with dependency */
            farr1[i] = accumulator * 0.9f + farr2[i];
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                float next_f = farr1[i+1] * 1.1f - (float)base;
                if (arr2[i+1] > 0) {
                    accumulator += next_f;
                    arr2[i+1] = arr2[i+1] * 2 - base;
                }
                i++;  /* Advance counter for manual unroll */
            }
            
            /* Inline assembly barrier to create scheduling complexity */
            asm volatile("" ::: "memory");
        }
        
        /* Loop-carried dependency across outer iterations */
        base = (base + int_acc) & 0x7F;
        accumulator = accumulator * 0.99f + (float)base;
    }
}

/* Test function 2: Volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void test_function_2(double *darr, int *iarr, int size) {
    volatile int v_limit = size;  /* Volatile prevents optimization */
    double sum = 0.0;
    
    for (volatile int v_i = 0; v_i < v_limit; v_i++) {
        int i = v_i;  /* Convert to non-volatile for array access */
        
        /* Complex dependency chain with multiple operation types */
        double d1 = darr[i] * 1.234567;
        double d2 = darr[size - i - 1] * 0.987654;
        
        /* Conditional with side effects */
        if (iarr[i] % 7 == 0) {
            darr[i] = d1 + d2;
            iarr[i] = (iarr[i] * 3 + i) & 0xFFFF;
        } else if (iarr[i] % 5 == 0) {
            darr[i] = d1 - d2;
            iarr[i] = (iarr[i] / 2 + i) & 0xFFFF;
        } else {
            darr[i] = d1 * d2;
            iarr[i] = (iarr[i] + 777) & 0xFFFF;
        }
        
        /* Running sum with loop-carried dependency */
        sum += darr[i] * (i % 10 + 1);
        
        /* Multiple assembly barriers to force scheduling boundaries */
        asm volatile("" ::: "memory");
        asm volatile("" ::: "memory");
        
        /* Additional unrolled iteration */
        if (i + 3 < size) {
            double temp = darr[i+3] * sum;
            darr[i+3] = temp * 0.5;
            asm volatile("" ::: "memory");
        }
    }
}

/* Test function 3: Outer-loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static void test_function_3(int *arr, float *farr, int size) {
    const int OUTER = 8;
    const int INNER = size / OUTER;
    
    int outer_state = 0;
    
    for (int j = 0; j < OUTER; j++) {
        /* Compute base from outer loop state - creates loop-carried dependency */
        int base = (outer_state * j + 123) & 0xFF;
        float factor = 1.0f + (float)(j % 4) * 0.25f;
        
        /* Inner loop with dependency on outer loop variables */
        for (int i = 0; i < INNER; i++) {
            int idx = j * INNER + i;
            if (idx >= size) break;
            
            /* Mixed operations with flow dependencies */
            int old_val = arr[idx];
            arr[idx] = (old_val + base) * (i % 3 + 1);
            
            float f_old = farr[idx];
            farr[idx] = (f_old + (float)base) * factor;
            
            /* Anti-dependency: read after write with different index */
            if (i > 0) {
                int prev_idx = j * INNER + (i - 1);
                arr[idx] += arr[prev_idx] % 256;
            }
            
            /* Output dependency simulation */
            {
                int temp = arr[idx];
                arr[idx] = temp ^ base;  /* Overwrites previous value */
            }
            
            /* Resource conflict: multiple FP operations */
            farr[idx] = farr[idx] * 1.1f - 0.1f;
            float f_temp = farr[idx] * farr[idx];
            farr[idx] = f_temp * 0.5f + farr[idx];
        }
        
        /* Update outer loop carried state */
        outer_state = (outer_state + arr[j * INNER]) & 0x7F;
    }
}

/* Main function with runtime variability */
int main(void) {
    const int ARRAY_SIZE = 1024;
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    float farray1[ARRAY_SIZE];
    float farray2[ARRAY_SIZE];
    double darray[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    init_arrays(array1, array2, farray1, farray2, ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        darray[i] = (double)(lcg_rand() % 1000) / 50.0;
    }
    
    /* Volatile flag for runtime control flow variability */
    volatile int volatile_flag = (array1[0] > 0) ? 1 : 0;
    
    /* Call test functions with runtime-dependent repetition */
    if (volatile_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_function_1(array1, array2, farray1, farray2, ARRAY_SIZE);
        }
    }
    
    /* Always execute these, but with different parameters */
    test_function_2(darray, array1, ARRAY_SIZE / 2);
    
    for (int rep = 0; rep < 3; rep++) {
        test_function_3(array2, farray2, ARRAY_SIZE);
    }
    
    /* Additional calls based on dynamic condition */
    volatile_flag = (array2[ARRAY_SIZE/2] % 2 == 0) ? 1 : 0;
    if (volatile_flag) {
        test_function_1(array2, array1, farray2, farray1, ARRAY_SIZE);
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i];
        checksum += array2[i];
        checksum += (int64_t)(farray1[i] * 1000.0f);
        checksum += (int64_t)(farray2[i] * 1000.0f);
        checksum += (int64_t)(darray[i] * 1000.0);
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    return 0;
}
