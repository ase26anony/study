/* test_omp_simt.c - Program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Vector type for mixed data type operations */
typedef float v4sf __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    /* Runtime condition to control SIMD execution path */
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    /* Arrays with different access patterns */
    float data[SIZE];
    float strided_data[STRIDE_SIZE * 2];
    int indices[SIZE];
    double mixed_data[SIZE];
    v4sf vector_data[SIZE/4];
    
    /* Reduction variable */
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        indices[i] = (i * 3) % SIZE;
        mixed_data[i] = (double)i * 0.25;
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.1f;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    /* Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* Complex loop with reduction and conditional break */
        #pragma omp simd reduction(+:sum) linear(i:1) aligned(data:32) safelen(16)
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            
            /* Data-dependent condition that may affect SIMD execution */
            if (data[i] > 1000.0f && i > SIZE/2) {
                /* This break creates complexity for SIMD transformation */
                break;
            }
        }
        
        /* Mixed data type operations */
        #pragma omp simd reduction(+:mixed_sum)
        for (int i = 0; i < SIZE; i++) {
            mixed_data[i] = mixed_data[i] * 1.5 + (double)data[i];
            mixed_sum += mixed_data[i];
        }
    } else {
        /* Sequential version */
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
        
        for (int i = 0; i < SIZE; i++) {
            mixed_data[i] = mixed_data[i] * 1.5 + (double)data[i];
            mixed_sum += mixed_data[i];
        }
    }
    
    /* Always present SIMD loop with non-unit stride - ensures SIMD parsing */
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous memory access pattern */
        strided_data[i*2] = strided_data[i*2] * 3.0f + strided_data[i*2 + 1];
    }
    
    /* Indirect indexing pattern */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        int idx = indices[i];
        if (idx < SIZE) {
            data[idx] = data[idx] * 0.5f;
        }
    }
    
    /* Vector type operations combined with OpenMP SIMD */
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = vector_data[i] * 2.0f + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    }
    
    /* GPU offloading section - may trigger SIMT transformation for GPU */
    if (use_offload) {
        float offload_data[SIZE];
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: offload_data[0:SIZE]) reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = data[i] * 3.0f;
            sum += offload_data[i];
            
            /* Complex conditional inside offloaded loop */
            if (i % 16 == 0) {
                offload_data[i] += 10.0f;
            }
        }
        
        /* Nested loops with SIMD on inner loop */
        #pragma omp target teams distribute parallel for collapse(2) \
            map(tofrom: offload_data[0:SIZE])
        for (int i = 0; i < 32; i++) {
            #pragma omp simd
            for (int j = 0; j < SIZE/32; j++) {
                int idx = i * (SIZE/32) + j;
                offload_data[idx] = offload_data[idx] * (i + 1) * 0.1f;
            }
        }
    }
    
    /* Prevent dead code elimination */
    printf("Sum: %f\n", sum);
    printf("Mixed sum: %lf\n", mixed_sum);
    printf("Sample data[0], [100], [500]: %f, %f, %f\n", 
           data[0], data[100], data[500]);
    printf("Sample strided_data[10], [100]: %f, %f\n",
           strided_data[10], strided_data[100]);
    
    return 0;
}
