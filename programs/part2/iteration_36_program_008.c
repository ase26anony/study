/* sel-sched-coverage.c
 * Designed to trigger selective scheduler debug output for sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict dest, int *src1, int *src2, 
                                float *farr, double *darr, int n) {
    int i;
    float ftmp = 0.0f;
    double dtmp = 0.0;
    int local_sum = 0;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Integer operations with carried dependency */
        int val1 = src1[i] + local_sum;
        int val2 = src2[i] * (i + 1);
        
        /* Memory store with potential aliasing */
        dest[i] = val1 + val2;
        
        /* Floating-point operations */
        ftmp += farr[i] * 1.5f;
        dtmp += darr[i] / 2.0;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in conditional path */
            ftmp = ftmp * 0.9f;
            dtmp = dtmp - 1.0;
            g_volatile_counter++;  /* Volatile access */
        } else if (i % 13 == 0) {
            /* Another basic block */
            ftmp = ftmp + 10.0f;
            dest[i] = dest[i] >> 1;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            local_sum += val1 * 2;
        } else {
            local_sum -= val2 / 3;
        }
        
        /* Complex expression with mixed operations */
        farr[i] = (float)(dest[i] % 256) + ftmp * 0.01f;
        darr[i] = (double)(val1 & 0xFF) + dtmp * 0.001;
        
        /* Inline asm with memory clobber to prevent optimization */
        asm volatile("" : : "r"(dest[i]), "r"(farr[i]) : "memory");
    }
    
    /* Final store with side effect */
    if (n > 0) {
        dest[0] += (int)(ftmp + dtmp);
    }
}

/* Another inline function to encourage inlining */
static inline int process_chunk(int *restrict out, int *in1, int *in2, 
                                float *fdata, double *ddata, int start, int end) {
    int checksum = 0;
    
    for (int i = start; i < end; i++) {
        /* Mixed operations */
        int tmp = in1[i] ^ in2[i];
        float ftmp = fdata[i] * 3.14159f;
        double dtmp = ddata[i] * 2.71828;
        
        /* Conditional with arithmetic */
        if (tmp % 11 == 0) {
            out[i] = tmp + (int)(ftmp * 100);
            checksum ^= out[i] * 3;
        } else {
            out[i] = tmp - (int)(dtmp * 50);
            checksum ^= out[i] * 7;
        }
        
        /* Update floating arrays */
        fdata[i] = ftmp * 0.5f;
        ddata[i] = dtmp * 0.25;
    }
    
    return checksum;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    int *result = malloc(SIZE * sizeof(int));
    float *farray = malloc(SIZE * sizeof(float));
    double *darray = malloc(SIZE * sizeof(double));
    
    if (!array1 || !array2 || !result || !farray || !darray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7 + 1;
        farray[i] = (float)i * 0.5f;
        darray[i] = (double)i * 0.25;
    }
    
    int final_checksum = 0;
    
    /* Multiple calls to hot functions to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call the main compute loop */
        compute_loop(result, array1, array2, farray, darray, SIZE);
        
        /* Process in chunks to create more scheduling opportunities */
        int chunk_size = SIZE / 4;
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * chunk_size;
            int end = (chunk == 3) ? SIZE : start + chunk_size;
            final_checksum ^= process_chunk(result, array1, array2, 
                                          farray, darray, start, end);
        }
        
        /* Rotate data to create varying patterns */
        if (iter % 100 == 0) {
            int temp = array1[0];
            for (int i = 0; i < SIZE - 1; i++) {
                array1[i] = array1[i + 1];
            }
            array1[SIZE - 1] = temp;
        }
    }
    
    /* Compute final result to prevent optimization */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum ^= result[i];
        sum += (int)farray[i];
        sum ^= (int)darray[i];
    }
    
    sum ^= final_checksum;
    sum += g_volatile_counter;
    
    printf("Result checksum: %d\n", sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(farray);
    free(darray);
    
    return 0;
}
