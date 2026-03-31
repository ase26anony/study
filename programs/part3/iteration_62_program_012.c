/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 8
#define INNER_ITER 128

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245U + 12345U) & 0x7fffffffU;
    return lcg_seed;
}

/* Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_scheduling_1(int *arr1, int *arr2, volatile int threshold) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Nested loop with data-dependent branches and mixed operations */
    for (int j = 0; j < OUTER_ITER; j++) {
        int base = (j * 37) & 0xFF;
        
        /* Manually unrolled inner loop with control flow */
        for (int i = 0; i < INNER_ITER; i += 2) {
            /* Multiple dependency types with arithmetic */
            int idx1 = (i + base) % SIZE;
            int idx2 = (i + base + 1) % SIZE;
            
            /* Flow dependency: sum depends on previous sum */
            sum = sum + arr1[idx1] * arr2[idx2];
            
            /* Anti dependency: arr1 read before write */
            int temp = arr1[idx1];
            arr1[idx1] = arr2[idx2] + base;
            
            /* Output dependency: arr2 written multiple times */
            arr2[idx2] = temp - base;
            
            /* Control dependency with branch */
            if (sum > threshold) {
                /* Mixed floating-point operations */
                fsum = fsum + (float)sum * 0.5f;
                dsum = dsum + (double)fsum * 0.25;
                sum = sum / 2;
                
                /* Inline assembly barrier - creates scheduling complexity */
                asm volatile("" ::: "memory");
            }
            
            /* Second unrolled iteration with different pattern */
            idx1 = (i + base + 2) % SIZE;
            idx2 = (i + base + 3) % SIZE;
            
            /* More complex arithmetic chain */
            int prod = arr1[idx1] * 3;
            int diff = arr2[idx2] - prod;
            
            /* Conditional with side effects */
            if (diff & 1) {  /* Data-dependent branch */
                arr1[idx1] = diff + (base << 1);
                fsum = fsum - (float)diff * 0.1f;
            } else {
                arr2[idx2] = prod + (base >> 1);
                dsum = dsum + (double)prod * 0.01;
            }
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Loop-carried dependency across outer iterations */
        threshold = (threshold + base) & 0x3FF;
    }
    
    /* Prevent dead code elimination */
    arr1[0] = (int)(fsum + dsum + sum);
}

/* Function with volatile counters and explicit barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
void test_selective_scheduling_2(float *farr, double *darr, volatile int limit) {
    volatile int counter = 0;
    double accumulator = 0.0;
    
    /* Loop with volatile condition */
    while (counter < limit) {
        int idx = counter % SIZE;
        
        /* Mixed FP operations with potential resource conflicts */
        float fval = farr[idx];
        double dval = darr[idx];
        
        /* Multiple FP operations that could compete for FP units */
        fval = fval * 1.5f + (float)counter * 0.1f;
        dval = dval * 2.5 - (double)counter * 0.01;
        
        /* Conditional FP store */
        if (fval > 100.0f || dval < -50.0) {
            farr[idx] = fval * 0.9f;
            darr[idx] = dval / 1.1;
            
            /* Memory barrier inside conditional path */
            asm volatile("" ::: "memory");
        }
        
        /* Complex dependency chain */
        accumulator = accumulator + (double)fval + dval;
        
        /* Data-dependent array update */
        if (accumulator > 1000.0) {
            int alt_idx = (idx * 7) % SIZE;
            farr[alt_idx] = (float)accumulator * 0.5f;
            accumulator = accumulator * 0.5;
        }
        
        counter++;
        
        /* Periodic barrier */
        if (counter % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Store result to prevent elimination */
    farr[SIZE-1] = (float)accumulator;
}

/* Outer-loop carried state pattern */
void test_outer_loop_carried(int *data, volatile int outer_iters) {
    int state = 0;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* State carried from outer to inner loop */
        int base = state + outer * 73;
        int factor = (base & 0x1F) + 1;
        
        /* Inner loop with outer-loop dependent computation */
        for (int inner = 0; inner < INNER_ITER; inner++) {
            int idx = (inner + outer) % SIZE;
            
            /* Complex update with outer-loop state */
            int old = data[idx];
            data[idx] = (old + base) * factor - (state >> 2);
            
            /* Update state based on computation */
            state = (state + data[idx]) & 0xFFF;
            
            /* Unrolled second iteration */
            if (inner + 1 < INNER_ITER) {
                idx = (inner + outer + 1) % SIZE;
                old = data[idx];
                data[idx] = (old - base) * (factor ^ 0xA5) + (state << 1);
                state = (state ^ data[idx]) & 0xFFF;
                inner++;
            }
        }
        
        /* Outer-loop update with barrier */
        asm volatile("" ::: "memory");
        base = (base * 97) & 0x3FF;
    }
    
    data[0] = state;
}

int main(void) {
    /* Initialize arrays with non-uniform data */
    int array1[SIZE], array2[SIZE];
    float farray[SIZE];
    double darray[SIZE];
    
    printf("Initializing arrays with pseudo-random data...\n");
    
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)lcg_rand() % 1000;
        array2[i] = (int)lcg_rand() % 1000;
        farray[i] = (float)(lcg_rand() % 1000) * 0.1f;
        darray[i] = (double)(lcg_rand() % 1000) * 0.01;
    }
    
    /* Volatile flag to prevent optimization */
    volatile int flag = 1;
    volatile int threshold = 500;
    volatile int limit = 256;
    
    printf("Running selective scheduling tests...\n");
    
    /* Variable control flow based on volatile flag */
    if (flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_scheduling_1(array1, array2, threshold);
            threshold += 50;  /* Change threshold each iteration */
        }
    }
    
    /* More conditional execution */
    volatile int alt_flag = array1[0] & 1;
    if (alt_flag) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_scheduling_2(farray, darray, limit);
            limit = (limit + 64) % 384;
        }
    }
    
    /* Always run outer loop test */
    for (int rep = 0; rep < 4; rep++) {
        test_outer_loop_carried(array1, OUTER_ITER);
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (int64_t)farray[i] + (int64_t)darray[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    printf("Test completed. Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -fdump-rtl-all sel-sched-trigger.c\n");
    
    return 0;
}
