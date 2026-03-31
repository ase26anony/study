/* Program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Force memory dependencies and prevent optimization */
static volatile int memory_barrier;

/* Inline function with hot loop - designed for selective scheduling */
static inline uint64_t compute_loop(int *restrict arr1, int *arr2, 
                                   float *restrict farr, double *darr,
                                   int start, int end, int seed) {
    uint64_t acc_int = seed;
    float acc_float = seed * 0.5f;
    double acc_double = seed * 0.25;
    
    /* Multiple basic blocks with conditional */
    for (int i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        acc_int = acc_int * 1103515245 + 12345;
        
        /* Floating-point operations - creates FPU instructions */
        acc_float = acc_float * 1.5f + farr[i % SIZE];
        acc_double = acc_double * 0.99 + darr[i % SIZE];
        
        /* Memory operations with potential aliasing */
        int temp1 = arr1[i % SIZE];
        int temp2 = arr2[i % SIZE];
        
        /* Mixed operations */
        arr1[i % SIZE] = temp1 + (int)(acc_float * 100);
        arr2[i % SIZE] = temp2 ^ (int)acc_int;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Different execution path */
            acc_int += temp1 * temp2;
            acc_float /= 1.1f;
            
            /* Inline asm to prevent optimization */
            asm volatile("" : : "r"(temp1), "r"(temp2) : "memory");
        } else if (i % 13 == 0) {
            /* Another basic block */
            acc_double -= temp1 * 0.01;
            memory_barrier = i;
        }
        
        /* Complex expression with multiple dependencies */
        int idx = (i * 17 + acc_int) % SIZE;
        farr[idx] = acc_float * 0.3f + farr[(idx + 1) % SIZE];
        darr[idx] = acc_double * 0.7 + darr[(idx + 31) % SIZE];
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 256 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Combine accumulators */
    return acc_int + (uint64_t)acc_float + (uint64_t)acc_double;
}

/* Another hot function to increase scheduling complexity */
static inline void process_chunk(int *restrict out, const int *in, 
                                float *work, int n, int mod) {
    float facc = 1.0f;
    for (int i = 0; i < n; i++) {
        /* Data-dependent branching */
        if (in[i] & 1) {
            out[i] = in[i] * 3 + (int)(facc * 10);
            facc *= 1.01f;
        } else {
            out[i] = in[i] / 2 - (int)(facc * 5);
            facc *= 0.99f;
        }
        
        /* Memory operations with stride */
        work[(i * 3) % SIZE] = facc;
        
        /* Division operation - creates complex RTL */
        if (i % 19 == 0) {
            out[i] /= (mod + 1);
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with different patterns */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    double *darray = (double*)malloc(SIZE * sizeof(double));
    int *temp_in = (int*)malloc(SIZE * sizeof(int));
    int *temp_out = (int*)malloc(SIZE * sizeof(int));
    float *work = (float*)malloc(SIZE * sizeof(float));
    
    if (!array1 || !array2 || !farray || !darray || 
        !temp_in || !temp_out || !work) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 - 2;
        farray[i] = i * 0.1f;
        darray[i] = i * 0.05;
        temp_in[i] = (i * 7) % 100;
        work[i] = 0.0f;
    }
    
    uint64_t total_checksum = 0;
    
    /* Call the hot function multiple times with different parameters */
    for (int iter = 0; iter < 10; iter++) {
        uint64_t result = compute_loop(array1, array2, farray, darray,
                                      0, ITERATIONS, iter * 1000);
        total_checksum ^= result;
        
        /* Process chunks to create more scheduling regions */
        process_chunk(temp_out, temp_in, work, SIZE, iter);
        
        /* Mix data between arrays to create dependencies */
        for (int i = 0; i < SIZE; i += 8) {
            array1[i] ^= temp_out[i];
            array2[i] += temp_in[i];
        }
        
        /* Prevent optimization across iterations */
        asm volatile("" : : : "memory");
    }
    
    /* Final computation to use all results */
    uint64_t final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i] + array2[i] + (uint64_t)farray[i] + (uint64_t)darray[i];
    }
    
    /* Combine checksums and print to prevent dead code elimination */
    total_checksum ^= final_sum;
    printf("Result checksum: %llu\n", (unsigned long long)total_checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray);
    free(darray);
    free(temp_in);
    free(temp_out);
    free(work);
    
    return 0;
}
