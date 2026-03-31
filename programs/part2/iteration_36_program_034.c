/* sel-sched-test.c
 * Program to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-test.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Force memory dependencies and prevent optimization */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict dest, int *src1, int *src2, 
                               float *farr, double *darr, int n) {
    int i;
    int temp_acc = 0;
    float f_acc = 1.0f;
    double d_acc = 2.0;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Integer operations with carried dependency */
        temp_acc += src1[i] * 3;
        
        /* Floating-point operations */
        f_acc = f_acc * 0.99f + farr[i] * 0.01f;
        d_acc = d_acc / 1.01 + darr[i % 8] * 0.5;
        
        /* Memory store with potential aliasing */
        dest[i] = temp_acc + src2[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in taken branch */
            temp_acc -= src1[(i + 1) % n];
            f_acc = f_acc * 1.1f;
        } else if (i % 13 == 0) {
            /* Another basic block */
            d_acc = d_acc - 0.1;
            temp_acc += global_seed; /* Volatile access */
        }
        
        /* More arithmetic diversity */
        if (i % 5 == 0) {
            dest[i] = dest[i] / 2;
        }
        
        /* Complex expression with multiple operations */
        farr[i % 16] = (f_acc + (float)temp_acc) * 0.5f;
    }
    
    /* Prevent dead code elimination */
    dest[0] += (int)f_acc + (int)d_acc;
}

/* Another hot function to encourage inlining */
static inline void process_chunk(int *restrict out, int *in1, int *in2,
                                float *fdata, double *ddata, int start, int end) {
    int i;
    double local_acc = 0.0;
    
    for (i = start; i < end; i++) {
        /* Mixed operations */
        int val = in1[i] ^ in2[i];
        out[i] = val + (i * 3);
        
        /* Floating point with dependency chain */
        local_acc = local_acc * 0.95 + ddata[i % 32] * 0.05;
        
        /* Memory operation with barrier */
        asm volatile("" : : : "memory");
        
        /* Branch with side effect */
        if (val % 11 == 0) {
            fdata[i % 64] = (float)local_acc;
            out[i] += global_seed;
        }
    }
}

int main(void) {
    int i, j;
    int result = 0;
    
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    int *output = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    double *darray = (double*)malloc(32 * sizeof(double));
    
    if (!array1 || !array2 || !output || !farray || !darray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 + 2;
        farray[i] = (float)i * 0.1f;
    }
    
    for (i = 0; i < 32; i++) {
        darray[i] = (double)i * 0.05;
    }
    
    /* Main computation - multiple calls to create scheduling regions */
    for (j = 0; j < ITERATIONS / SIZE + 1; j++) {
        /* Call the hot function multiple times */
        compute_loop(output, array1, array2, farray, darray, SIZE);
        
        /* Process in chunks to create different scheduling contexts */
        for (i = 0; i < SIZE; i += 128) {
            int end = i + 128;
            if (end > SIZE) end = SIZE;
            process_chunk(output + i, array1 + i, array2 + i, 
                         farray, darray, i, end);
        }
        
        /* Modify inputs slightly to prevent complete optimization */
        array1[0] += j;
        array2[0] ^= j;
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < SIZE; i++) {
        result ^= output[i];
        result += array1[i];
        result ^= (int)farray[i];
    }
    
    printf("Result checksum: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(output);
    free(farray);
    free(darray);
    
    return 0;
}
