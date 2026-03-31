/* test_omp_simt.c - Program to trigger SIMT transformation in omp-low.cc */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Mixed data types and vector operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    /* Runtime condition to control SIMD execution path */
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    /* Arrays with different access patterns */
    float data[SIZE];
    float data2[SIZE];
    int indices[SIZE];
    float result = 0.0f;
    double mixed_result = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        data2[i] = (float)i * 0.25f;
        indices[i] = (i * 3) % SIZE;  /* Non-contiguous pattern */
    }
    
    /* Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* Complex loop with reduction and conditional break */
        float sum = 0.0f;
        float threshold = 500.0f;
        
        /* This loop has data-dependent condition that encourages SIMT transformation */
        #pragma omp simd reduction(+:sum) safelen(16) linear(i:1)
        for (int i = 0; i < SIZE; i++) {
            /* Data-dependent operation that may cause lane divergence */
            if (data[i] > threshold) {
                /* SIMT needs to handle conditional execution */
                data[i] = data[i] * 0.5f;
            } else {
                data[i] = data[i] * 2.0f;
            }
            
            /* Reduction operation */
            sum += data[i];
            
            /* Complex access pattern */
            data2[indices[i]] += data[i] * 0.1f;
        }
        
        result = sum;
        printf("SIMD path result: %f\n", result);
    } else {
        /* Sequential fallback */
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 1.5f;
        }
        printf("Sequential path\n");
    }
    
    /* Unconditional SIMD loop with non-unit stride */
    /* This should always be processed by SIMD lowering */
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous memory access pattern */
        data[i * 2] = data[i * 2] * 3.0f + data2[i];
    }
    
    /* Mixed data types in SIMD loop */
    #pragma omp simd reduction(+:mixed_result)
    for (int i = 0; i < SIZE; i++) {
        /* Mix float and double operations */
        double temp = (double)data[i];
        mixed_result += temp * 0.01;
    }
    
    /* GPU offloading with SIMD - may trigger full SIMT transformation */
    if (use_offload) {
        float offload_data[SIZE];
        
        /* Initialize offload data */
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = (float)i;
        }
        
        /* Target offloading with SIMD - most likely to trigger the uncovered code */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: offload_data[0:SIZE]) \
            reduction(+:mixed_result)
        for (int i = 0; i < SIZE; i++) {
            /* Complex computation with conditional */
            if (offload_data[i] > 100.0f) {
                offload_data[i] = offload_data[i] * 2.0f + 1.0f;
            } else {
                offload_data[i] = offload_data[i] * 0.5f - 1.0f;
            }
            
            /* Reduction with mixed types */
            mixed_result += (double)offload_data[i];
            
            /* Additional complexity with non-linear access */
            if (i % 4 == 0) {
                offload_data[(i + 1) % SIZE] += offload_data[i] * 0.1f;
            }
        }
        
        printf("Offload completed, mixed_result: %lf\n", mixed_result);
    }
    
    /* Nested loops with inner SIMD */
    #pragma omp parallel for
    for (int outer = 0; outer < 4; outer++) {
        /* Inner loop with SIMD - may trigger SIMT transformation */
        #pragma omp simd reduction(+:result)
        for (int inner = 0; inner < SIZE/4; inner++) {
            int idx = outer * (SIZE/4) + inner;
            data[idx] = data[idx] * 1.1f + (float)outer;
            result += data[idx];
        }
    }
    
    /* Vector type operations with OpenMP */
    v4sf vec_data[SIZE/4];
    v4si vec_indices[SIZE/4];
    
    /* Initialize vector arrays */
    for (int i = 0; i < SIZE/4; i++) {
        for (int j = 0; j < 4; j++) {
            vec_data[i][j] = (float)(i * 4 + j);
            vec_indices[i][j] = (i * 4 + j) * 2 % SIZE;
        }
    }
    
    /* SIMD loop with vector types */
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = vec_data[i] * 2.0f;
        
        /* Indirect access using vector indices */
        for (int j = 0; j < 4; j++) {
            data[vec_indices[i][j]] += vec_data[i][j];
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Final results:\n");
    printf("  result: %f\n", result);
    printf("  mixed_result: %lf\n", mixed_result);
    printf("  data[0], data[100], data[1000]: %f, %f, %f\n", 
           data[0], data[100], data[1000]);
    
    return 0;
}
