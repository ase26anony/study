/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_sched_1(int* arr_a, int* arr_b, int* arr_c, int size) {
    volatile int threshold = 1000; /* Prevent constant propagation */
    int sum = 0;
    int factor = 3;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int j = 0; j < 4; j++) { /* Outer loop */
        int base = (j * 7) & 0xF; /* Simple computation for outer loop state */
        
        /* Manually unrolled inner loop with scheduling barriers */
        for (int i = 0; i < size - 3; i += 4) {
            /* First iteration - mixed arithmetic with flow dependency */
            int temp1 = arr_a[i] * factor + base;
            asm volatile("" ::: "memory"); /* Scheduling barrier */
            
            /* Data-dependent conditional branch */
            if (temp1 & 1) {
                arr_b[i] = temp1 + arr_b[i];
                sum += arr_b[i];
            } else {
                arr_b[i] = temp1 - arr_b[i];
                sum -= arr_b[i];
            }
            
            /* Second iteration - different operations */
            float ftemp = (float)arr_a[i+1] * 1.5f + (float)base;
            asm volatile("" ::: "memory");
            
            if (sum > threshold) {
                arr_c[i+1] = (int)ftemp;
                sum = sum / 2; /* Control-dependent update */
            }
            
            /* Third iteration - more complex dependency chain */
            double dtemp = (double)arr_a[i+2] * 2.5 + (double)base;
            int itemp = (int)dtemp;
            
            /* Anti-dependency: arr_b[i+2] read before write */
            int old_val = arr_b[i+2];
            arr_b[i+2] = itemp * old_val;
            sum += arr_b[i+2] - old_val;
            
            /* Fourth iteration - output dependency */
            arr_c[i+3] = arr_a[i+3] + base;
            asm volatile("" ::: "memory");
            arr_c[i+3] = arr_c[i+3] * factor; /* Overwrites previous value */
            
            /* Loop-carried dependency through sum */
            if (sum < -threshold) {
                sum = 0;
            }
        }
    }
}

/* Function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
void test_selective_sched_2(double* darr, float* farr, int size) {
    volatile int v_counter = size; /* Volatile prevents optimization */
    double acc_d = 0.0;
    float acc_f = 0.0f;
    
    /* Loop with multiple dependency types and resource conflicts */
    for (volatile int vi = 0; vi < v_counter; vi++) {
        /* Multiple FP operations that could compete for FP units */
        double d1 = darr[vi] * 1.234567;
        double d2 = darr[vi] * 2.345678;
        asm volatile("" ::: "memory");
        
        float f1 = farr[vi] * 3.456789f;
        float f2 = farr[vi] * 4.567890f;
        
        /* Conditional with unpredictable pattern */
        if ((vi * 1103515245 + 12345) & 0x100) {
            darr[vi] = d1 + d2;
            acc_d += darr[vi];
        } else {
            farr[vi] = f1 - f2;
            acc_f += farr[vi];
        }
        
        /* Cross-iteration dependency through accumulator */
        if (acc_d > 10000.0) {
            acc_d *= 0.9;
            asm volatile("" ::: "memory");
        }
        
        /* Nested conditional for additional control flow */
        if (vi % 7 == 0) {
            for (int k = 0; k < 2; k++) { /* Tiny inner loop */
                darr[vi] += k * 0.5;
            }
        }
    }
}

/* Outer loop carried state pattern */
void test_outer_carried_state(int* arr, int size, int outer_iters) {
    int state = 0;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Compute state that depends on outer iteration */
        int base = (state + outer * 13) & 0xFF;
        int factor = 1 + (base % 5); /* Varying factor */
        
        /* Inner loop with outer-loop carried state */
        for (int i = 0; i < size; i++) {
            /* Complex addressing with potential aliasing */
            int idx = (i + base) % size;
            
            /* Multiple operations with loop-carried dependency */
            int old = arr[idx];
            arr[idx] = (old * factor + base) ^ state;
            
            /* Update state based on result */
            state = (state + arr[idx]) & 0xFFFF;
            
            /* Data-dependent branch */
            if (arr[idx] > 1000000) {
                arr[idx] = arr[idx] % 1000;
                state >>= 1;
            }
        }
        
        /* Outer loop update with dependency on inner loop results */
        if (state > 10000) {
            state = 0;
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    int array_c[SIZE];
    double darray[SIZE];
    float farray[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = 0;
        darray[i] = (double)(lcg_rand() % 1000) / 10.0;
        farray[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    volatile int run_flag = 1; /* Prevent compile-time elimination */
    int checksum = 0;
    
    /* Variable execution pattern based on runtime values */
    if (run_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_sched_1(array_a, array_b, array_c, SIZE);
        }
    }
    
    /* Conditional execution based on array content */
    if (array_a[0] % 2 == 0) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_sched_2(darray, farray, SIZE);
        }
    }
    
    /* Always run outer carried state test */
    test_outer_carried_state(array_b, SIZE, 8);
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i] + array_b[i] + array_c[i];
        checksum += (int)darray[i] + (int)farray[i];
        checksum &= 0xFFFFFF; /* Keep it bounded */
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
