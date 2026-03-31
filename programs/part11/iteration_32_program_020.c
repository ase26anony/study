/* test_omp_simt.c - Program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Define a vector type for mixed data type operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define ARRAY_SIZE 1024
#define STRIDE_SIZE 2

int main(int argc, char *argv[]) {
    /* Runtime condition to control SIMD execution path */
    int use_simd = argc > 1;
    int use_gpu_offload = argc > 2;
    
    /* Declare and initialize arrays with mixed patterns */
    float data[ARRAY_SIZE];
    float data2[ARRAY_SIZE * STRIDE_SIZE];
    int indices[ARRAY_SIZE];
    double mixed_data[ARRAY_SIZE];
    float reduction_sum = 0.0f;
    double double_sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i * 0.1f;
        indices[i] = (i * 3) % ARRAY_SIZE;
        mixed_data[i] = (double)i * 0.05;
    }
    
    for (int i = 0; i < ARRAY_SIZE * STRIDE_SIZE; i++) {
        data2[i] = (float)i * 0.2f;
    }
    
    /* Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* Complex reduction with data-dependent condition */
        #pragma omp simd reduction(+:reduction_sum) linear(i:1) safelen(32)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Data-dependent condition inside SIMD loop */
            if (data[i] > 50.0f) {
                /* Early exit condition - complex for SIMT */
                if (i > ARRAY_SIZE/2) break;
            }
            
            /* Mixed data type operations */
            double temp = (double)data[i] + mixed_data[i];
            data[i] = (float)temp * 2.0f + 1.0f;
            reduction_sum += data[i];
            
            /* Non-contiguous memory access */
            if (i % STRIDE_SIZE == 0) {
                data2[i * STRIDE_SIZE] = data[i] * 3.0f;
            }
        }
        
        /* Nested loops with inner SIMD */
        #pragma omp parallel for simd collapse(2) reduction(+:double_sum)
        for (int i = 0; i < ARRAY_SIZE/16; i++) {
            for (int j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                mixed_data[idx] = mixed_data[idx] * 1.5 + sin((double)idx * 0.01);
                double_sum += mixed_data[idx];
            }
        }
    } else {
        /* Sequential fallback path */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (data[i] > 50.0f && i > ARRAY_SIZE/2) break;
            double temp = (double)data[i] + mixed_data[i];
            data[i] = (float)temp * 2.0f + 1.0f;
            reduction_sum += data[i];
            if (i % STRIDE_SIZE == 0) {
                data2[i * STRIDE_SIZE] = data[i] * 3.0f;
            }
        }
    }
    
    /* GPU offloading with SIMD - triggers SIMT transformation */
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:ARRAY_SIZE]) \
            map(to: indices[0:ARRAY_SIZE]) \
            reduction(+:reduction_sum) \
            safelen(64)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Indirect indexing - complex memory pattern */
            int idx = indices[i];
            data[idx] = data[idx] * 2.5f + sinf((float)idx * 0.01f);
            
            /* Conditional execution within SIMD */
            if (data[idx] < 100.0f) {
                data[idx] = sqrtf(data[idx]);
            }
            reduction_sum += data[idx];
        }
    }
    
    /* Unconditional SIMD loop with non-unit stride and safelen clause */
    /* This ensures SIMD constructs are always parsed */
    #pragma omp simd safelen(16) aligned(data:16)
    for (int i = 0; i < ARRAY_SIZE/STRIDE_SIZE; i++) {
        /* Non-unit stride access */
        data[i * STRIDE_SIZE] = data[i * STRIDE_SIZE] * 2.0f;
        
        /* Vector type operations */
        if (i % 4 == 0 && i + 4 < ARRAY_SIZE/STRIDE_SIZE) {
            v4sf* vptr = (v4sf*)&data[i * STRIDE_SIZE];
            v4sf multiplier = {1.1f, 1.2f, 1.3f, 1.4f};
            *vptr = *vptr * multiplier;
        }
    }
    
    /* Another complex SIMD loop with early exit possibility */
    float threshold = 75.0f;
    int early_exit_count = 0;
    
    #pragma omp simd reduction(+:early_exit_count) linear(i:1)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Data-dependent computation that might trigger SIMT conditional */
        data[i] = data[i] + mixed_data[i];
        
        if (data[i] > threshold) {
            early_exit_count++;
            /* Complex condition that varies per lane */
            data[i] = data[i] / (1.0f + (float)early_exit_count * 0.1f);
        }
        
        /* Cross-lane dependency simulation */
        if (i > 0) {
            data[i] = data[i] + data[i-1] * 0.01f;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("Reduction sum: %f\n", reduction_sum);
    printf("Double sum: %lf\n", double_sum);
    printf("Early exit count: %d\n", early_exit_count);
    printf("Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data[0], data[100], data[500]);
    printf("Sample data2[10]: %f\n", data2[10]);
    
    return 0;
}
