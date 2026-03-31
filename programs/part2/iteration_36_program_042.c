/* sel-sched-coverage.c
 * Program designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with memory aliasing to create complex dependencies */
static inline void compute_loop(int *restrict arr_a, int *arr_b, 
                                float *restrict farr, double *darr, 
                                int n, int iter) {
    int i;
    float f_acc = 1.0f;
    double d_acc = 0.5;
    int int_acc = iter;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Carried dependency - creates loop-carried dependency */
        int_acc = int_acc * 1103515245 + 12345;
        
        /* Independent integer operations */
        int temp1 = arr_a[i] * 3;
        int temp2 = arr_b[i % 8] + 7;
        
        /* Floating-point operations mixing types */
        f_acc = f_acc * 1.1f + (float)temp1 * 0.01f;
        d_acc = d_acc / 1.01 + (double)temp2 * 0.001;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Different execution path */
            f_acc = f_acc - 0.5f;
            arr_a[i] = (int)(f_acc * 10.0f) + int_acc % 100;
        } else if (i % 13 == 0) {
            /* Another execution path */
            d_acc = d_acc * 1.5;
            arr_b[i % 8] = (int)(d_acc * 20.0) + global_seed;
        } else {
            /* Default path */
            arr_a[i] = temp1 + temp2 + (int)(f_acc + d_acc);
        }
        
        /* Memory operations with potential aliasing */
        arr_b[(i + 1) % 8] = arr_a[i] % 256;
        
        /* More arithmetic diversity */
        if (i % 5 == 0) {
            farr[i % 16] = f_acc * 2.0f;
            darr[i % 16] = d_acc / 2.0;
        }
        
        /* Additional carried dependency */
        global_seed = (global_seed * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Store final accumulators */
    farr[15] = f_acc;
    darr[15] = d_acc;
    arr_b[7] = int_acc;
}

/* Another inline function to increase scheduling complexity */
static inline void process_chunk(int *data, float *fdata, int start, int end) {
    double local_acc = 0.0;
    int i;
    
    for (i = start; i < end; i++) {
        /* Mix of operations */
        int val = data[i];
        float fval = fdata[i % 16];
        
        /* Complex expression with multiple operations */
        data[i] = (val * 3 + (int)(fval * 100.0f)) ^ (i * 7);
        
        /* Floating-point operation chain */
        local_acc += (double)val * 0.01 + (double)fval * 0.001;
        
        /* Conditional with side effect */
        if (val % 11 == 0) {
            fdata[i % 16] = (float)local_acc * 1.1f;
            local_acc = local_acc * 0.9;
        }
    }
    
    /* Store result */
    fdata[0] += (float)local_acc;
}

int main(void) {
    const int ARRAY_SIZE = 256;
    const int ITERATIONS = 1000;
    
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array_b = (int*)malloc(8 * sizeof(int));
    float *farray = (float*)malloc(16 * sizeof(float));
    double *darray = (double*)malloc(16 * sizeof(double));
    
    int i, j;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (i * 13 + 7) % 100;
    }
    
    for (i = 0; i < 8; i++) {
        array_b[i] = (i * 17 + 3) % 50;
    }
    
    for (i = 0; i < 16; i++) {
        farray[i] = (float)i * 0.1f;
        darray[i] = (double)i * 0.05;
    }
    
    /* Perform multiple iterations to create hot loop */
    for (j = 0; j < ITERATIONS; j++) {
        /* Call the inline function multiple times */
        compute_loop(array_a, array_b, farray, darray, ARRAY_SIZE, j);
        
        /* Process in chunks to create different scheduling regions */
        for (i = 0; i < ARRAY_SIZE; i += 64) {
            int end = i + 64;
            if (end > ARRAY_SIZE) end = ARRAY_SIZE;
            process_chunk(array_a, farray, i, end);
        }
        
        /* Modify global seed */
        global_seed = (global_seed + j) * 1103515245;
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array_a[i];
    }
    
    for (i = 0; i < 8; i++) {
        checksum ^= array_b[i];
    }
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(checksum));
    
    printf("Result checksum: %d\n", checksum);
    printf("Global seed: %d\n", global_seed);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(farray);
    free(darray);
    
    return 0;
}
