/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;
static volatile double g_volatile_double = 1.0;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict dest, int *src1, int *src2, 
                               float *farr, double *darr, int n) {
    int i;
    float ftemp = 0.0f;
    double dtemp = g_volatile_double;
    int local_sum = 0;
    
    /* Complex loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Integer operations with carried dependency */
        int idx = (i * 13) % n;
        int val1 = src1[idx] + local_sum;  /* Carried dependency */
        int val2 = src2[i] * 3;
        
        /* Floating-point operations */
        float fval = farr[i] * 2.5f + ftemp;
        double dval = darr[idx] / (dtemp + 1.0);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Different execution path */
            val1 = val1 ^ (i & 0xFF);
            fval = fval - 1.0f;
            dval = dval * 0.5;
            
            /* Memory store with potential aliasing concern */
            dest[i] = val1 + val2 + (int)fval;
            
            /* More operations in this branch */
            ftemp = fval * 0.8f;
            dtemp = dval + 0.1;
        } else if (i % 13 == 0) {
            /* Another execution path */
            val1 = val1 >> 2;
            fval = fval + 2.0f;
            
            dest[i] = val1 - val2;
            
            /* Complex floating-point chain */
            ftemp = ftemp + fval * 0.3f;
            dtemp = dtemp - dval * 0.2;
        } else {
            /* Default path */
            dest[i] = val1 * val2 - (int)(fval * 10.0f);
            
            /* Update accumulators with dependencies */
            ftemp = fval;
            dtemp = dval;
        }
        
        /* Update carried dependency */
        local_sum = local_sum + dest[i] % 256;
        
        /* More arithmetic diversity */
        if (i % 5 == 0) {
            /* Integer division - expensive operation */
            local_sum = local_sum / (val1 % 16 + 1);
        }
        
        /* Memory operations with different access patterns */
        if (i % 3 == 0) {
            src1[(i + 1) % n] = src1[i] ^ 0x55;
        }
        
        /* Prevent loop invariant code motion */
        g_volatile_counter++;
    }
    
    /* Final store with side effect */
    dest[n-1] = local_sum;
}

/* Another hot function to encourage inlining */
static inline void process_chunk(int *restrict out, int *in1, int *in2,
                                float *fdata, double *ddata, int start, int end) {
    for (int i = start; i < end; i++) {
        /* Mixed operations */
        int base = in1[i] * 2 + in2[i % 256];
        float f = fdata[i] * 3.14159f;
        double d = ddata[i % 512] * 2.71828;
        
        /* Complex expression with multiple dependencies */
        out[i] = base + (int)(f * 100.0f) + (int)(d * 50.0);
        
        /* Update with carried dependency */
        if (i > 0) {
            out[i] = out[i] + out[i-1] % 1000;
        }
        
        /* More floating-point ops */
        fdata[i] = f * 0.9f;
        ddata[i % 512] = d / (g_volatile_double + 1.0);
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    int *result = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    double *darray = (double*)malloc(SIZE * sizeof(double));
    
    if (!array1 || !array2 || !result || !farray || !darray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 - 2;
        farray[i] = (float)i * 0.1f;
        darray[i] = (double)i * 0.05;
    }
    
    int checksum = 0;
    
    /* Call hot functions multiple times to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        /* Vary parameters slightly each iteration */
        int offset = iter % 64;
        
        /* First hot loop */
        compute_loop(result, array1 + offset, array2, farray, darray, SIZE - offset);
        
        /* Second hot loop with different access pattern */
        process_chunk(array1, array2, result, farray, darray, offset, SIZE);
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < SIZE; i += 128) {
            checksum ^= result[i] + array1[i] + (int)farray[i];
        }
        
        /* Modify inputs for next iteration */
        if (iter % 3 == 0) {
            for (int i = 0; i < SIZE; i += 7) {
                array1[i] = array1[i] * 2 - iter;
                farray[i] = farray[i] + 0.01f;
            }
        }
        
        /* Force memory synchronization */
        g_volatile_counter = iter;
    }
    
    /* Final computation to ensure all code is used */
    int final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        final_result += result[i] % 256;
        final_result ^= array1[i] % 256;
    }
    
    checksum ^= final_result;
    
    printf("Checksum: %d\n", checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(farray);
    free(darray);
    
    return 0;
}
