#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Mixed data types and vector operations
typedef float v4sf __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE 2

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare and initialize arrays with mixed patterns
    float data[SIZE * STRIDE];
    float data2[SIZE];
    int indices[SIZE];
    double double_data[SIZE];
    v4sf vector_data[SIZE/4];
    
    // Initialize arrays
    for (int i = 0; i < SIZE * STRIDE; i++) {
        data[i] = (float)(i % 100) * 0.1f;
    }
    for (int i = 0; i < SIZE; i++) {
        data2[i] = (float)i * 0.5f;
        indices[i] = (i * 3) % SIZE;
        double_data[i] = (double)i * 0.25;
    }
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    float sum = 0.0f;
    double double_sum = 0.0;
    
    // Conditional SIMD execution based on runtime flag
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp simd reduction(+:sum) safelen(16) linear(i:1)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might cause early exit
            if (data2[i] > 50.0f && i > SIZE/2) {
                // This could affect SIMD lane execution
                sum += 10.0f;
            } else {
                sum += data2[i];
            }
            
            // Non-contiguous memory access with stride
            data[i * STRIDE] = data[i * STRIDE] * 2.0f + 1.0f;
        }
        
        // Mixed data type operations
        #pragma omp simd reduction(+:double_sum)
        for (int i = 0; i < SIZE; i++) {
            double_data[i] = double_data[i] * 1.5 + data[i] * 0.01;
            double_sum += double_data[i];
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            if (data2[i] > 50.0f && i > SIZE/2) {
                sum += 10.0f;
            } else {
                sum += data2[i];
            }
            data[i * STRIDE] = data[i * STRIDE] * 2.0f + 1.0f;
        }
        for (int i = 0; i < SIZE; i++) {
            double_data[i] = double_data[i] * 1.5 + data[i] * 0.01;
            double_sum += double_data[i];
        }
    }
    
    // Unconditional SIMD loop with complex access pattern
    // This should always be processed by the SIMT lowering
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < SIZE; i++) {
        // Indirect indexing - creates complex memory access pattern
        int idx = indices[i];
        data2[idx] = data2[idx] * 3.0f - data[i * STRIDE] * 0.5f;
    }
    
    // Vector type operations with SIMD
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = vector_data[i] * 2.0f + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    }
    
    // GPU offloading section - triggers different SIMT path
    if (use_offload) {
        float offload_data[SIZE];
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = (float)i;
        }
        
        // Target offloading directive - often uses SIMT model
        #pragma omp target teams distribute parallel for simd \
            map(tofrom:offload_data[0:SIZE]) reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = offload_data[i] * 2.0f + 1.0f;
            if (offload_data[i] > 100.0f) {
                sum += offload_data[i];
            }
        }
        
        // Nested loops with SIMD on inner loop
        #pragma omp target teams distribute parallel for collapse(2) \
            map(tofrom:offload_data[0:SIZE])
        for (int i = 0; i < 32; i++) {
            #pragma omp simd
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < SIZE) {
                    offload_data[idx] = offload_data[idx] * (i + 1) * 0.1f;
                }
            }
        }
    }
    
    // Prevent dead code elimination
    printf("Results: sum = %f, double_sum = %f\n", sum, double_sum);
    printf("Sample data[0] = %f, data2[100] = %f\n", data[0], data2[100]);
    printf("Vector sample: %f, %f, %f, %f\n", 
           vector_data[0][0], vector_data[0][1], 
           vector_data[0][2], vector_data[0][3]);
    
    return 0;
}
