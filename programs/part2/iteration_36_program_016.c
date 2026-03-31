/* sel-sched-coverage.c
 * Program designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with memory aliasing to create complex dependencies */
static inline double compute_loop(double *restrict arr1, double *arr2, 
                                  int *restrict int_arr, float *float_arr, 
                                  int start, int end, int modifier) {
    double sum = 0.0;
    double product = 1.0;
    int int_sum = 0;
    float float_sum = 0.0f;
    
    /* Create a hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Memory loads with potential aliasing */
        double val1 = arr1[i];
        double val2 = arr2[i % (ARRAY_SIZE/2)];
        int ival = int_arr[i];
        float fval = float_arr[i];
        
        /* Carried dependency chain - prevents parallelization */
        sum = sum + val1 * modifier;
        product = product * (val2 + 1.0);
        
        /* Integer operations with dependency */
        int_sum = int_sum + ival * (i % 13);
        
        /* Floating point operations */
        float_sum = float_sum + fval / (modifier + 1);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in taken branch */
            sum = sum / (fabs(val1) + 1.0);
            product = product * 0.999;
            
            /* Memory store with aliasing */
            arr2[(i * 3) % (ARRAY_SIZE/2)] = sum * 0.5;
        } else if (i % 13 == 0) {
            /* Another basic block */
            int_sum = int_sum ^ (int)(sum * 1000);
            float_sum = float_sum * 0.95f;
        }
        
        /* More arithmetic diversity */
        if (i % 17 == 0) {
            /* Use inline assembly to create memory clobber */
            asm volatile("" : : "r"(arr1), "r"(arr2) : "memory");
        }
        
        /* Additional floating point operation with dependency */
        double temp = sin(val1 * 0.01) * cos(val2 * 0.01);
        sum = sum + temp * 0.1;
        
        /* Integer division - expensive operation */
        if (int_sum != 0) {
            product = product / (fabs(int_sum % 100) + 1.0);
        }
        
        /* Store results back to arrays */
        arr1[i] = sum;
        float_arr[i] = float_sum;
        int_arr[i] = int_sum;
    }
    
    /* Mix results to create return value dependency */
    return sum + product + int_sum + float_sum;
}

/* Another hot function to encourage inlining and create more scheduling regions */
static inline double process_chunk(double *restrict a, double *b, 
                                   int *restrict ia, float *fa, 
                                   int chunk_size, int offset) {
    double total = 0.0;
    
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = offset + chunk * chunk_size;
        int end = start + chunk_size;
        
        /* Call the compute loop multiple times */
        total += compute_loop(a + start, b, ia + start, fa + start, 
                             0, chunk_size, global_seed + chunk);
        
        /* Modify global volatile to prevent optimization */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return total;
}

int main() {
    /* Allocate and initialize arrays with different patterns */
    double *array1 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *array2 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    if (!array1 || !array2 || !int_array || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (i * 1.2345) / (i % 37 + 1);
        array2[i] = sin(i * 0.123) * cos(i * 0.456);
        int_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        float_array[i] = (i * 0.789f) / (i % 23 + 1);
    }
    
    double final_result = 0.0;
    
    /* Main computation - creates hot loop for scheduler */
    for (int iter = 0; iter < ITERATIONS / ARRAY_SIZE + 1; iter++) {
        int chunk_size = ARRAY_SIZE / 8;
        
        /* Process multiple chunks to create scheduling regions */
        for (int chunk = 0; chunk < 8; chunk++) {
            int offset = chunk * chunk_size;
            
            final_result += process_chunk(array1 + offset, 
                                         array2, 
                                         int_array + offset, 
                                         float_array + offset,
                                         chunk_size,
                                         offset);
            
            /* Use volatile variable to prevent dead code elimination */
            global_seed = (global_seed ^ (int)final_result) & 0x7fffffff;
        }
        
        /* Occasionally modify arrays to create varying patterns */
        if (iter % 3 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i += 17) {
                array1[i] *= 1.01;
                array2[i] = cos(array2[i] * 0.5);
            }
        }
    }
    
    /* Create a checksum to ensure computation isn't optimized away */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= *(uint64_t*)&array1[i];
        checksum ^= *(uint64_t*)&array2[i];
        checksum ^= int_array[i];
        checksum ^= *(uint32_t*)&float_array[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Checksum: 0x%016lx\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(int_array);
    free(float_array);
    
    return 0;
}
