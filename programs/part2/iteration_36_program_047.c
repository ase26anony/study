/* sel-sched-test.c
 * Program designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-test.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Use volatile to prevent optimization of dependencies */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline void compute_loop(float *restrict arr1, float *arr2, 
                               double *restrict darr, int *restrict iarr,
                               int start, int end, int step) {
    float local_acc = 0.0f;
    double double_acc = 0.0;
    int int_acc = 0;
    
    /* Complex loop with multiple dependencies and operations */
    for (int i = start; i < end; i += step) {
        /* Create carried dependency chain */
        local_acc = local_acc * 1.01f + arr1[i];
        
        /* Independent floating point operation */
        double temp = (double)arr2[i] * 2.5;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and hard to schedule */
            double_acc += temp / (global_seed + 1);
            
            /* Integer operation mixing */
            int_acc ^= (iarr[i] * 3);
            
            /* Memory store with potential aliasing */
            arr2[i] = (float)(double_acc * 0.1);
        } else if (i % 3 == 0) {
            /* Different execution path */
            double_acc -= temp * 0.75;
            int_acc += iarr[i] >> 2;
            
            /* Another store */
            darr[i % 256] = double_acc;
        } else {
            /* Default path */
            double_acc = double_acc * 0.99 + temp;
            int_acc = int_acc * 2 - iarr[i];
        }
        
        /* More arithmetic diversity */
        if (i % 13 == 0) {
            /* Use inline asm to create memory clobber */
            asm volatile("" : : : "memory");
            
            /* Complex expression with mixed types */
            arr1[i] = (float)((local_acc + int_acc) * 0.5);
        }
        
        /* Additional integer operations with carried dependency */
        int_acc = (int_acc * 1103515245 + 12345) & 0x7fffffff;
        
        /* Floating point operation with dependency on previous result */
        local_acc = local_acc + (float)sin((double)i * 0.01);
    }
    
    /* Store results to prevent dead code elimination */
    arr1[0] += local_acc;
    darr[0] += double_acc;
    iarr[0] += int_acc;
}

/* Another hot function to encourage inlining and scheduling */
static inline void process_chunk(float *a, float *b, double *c, int *d,
                                int chunk_size, int offset) {
    for (int j = 0; j < 3; j++) {
        compute_loop(a + offset, b + offset, c, d + offset,
                    0, chunk_size, 1 + (j % 2));
    }
}

int main(void) {
    /* Allocate and initialize arrays with different patterns */
    float *array1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *array2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *array3 = (double*)aligned_alloc(64, 256 * sizeof(double));
    int *array4 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns to create varied dependencies */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (float)(i * 1.5);
        array2[i] = (float)(i * 0.7);
        array4[i] = i * 3;
    }
    
    for (int i = 0; i < 256; i++) {
        array3[i] = i * 2.3;
    }
    
    uint64_t checksum = 0;
    
    /* Main computation - multiple calls to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Vary parameters to prevent constant propagation */
        int chunk = 64 + (iter % 32);
        int offset = (iter * 17) % (ARRAY_SIZE - 128);
        
        /* Call the hot function */
        process_chunk(array1, array2, array3, array4, chunk, offset);
        
        /* Update global seed to create loop-variant behavior */
        global_seed = (global_seed * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Simple checksum to prevent optimization */
        checksum ^= *(uint64_t*)&array1[offset];
        checksum ^= *(uint64_t*)&array2[offset];
        checksum ^= *(uint64_t*)&array3[offset % 256];
        checksum ^= array4[offset];
    }
    
    /* Final reduction to ensure all computations are used */
    float final_float = 0.0f;
    double final_double = 0.0;
    int final_int = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_float += array1[i];
        final_float -= array2[i];
        final_int ^= array4[i];
    }
    
    for (int i = 0; i < 256; i++) {
        final_double += array3[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Checksum: %016llx\n", (unsigned long long)checksum);
    printf("Final float: %f\n", final_float);
    printf("Final double: %f\n", final_double);
    printf("Final int: %d\n", final_int);
    printf("Global seed: %d\n", global_seed);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
