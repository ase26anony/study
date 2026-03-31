/* test_omp_simt.c - Program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Mixed data types and vector type for complexity */
typedef float v4sf __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    /* Runtime condition to control SIMD execution path */
    int use_simd = argc > 1;  /* SIMD path if command line argument exists */
    int use_gpu_offload = argc > 2; /* GPU offload if second argument exists */
    
    /* Arrays with different access patterns */
    float data[SIZE];
    float data_stride[SIZE * 2];  /* For non-unit stride access */
    int indices[SIZE];
    double mixed_data[SIZE];      /* Mixed data type */
    v4sf vector_data[SIZE/4];     /* Vector type data */
    
    /* Reduction variable */
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        indices[i] = (i * 3) % SIZE;  /* Non-linear indexing */
    }
    
    for (int i = 0; i < SIZE * 2; i++) {
        data_stride[i] = (float)i * 0.05f;
    }
    
    for (int i = 0; i < SIZE; i++) {
        mixed_data[i] = (double)i * 0.2;
    }
    
    /* Conditional SIMD execution based on runtime flag */
    if (use_simd) {
        /* Option 1: GPU offloading with SIMD - most likely to trigger SIMT path */
        if (use_gpu_offload) {
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:SIZE]) map(to: indices[0:SIZE]) \
                reduction(+:sum) safelen(32)
            for (int i = 0; i < SIZE; i++) {
                /* Data-dependent condition inside SIMD loop */
                if (data[i] > 50.0f) {
                    /* Early exit - creates complex control flow */
                    data[i] = 100.0f;
                } else {
                    /* Complex computation with indirect access */
                    int idx = indices[i];
                    data[i] = data[i] * 2.0f + data[idx % SIZE] * 0.5f;
                    sum += data[i];
                    
                    /* Additional condition to encourage SIMT masking */
                    if (i % 8 == 0) {
                        data[i] *= 1.5f;
                    }
                }
            }
        } 
        /* Option 2: Nested loops with inner SIMD */
        else {
            #pragma omp parallel for reduction(+:mixed_sum)
            for (int outer = 0; outer < 4; outer++) {
                /* Inner loop with SIMD and complex clauses */
                #pragma omp simd reduction(+:mixed_sum) linear(outer:1) \
                    safelen(16) aligned(mixed_data:32)
                for (int i = 0; i < SIZE/4; i++) {
                    int actual_idx = outer * (SIZE/4) + i;
                    
                    /* Mixed data type operations */
                    mixed_data[actual_idx] = mixed_data[actual_idx] * 1.1 + 
                                            (double)actual_idx * 0.01;
                    
                    /* Conditional break - creates data dependency */
                    if (mixed_data[actual_idx] > 100.0) {
                        mixed_data[actual_idx] = 100.0;
                    }
                    
                    mixed_sum += mixed_data[actual_idx];
                }
            }
        }
    } else {
        /* Sequential fallback - should not trigger SIMT path */
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
    }
    
    /* Unconditional SIMD loop with non-unit stride access */
    /* This should always be processed by the SIMD/SIMT lowering */
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous memory access pattern */
        data_stride[i * 2] = data_stride[i * 2] * 3.0f + 
                            data_stride[i * 2 + 1] * 0.5f;
        
        /* Additional complexity with conditional */
        if (data_stride[i * 2] > 50.0f) {
            data_stride[i * 2] = 50.0f;
        }
    }
    
    /* Vector type operations with OpenMP */
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        v4sf temp = {1.0f, 2.0f, 3.0f, 4.0f};
        vector_data[i] = vector_data[i] + temp;
    }
    
    /* Nested loop structure with conditional SIMD */
    for (int block = 0; block < 2; block++) {
        int start = block * (SIZE/2);
        int end = start + (SIZE/2);
        
        /* Runtime condition inside loop nest */
        if (use_simd && (block == 0 || argc > 3)) {
            #pragma omp simd reduction(+:sum) linear(start:1)
            for (int i = start; i < end; i++) {
                /* Complex indexing pattern */
                data[i] = data[i] + data[(i + 1) % SIZE] * 0.3f;
                
                /* Data-dependent operation */
                if (data[i] < 0.0f) {
                    data[i] = 0.0f;
                }
                sum += data[i];
            }
        } else {
            for (int i = start; i < end; i++) {
                data[i] = data[i] + data[(i + 1) % SIZE] * 0.3f;
                if (data[i] < 0.0f) {
                    data[i] = 0.0f;
                }
                sum += data[i];
            }
        }
    }
    
    /* Prevent dead code elimination */
    printf("Results: sum = %.2f, mixed_sum = %.2f\n", sum, mixed_sum);
    printf("Sample data[0]=%.2f, data[100]=%.2f, data[500]=%.2f\n", 
           data[0], data[100], data[500]);
    printf("Stride data[10]=%.2f, data_stride[20]=%.2f\n",
           data_stride[10], data_stride[20]);
    
    return 0;
}
