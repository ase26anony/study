/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx() in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with memory aliasing - restrict and non-restrict pointers */
static inline void compute_loop(int *restrict dest, int *src1, float *src2, 
                                double *src3, int n, int start_val) {
    int i;
    int acc_int = start_val;
    float acc_float = start_val * 0.5f;
    double acc_double = start_val * 0.25;
    
    /* Complex loop with multiple dependencies and basic blocks */
    for (i = 0; i < n; i++) {
        /* Integer operations with carried dependency */
        acc_int = acc_int * 1103515245 + 12345;
        
        /* Floating-point operations */
        acc_float = acc_float * 1.5f + src2[i % 8];
        
        /* Double precision operations */
        acc_double = acc_double / 1.7 + src3[i % 8] * 0.3;
        
        /* Memory operations with potential aliasing */
        int load1 = src1[i];
        int load2 = src1[(i + 1) % n];
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Branch taken path - more complex operations */
            acc_int += (load1 * load2) / 3;
            acc_float = acc_float - (load1 * 0.01f);
            
            /* Inline asm with memory clobber to prevent reordering */
            asm volatile("" : : : "memory");
        } else {
            /* Branch not taken path - different operations */
            acc_int -= load1 >> 2;
            acc_double += load2 * 0.001;
            
            /* Mix of operations to create diverse RTL */
            if (i % 13 == 0) {
                acc_float = acc_float * 2.0f - 1.0f;
            }
        }
        
        /* Another conditional to increase control flow complexity */
        if (i % 11 == 0 && acc_int > 1000) {
            acc_int = acc_int % 1000;
            acc_double = acc_double * 0.9;
        }
        
        /* Store result with potential aliasing concern */
        dest[i] = acc_int + (int)acc_float + (int)acc_double;
        
        /* Additional arithmetic to increase instruction count */
        int temp = (dest[i] * 3) / 2;
        if (temp > 0) {
            dest[i] = temp & 0xFF;
        }
        
        /* Use volatile global to create external dependency */
        dest[i] ^= global_seed;
    }
    
    /* Final store to create loop-carried dependency */
    if (n > 0) {
        dest[0] = acc_int;
    }
}

/* Second hot function with different operation mix */
static inline void compute_loop2(double *restrict dest, int *src1, 
                                 float *src2, int n, int modifier) {
    int i;
    double acc1 = modifier * 1.1;
    float acc2 = modifier * 0.7f;
    
    for (i = 0; i < n; i++) {
        /* Different operation mix to trigger different RTL patterns */
        acc1 = acc1 * 1.01 + src1[i % 16] * 0.5;
        acc2 = acc2 - src2[i % 16] * 0.3f;
        
        /* Complex conditional with arithmetic */
        if ((i ^ modifier) % 5 == 0) {
            acc1 = acc1 / (1.0 + (i % 10));
            acc2 = acc2 + (i % 7) * 0.1f;
            
            /* Memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Integer division - expensive operation for scheduler */
        int div_result = (i + modifier) / (src1[i % 16] + 1);
        
        /* Store with type conversion */
        dest[i] = acc1 + acc2 + div_result;
        
        /* Periodic reset to prevent overflow */
        if (i % 97 == 0) {
            acc1 = acc1 * 0.5;
            acc2 = acc2 * 0.5f;
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with different patterns */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *array3 = (float*)malloc(16 * sizeof(float));
    double *array4 = (double*)malloc(16 * sizeof(double));
    double *array5 = (double*)malloc(SIZE * sizeof(double));
    
    if (!array1 || !array2 || !array3 || !array4 || !array5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFF;
        array2[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    
    for (int i = 0; i < 16; i++) {
        array3[i] = (i * 0.1f) - 0.5f;
        array4[i] = (i * 0.05) + 0.25;
    }
    
    int checksum = 0;
    
    /* Call hot functions multiple times to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different loop configurations */
        if (iter % 3 == 0) {
            compute_loop(array2, array1, array3, array4, SIZE, iter);
        } else if (iter % 3 == 1) {
            compute_loop2(array5, array1, array3, SIZE, iter);
        } else {
            /* Third variant with different parameters */
            compute_loop(array1, array2, array3, array4, SIZE / 2, iter * 2);
        }
        
        /* Update checksum to prevent dead code elimination */
        checksum ^= array1[iter % SIZE];
        checksum ^= array2[iter % SIZE];
        
        /* Modify global volatile to affect loop dependencies */
        global_seed = (global_seed * 1664525 + 1013904223) & 0xFFFF;
    }
    
    /* Final computation to use all results */
    int final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        final_result += array1[i] + array2[i] + (int)array5[i % SIZE];
        final_result = (final_result * 13 + 7) & 0xFFFFFF;
    }
    
    final_result ^= checksum;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}
