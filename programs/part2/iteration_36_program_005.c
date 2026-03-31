/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx in GCC's selective scheduler
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

/* Function with memory aliasing - both restrict and non-restrict pointers */
static inline double compute_loop(double *restrict arr1, double *arr2, 
                                  float *restrict flt_arr, int *int_arr,
                                  int start, int end, int stride) {
    double sum = 0.0;
    double prod = 1.0;
    float fsum = 0.0f;
    int isum = 0;
    
    /* Complex loop with multiple dependencies and operations */
    for (int i = start; i < end; i += stride) {
        /* Integer operations with carried dependency */
        isum += int_arr[i];
        int_arr[i] = isum ^ global_seed;
        
        /* Floating-point operations */
        double temp = arr1[i] * 1.234567;
        arr1[i] = temp + sin((double)i * 0.01);
        
        /* Mixed-type operations */
        fsum += (float)arr2[i] * 0.5f;
        flt_arr[i] = fsum;
        
        /* More complex FP operations with dependency chain */
        prod *= (arr1[i] + 1.0) / (fabs(arr2[i]) + 1.0);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and hard to schedule */
            sum += prod / (temp + 1.0);
            arr2[i] = sum * 0.987654;
        } else if (i % 13 == 0) {
            /* Another basic block */
            sum -= sqrt(fabs(prod));
            arr2[i] = sum * 1.123456;
        } else {
            /* Default case */
            sum += arr1[i] * arr2[i];
            arr2[i] = sum;
        }
        
        /* Memory operations with potential aliasing */
        if (i > 0) {
            arr1[i] += arr2[i-1] * 0.25;
        }
        
        /* Inline assembly to create memory clobber and prevent optimization */
        asm volatile("" : "+m" (arr1[i]), "+m" (arr2[i]), "+m" (int_arr[i]));
    }
    
    /* Return mixed result to create register pressure */
    return sum + prod + (double)fsum + (double)isum;
}

/* Another hot function to encourage inlining and create more scheduling regions */
static inline double process_chunk(double *restrict a, double *b,
                                   float *restrict f, int *iarr,
                                   int chunk_size, int offset) {
    double total = 0.0;
    
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = offset + chunk * chunk_size;
        int end = start + chunk_size;
        
        /* Call the compute loop multiple times */
        total += compute_loop(a + start, b + start, 
                             f + start, iarr + start,
                             0, chunk_size, 1 + (chunk % 3));
        
        /* Additional operations between loop calls */
        for (int j = 0; j < chunk_size; j += 2) {
            b[start + j] = a[start + j] * total;
            asm volatile("" : "+m" (b[start + j]));
        }
    }
    
    return total;
}

int main(void) {
    /* Allocate and initialize arrays with different patterns */
    double *array1 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *array2 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !float_array || !int_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns to create varied execution paths */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = sin((double)i * 0.1) + 1.0;
        array2[i] = cos((double)i * 0.05) * 2.0;
        float_array[i] = (float)i * 0.25f;
        int_array[i] = i * 3 + global_seed;
    }
    
    double final_result = 0.0;
    uint64_t checksum = 0;
    
    /* Main performance-critical section with multiple hot loops */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Vary parameters to prevent complete loop unrolling */
        int chunk_size = 64 + (iter % 32);
        int offset = (iter * 7) % (ARRAY_SIZE - 256);
        
        /* This is the hot code that should trigger selective scheduling */
        double result = process_chunk(array1 + offset, array2 + offset,
                                     float_array + offset, int_array + offset,
                                     chunk_size, 0);
        
        final_result += result;
        
        /* Update global seed to create loop-carried dependency */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Simple checksum to prevent dead code elimination */
        checksum ^= *(uint64_t*)&result;
    }
    
    /* Use results to prevent optimization */
    printf("Final result: %.12f\n", final_result);
    printf("Checksum: 0x%016lx\n", checksum);
    
    /* Additional computation to ensure arrays are used */
    double verify = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify += array1[i] + array2[i] + float_array[i] + int_array[i];
    }
    printf("Verification sum: %.12f\n", verify);
    
    free(array1);
    free(array2);
    free(float_array);
    free(int_array);
    
    return 0;
}
