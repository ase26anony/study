#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Mixed data types and vector operations
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    // Runtime condition for conditional SIMD execution
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare arrays with different access patterns
    float data[SIZE];
    float data_stride[SIZE * 2];  // For non-contiguous access
    int indices[SIZE];
    double mixed_data[SIZE];      // Mixed data type
    v4sf vector_data[SIZE/4];     // Explicit vector type
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        indices[i] = (i * 3) % SIZE;  // Non-linear indexing
    }
    
    for (int i = 0; i < SIZE * 2; i++) {
        data_stride[i] = (float)i * 0.25f;
    }
    
    for (int i = 0; i < SIZE; i++) {
        mixed_data[i] = (double)i * 0.75;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    // Conditional SIMD execution based on runtime flag
    if (use_simd) {
        // Complex loop with data-dependent condition and reduction
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(16)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might affect SIMD execution
            if (data[i] > 250.0f) {
                // This could affect lane masking in SIMT
                data[i] = data[i] * 0.5f;
            }
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            
            // Non-contiguous memory access pattern
            if (i % 2 == 0) {
                data_stride[i*2] = data[i] * 3.0f;
            }
        }
        
        // Nested loop with inner SIMD
        #pragma omp parallel for simd collapse(2) reduction(+:mixed_sum)
        for (int i = 0; i < SIZE/16; i++) {
            for (int j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                mixed_data[idx] = mixed_data[idx] * 1.5 + 0.5;
                mixed_sum += mixed_data[idx];
            }
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            if (data[i] > 250.0f) {
                data[i] = data[i] * 0.5f;
            }
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            
            if (i % 2 == 0) {
                data_stride[i*2] = data[i] * 3.0f;
            }
        }
        
        for (int i = 0; i < SIZE; i++) {
            mixed_data[i] = mixed_data[i] * 1.5 + 0.5;
            mixed_sum += mixed_data[i];
        }
    }
    
    // Unconditional SIMD loop with complex access pattern
    // This should always be processed by the SIMT lowering
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        data_stride[i*2] = data_stride[i*2] * 3.0f + data_stride[i*2 + 1];
        
        // Indirect indexing - complex pattern for SIMT
        int idx = indices[i];
        if (idx < SIZE) {
            data[idx] += data_stride[i*2] * 0.1f;
        }
    }
    
    // GPU offloading section - triggers SIMT for GPU targets
    if (use_offload) {
        float offload_sum = 0.0f;
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(tofrom: offload_sum) \
            reduction(+:offload_sum)
        for (int i = 0; i < SIZE; i++) {
            // Complex computation with conditional
            if (data[i] < 500.0f) {
                data[i] = data[i] * 1.8f - 0.3f;
            } else {
                data[i] = data[i] * 0.7f + 0.9f;
            }
            offload_sum += data[i];
            
            // Additional operation with stride
            if (i % 4 == 0) {
                data_stride[i] = data[i] * 0.5f;
            }
        }
        
        sum += offload_sum;
    }
    
    // Vector type operations with SIMD
    v4sf vec_sum = {0.0f, 0.0f, 0.0f, 0.0f};
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = vector_data[i] * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        vec_sum += vector_data[i];
    }
    
    // Print results to prevent dead code elimination
    printf("Results:\n");
    printf("  Sum: %.2f\n", sum);
    printf("  Mixed sum: %.2f\n", mixed_sum);
    printf("  Vector sum: %.2f, %.2f, %.2f, %.2f\n", 
           vec_sum[0], vec_sum[1], vec_sum[2], vec_sum[3]);
    printf("  Sample data[0], [100], [500]: %.2f, %.2f, %.2f\n",
           data[0], data[100], data[500]);
    printf("  Sample stride[0], [200]: %.2f, %.2f\n",
           data_stride[0], data_stride[200]);
    
    return 0;
}
