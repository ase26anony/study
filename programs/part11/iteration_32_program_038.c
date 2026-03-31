/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

/* Mixed data types to test SIMT handling */
typedef float v4sf __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    /* Runtime condition for conditional SIMD execution */
    int use_simd = argc > 1;
    int use_gpu_offload = argc > 2;
    
    /* Arrays with different access patterns */
    float data[SIZE];
    float data2[SIZE];
    int indices[SIZE];
    double mixed_data[SIZE];  /* Different type */
    v4sf vector_data[SIZE/4]; /* Vector type */
    
    /* Reduction variable */
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.1f;
        data2[i] = (float)i * 0.2f;
        indices[i] = (i * 3) % SIZE;  /* Non-linear pattern */
        mixed_data[i] = (double)i * 0.05;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    /* Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        printf("Using SIMD path\n");
        
        /* Complex loop with reduction and conditional break - encourages SIMT transformation */
        #pragma omp simd reduction(+:sum) safelen(32) aligned(data:32)
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            
            /* Data-dependent condition inside SIMD loop */
            if (data[i] > 100.0f && i > SIZE/2) {
                /* This break complicates SIMD execution */
                data[i] = 100.0f;
            }
        }
        
        /* Nested loop with inner SIMD */
        #pragma omp parallel for simd collapse(2) reduction(+:mixed_sum)
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < SIZE) {
                    mixed_data[idx] = mixed_data[idx] * 1.5;
                    mixed_sum += mixed_data[idx];
                }
            }
        }
    } else {
        /* Sequential fallback */
        printf("Using sequential path\n");
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
    }
    
    /* GPU offloading with SIMD - likely to trigger SIMT transformation */
    if (use_gpu_offload) {
        printf("Using GPU offload\n");
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data2[0:SIZE]) map(to: indices[0:SIZE]) \
            reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            /* Indirect indexing - complex memory pattern */
            int idx = indices[i];
            data2[idx] = data2[idx] * 3.0f + sinf((float)i * 0.01f);
            sum += data2[idx];
        }
    }
    
    /* Always present SIMD loop with non-unit stride and safelen clause */
    /* This ensures SIMD constructs are always parsed */
    #pragma omp simd safelen(16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous access pattern */
        data[i*2] = data[i*2] * 1.1f;
    }
    
    /* Mixed data type operations in SIMD loop */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Operation mixing float and double */
        data[i] = (float)(data[i] + mixed_data[i] * 0.5);
    }
    
    /* Vector type operations with SIMD */
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = vector_data[i] * 2.0f;
    }
    
    /* Linear clause usage */
    int linear_counter = 0;
    #pragma omp simd linear(linear_counter:1)
    for (int i = 0; i < SIZE; i++) {
        data[i] += linear_counter * 0.01f;
        linear_counter++;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Sum: %f\n", sum);
    printf("Mixed sum: %lf\n", mixed_sum);
    printf("Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data[0], data[100], data[500]);
    
    return 0;
}
