/* sel-sched-test.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict dest, int *src1, int *src2, 
                                float *farr, double *darr, int n) {
    int i;
    float f_acc = 1.0f;
    double d_acc = 1.0;
    int int_acc = global_seed;
    
    /* Complex loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Integer operations with carried dependency */
        int_acc = int_acc * 1103515245 + 12345;
        
        /* Load operations with potential aliasing */
        int val1 = src1[i];
        int val2 = src2[i % 16];  /* Smaller array for modulo */
        
        /* Multiple independent arithmetic operations */
        int sum = val1 + val2;
        int diff = val1 - val2;
        int prod = val1 * val2;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and hard to schedule */
            if (val2 != 0) {
                dest[i] = sum / (val2 + 1);
            } else {
                dest[i] = sum;
            }
            
            /* Floating point operations */
            f_acc = f_acc * 1.01f + farr[i % 8];
            d_acc = d_acc * 1.0001 + darr[i % 8];
        } else if (i % 3 == 0) {
            dest[i] = diff + int_acc % 256;
            
            /* More floating point ops */
            f_acc = f_acc / 1.02f - farr[(i + 1) % 8];
            d_acc = d_acc / 1.0002 - darr[(i + 1) % 8];
        } else {
            dest[i] = prod - int_acc % 128;
            
            /* Mixed precision operations */
            f_acc = f_acc + (float)d_acc * 0.5f;
            d_acc = d_acc + (double)f_acc * 0.5;
        }
        
        /* Memory store with barrier-like effect */
        if (i % 13 == 0) {
            /* Inline asm to create memory clobber */
            asm volatile("" ::: "memory");
        }
        
        /* Additional independent operations to increase ILP */
        float ftmp = farr[i % 8] * 2.0f;
        double dtmp = darr[i % 8] * 3.0;
        int bitop = val1 ^ val2 ^ int_acc;
        
        /* Use results to prevent dead code elimination */
        farr[i % 8] = ftmp + 1.0f;
        darr[i % 8] = dtmp - 1.0;
        src1[i] = bitop;
    }
    
    /* Store accumulated values to prevent optimization */
    dest[0] += (int)f_acc;
    dest[1] += (int)d_acc;
}

/* Secondary computation with different pattern */
static inline void compute_loop2(double *restrict out, const int *in, 
                                 float *work, int n) {
    double acc = 0.0;
    float f_acc = 0.0f;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Polynomial calculation with dependencies */
        double x = (double)in[i] / 1000.0;
        acc = acc + x * x * 1.5 - x * 2.0 + 1.0;
        
        /* Trigonometric approximation */
        float fx = (float)x;
        f_acc = f_acc + fx * fx * fx / 6.0f - fx * fx / 2.0f + fx;
        
        /* Conditional store */
        if (i % 5 == 0) {
            out[i] = acc;
            work[i % 16] = f_acc;
        } else {
            out[i] = -acc;
            work[i % 16] = -f_acc;
        }
        
        /* Periodic memory barrier */
        if (i % 17 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

int main(void) {
    const int N = 1024;
    const int ITERS = 1000;
    
    /* Allocate and initialize arrays */
    int *dest = (int*)malloc(N * sizeof(int));
    int *src1 = (int*)malloc(N * sizeof(int));
    int *src2 = (int*)malloc(16 * sizeof(int));
    float *farr = (float*)malloc(8 * sizeof(float));
    double *darr = (double*)malloc(8 * sizeof(double));
    double *out = (double*)malloc(N * sizeof(double));
    float *work = (float*)malloc(16 * sizeof(float));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        src1[i] = (i * 1103515245 + 12345) & 0x7FFF;
        dest[i] = 0;
    }
    
    for (int i = 0; i < 16; i++) {
        src2[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    
    for (int i = 0; i < 8; i++) {
        farr[i] = (float)i * 0.5f;
        darr[i] = (double)i * 0.25;
    }
    
    for (int i = 0; i < 16; i++) {
        work[i] = (float)i * 0.1f;
    }
    
    /* Perform multiple iterations to create hot loop */
    int checksum = 0;
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify global seed to change computation */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call the compute loops - these should be inlined */
        compute_loop(dest, src1, src2, farr, darr, N);
        compute_loop2(out, dest, work, N);
        
        /* Simple checksum to prevent optimization */
        for (int i = 0; i < 16; i++) {
            checksum ^= dest[i];
            checksum ^= (int)out[i];
            checksum ^= (int)work[i];
        }
    }
    
    /* Print result to ensure computation isn't optimized away */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dest);
    free(src1);
    free(src2);
    free(farr);
    free(darr);
    free(out);
    free(work);
    
    return 0;
}
