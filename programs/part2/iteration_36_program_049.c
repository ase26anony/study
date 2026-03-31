/* sel-sched-coverage.c
 * Program designed to trigger debug_insn_rtx in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with memory aliasing to create complex dependencies */
static inline double compute_value(double* restrict arr1, double* arr2, 
                                   float* farr, int* iarr, int index) {
    /* Mixed operations creating various RTL instructions */
    double temp = arr1[index] * 1.5;
    float ftemp = farr[index] + 0.5f;
    
    /* Integer operations */
    int itemp = iarr[index] + global_seed;
    
    /* Conditional creating basic block boundaries */
    if (index % 7 == 0) {
        temp /= 2.0;
        ftemp *= 1.1f;
        itemp ^= 0x55AA55AA;
    } else if (index % 3 == 0) {
        temp = sqrt(temp);
        ftemp = ftemp - 0.25f;
        itemp = itemp >> 2;
    }
    
    /* More arithmetic diversity */
    temp = temp + sin((double)index * 0.01);
    ftemp = ftemp * cosf((float)index * 0.02f);
    
    /* Potential aliasing between arr1 and arr2 */
    arr2[index] = temp + (double)ftemp;
    
    /* Return mixed result */
    return temp + (double)ftemp + (double)itemp;
}

/* Hot loop with carried dependencies and multiple operations */
static inline void compute_loop(double* restrict out, double* in1, 
                                double* in2, float* fin, int* iin, 
                                int start, int end) {
    double acc1 = 0.0;
    double acc2 = 1.0;
    float facc = 0.0f;
    int iacc = 0;
    
    /* Loop with carried dependencies to force scheduling complexity */
    for (int i = start; i < end; i++) {
        /* Load operations */
        double val1 = in1[i];
        double val2 = in2[i];
        float fval = fin[i];
        int ival = iin[i];
        
        /* Independent floating point operations */
        double t1 = val1 * val2;
        double t2 = val1 / (val2 + 1.0);
        float t3 = fval * 2.0f;
        float t4 = fval / 3.0f;
        
        /* Integer operations mixed in */
        int t5 = ival * 3;
        int t6 = ival + i;
        
        /* Conditional inside loop for control flow */
        if ((i + global_seed) % 11 == 0) {
            t1 = t1 + t2;
            t3 = t3 - t4;
            t5 = t5 ^ t6;
        } else {
            t1 = t1 - t2;
            t3 = t3 + t4;
            t5 = t5 | t6;
        }
        
        /* More arithmetic operations */
        t1 = t1 * 0.95;
        t3 = t3 * 0.9f;
        t5 = t5 & 0xFF;
        
        /* Store operations */
        out[i] = t1 + (double)t3 + (double)t5;
        
        /* Update accumulators with carried dependencies */
        acc1 = acc1 + out[i];
        acc2 = acc2 * (out[i] + 1.0);
        facc = facc + t3;
        iacc = iacc ^ t5;
        
        /* Inline assembly to prevent optimization and create memory clobbers */
        asm volatile("" : "+r"(acc1), "+r"(acc2), "+r"(facc), "+r"(iacc) : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    out[0] = acc1 + acc2 + (double)facc + (double)iacc;
}

/* Another hot function with different pattern */
static inline void transform_array(double* arr, int size, int offset) {
    for (int i = 0; i < size; i++) {
        /* Complex expression with multiple operations */
        double x = arr[i];
        double y = arr[(i + offset) % size];
        
        /* Mixed operations */
        arr[i] = (x * y) + (x / (y + 1.0)) - sin(x * 0.1) * cos(y * 0.1);
        
        /* Additional conditional */
        if (i % 13 == 0) {
            arr[i] = arr[i] * 2.0 - 1.0;
        }
        
        /* Prevent optimization */
        asm volatile("" : "+m"(arr[i]));
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    double* array1 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* array2 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* result = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* farray = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int* iarray = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !result || !farray || !iarray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 0.1;
        array2[i] = (double)(i + 2) * 0.2;
        farray[i] = (float)i * 0.3f;
        iarray[i] = i * 7;
    }
    
    /* Call compute_loop multiple times to create hot region */
    double checksum = 0.0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Update global seed to vary execution */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call the hot loop function */
        compute_loop(result, array1, array2, farray, iarray, 
                    0, ARRAY_SIZE - 1);
        
        /* Call compute_value on some elements */
        for (int i = 0; i < 10; i++) {
            int idx = (iter + i) % ARRAY_SIZE;
            double val = compute_value(array1, array2, farray, iarray, idx);
            checksum += val;
        }
        
        /* Transform arrays */
        transform_array(array1, ARRAY_SIZE, iter % 10 + 1);
        transform_array(array2, ARRAY_SIZE, iter % 5 + 1);
        
        /* Update checksum */
        checksum += result[0] + result[ARRAY_SIZE - 1];
    }
    
    /* Final computation to use all results */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += result[i] + array1[i] + array2[i];
    }
    final_result += checksum;
    
    /* Print result to prevent optimization */
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(result);
    free(farray);
    free(iarray);
    
    return 0;
}
