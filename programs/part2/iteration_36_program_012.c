/* sel-sched-test.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-test.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with memory aliasing - restrict and non-restrict pointers */
static inline void compute_loop(float *restrict arr1, float *arr2, 
                                double *restrict darr, int *restrict iarr,
                                int start, int end, int step) {
    float local_acc = 0.0f;
    double dbl_acc = 0.0;
    int int_acc = 0;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i += step) {
        /* Integer operations with carried dependency */
        int_acc = int_acc * 13 + i * 17;
        iarr[i % SIZE] = int_acc;
        
        /* Floating-point operations mixing float and double */
        float temp_f = arr1[i % SIZE] * 1.5f;
        dbl_acc = dbl_acc * 0.99 + (double)temp_f * 1.01;
        
        /* Memory operations with potential aliasing */
        arr2[i % SIZE] = temp_f + (float)dbl_acc;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and hard to schedule */
            local_acc += arr1[i % SIZE] / 3.14159f;
            darr[i % SIZE] = dbl_acc * 2.0;
        } else if (i % 13 == 0) {
            /* Another basic block path */
            local_acc -= arr2[i % SIZE] * 0.5f;
            int_acc ^= (i << 3);
        } else {
            /* Default path */
            local_acc = local_acc * 0.9f + arr1[i % SIZE];
            dbl_acc = dbl_acc - (double)arr2[i % SIZE];
        }
        
        /* More arithmetic diversity */
        if (i % 23 == 0) {
            /* Use inline asm to create memory clobber */
            asm volatile("" : : : "memory");
            int_acc = (int_acc << 1) | (int_acc >> 31); /* rotate */
        }
        
        /* Mix in some integer division */
        if (i % 100 == 0 && int_acc != 0) {
            global_seed = global_seed / (abs(int_acc % 10) + 1);
        }
    }
    
    /* Store final accumulated values */
    arr1[0] += local_acc;
    darr[0] += dbl_acc;
    iarr[0] += int_acc;
}

/* Second hot function with different access pattern */
static inline void compute_loop2(double *darr, int *iarr, float *farr, 
                                 int n, int offset) {
    double sum_d = 0.0;
    int sum_i = 0;
    float sum_f = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Interleaved memory accesses */
        double dval = darr[(i + offset) % SIZE];
        int ival = iarr[(i * 3) % SIZE];
        float fval = farr[(i * 7) % SIZE];
        
        /* Complex dependency chain */
        sum_d = sum_d * 0.8 + dval * ival;
        sum_i = sum_i ^ (ival * (int)dval);
        sum_f = sum_f + fval / ((i % 20) + 1);
        
        /* Conditional store */
        if (sum_i > 0) {
            darr[i % SIZE] = sum_d;
            farr[i % SIZE] = sum_f;
        } else {
            iarr[i % SIZE] = sum_i;
        }
        
        /* Periodic heavy operation */
        if (i % 50 == 0) {
            sum_d = sum_d / 3.1415926535;
            sum_f = sum_f * 1.1f;
        }
    }
    
    /* Final accumulation with memory barrier */
    asm volatile("" : : : "memory");
    darr[SIZE-1] = sum_d;
    iarr[SIZE-1] = sum_i;
    farr[SIZE-1] = sum_f;
}

int main(void) {
    /* Allocate and initialize arrays */
    float *arr1 = (float*)malloc(SIZE * sizeof(float));
    float *arr2 = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    int *iarr = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (float)(i * 1.1);
        arr2[i] = (float)(i * 0.7);
        darr[i] = (double)i * 0.3;
        iarr[i] = i * 11;
    }
    
    /* Call hot functions multiple times to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        compute_loop(arr1, arr2, darr, iarr, 
                     iter, iter + 100, 1 + (iter % 3));
        
        compute_loop2(darr, iarr, arr1, 200, iter * 7);
        
        /* Alternate pattern */
        compute_loop(arr2, arr1, darr, iarr, 
                     0, 150, 2 + (iter % 5));
    }
    
    /* Compute checksum to prevent optimization */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= *(uint64_t*)&arr1[i];
        checksum ^= *(uint64_t*)&arr2[i];
        checksum ^= *(uint64_t*)&darr[i];
        checksum ^= iarr[i];
    }
    
    /* Also mix in global_seed */
    checksum ^= global_seed;
    
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Free memory */
    free(arr1);
    free(arr2);
    free(darr);
    free(iarr);
    
    return 0;
}
