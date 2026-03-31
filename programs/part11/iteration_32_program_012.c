#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Mixed data types and vector operations
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE 2

int main(int argc, char *argv[]) {
    // Runtime condition for conditional SIMD execution
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare arrays with mixed data types
    float data_f[SIZE * STRIDE];
    double data_d[SIZE];
    int indices[SIZE];
    v4sf vector_data[SIZE/4];
    
    // Initialize arrays
    for (int i = 0; i < SIZE * STRIDE; i++) {
        data_f[i] = (float)(i % 100) * 0.1f;
    }
    for (int i = 0; i < SIZE; i++) {
        data_d[i] = (double)(i % 50) * 0.2;
        indices[i] = (i * 3) % (SIZE * STRIDE);
    }
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    double sum = 0.0;
    float reduction_sum = 0.0f;
    
    // Conditional SIMD execution based on runtime flag
    if (use_simd) {
        // Complex SIMD loop with reduction and data-dependent condition
        #pragma omp simd reduction(+:reduction_sum) linear(i:1) safelen(16)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might affect SIMD execution
            if (data_f[i] > 50.0f && i < SIZE - 1) {
                // This creates complexity for SIMT transformation
                data_f[i+1] *= 0.5f;
            }
            
            // Non-contiguous memory access with stride
            data_f[i * STRIDE] = data_f[i * STRIDE] * 2.0f + 1.0f;
            
            // Reduction operation
            reduction_sum += data_f[i * STRIDE];
            
            // Mixed data type operations
            data_d[i] = (double)data_f[i * STRIDE] * 1.5;
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            data_f[i * STRIDE] = data_f[i * STRIDE] * 2.0f + 1.0f;
            reduction_sum += data_f[i * STRIDE];
            data_d[i] = (double)data_f[i * STRIDE] * 1.5;
        }
    }
    
    // Always present SIMD loop with complex access pattern
    // This ensures SIMD constructs are always parsed
    #pragma omp simd safelen(8) aligned(data_f:32)
    for (int i = 0; i < SIZE/2; i++) {
        // Indirect indexing - complex memory access pattern
        int idx = indices[i];
        if (idx < SIZE * STRIDE) {
            data_f[idx] = data_f[idx] * 3.0f + (float)i * 0.1f;
        }
        
        // Vector operations
        if (i % 4 == 0 && i/4 < SIZE/4) {
            vector_data[i/4] = vector_data[i/4] * 2.0f;
        }
    }
    
    // GPU offloading section - triggers SIMT transformation for GPU
    if (use_offload) {
        float offload_data[SIZE];
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = (float)i;
        }
        
        // Target offloading directive - often uses SIMT model
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: offload_data[0:SIZE]) reduction(+:sum) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < SIZE; i++) {
            // Complex computation with conditional
            if (offload_data[i] > 500.0f) {
                offload_data[i] = offload_data[i] * 0.9f;
            } else {
                offload_data[i] = offload_data[i] * 1.1f + (float)i * 0.01f;
            }
            sum += (double)offload_data[i];
        }
        
        printf("Offload sum: %f\n", sum);
    }
    
    // Nested loops with SIMD on inner loop
    // This creates additional complexity for SIMT transformation
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 16; i++) {
        #pragma omp simd
        for (int j = 0; j < 64; j++) {
            int idx = i * 64 + j;
            if (idx < SIZE) {
                data_f[idx] = data_f[idx] + (float)(i + j) * 0.01f;
            }
        }
    }
    
    // Prevent dead code elimination
    printf("Reduction sum: %f\n", reduction_sum);
    printf("Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data_f[0], data_f[100], data_f[500]);
    printf("SIMD enabled: %d, Offload enabled: %d\n", use_simd, use_offload);
    
    return 0;
}
