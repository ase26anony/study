#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to complicate vectorization
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    // Runtime condition for conditional SIMD execution
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare arrays with different types and sizes
    float data[SIZE];
    double dbl_data[SIZE];
    int indices[SIZE];
    float strided_data[STRIDE_SIZE * 2]; // For non-unit stride access
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        dbl_data[i] = (double)i * 0.25;
        indices[i] = (i * 3) % SIZE; // Non-linear indexing
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.1f;
    }
    
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    // Conditional SIMD execution based on runtime flag
    if (use_simd) {
        // Complex SIMD loop with reduction and data-dependent condition
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(16)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might affect SIMD execution
            if (data[i] > 250.0f && i < SIZE - 1) {
                // Early exit condition - complicates SIMD vectorization
                data[i+1] = data[i] * 0.5f;
            }
            sum += data[i] * 2.0f;
            
            // Mixed type operation
            dbl_data[i] = (double)data[i] * 1.5;
        }
        
        // Nested loop with inner SIMD
        #pragma omp parallel for simd collapse(2) reduction(+:dbl_sum)
        for (int i = 0; i < SIZE/32; i++) {
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                dbl_sum += dbl_data[idx] * (j % 2 == 0 ? 1.0 : -1.0);
            }
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            sum += data[i] * 2.0f;
            dbl_data[i] = (double)data[i] * 1.5;
        }
        for (int i = 0; i < SIZE; i++) {
            dbl_sum += dbl_data[i] * (i % 2 == 0 ? 1.0 : -1.0);
        }
    }
    
    // Unconditional SIMD loop with non-unit stride and safelen clause
    // This should always be processed for SIMD transformation
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        strided_data[i * 2] = strided_data[i * 2] * 3.0f + 
                             strided_data[(i * 2 + 1) % (STRIDE_SIZE * 2)] * 0.5f;
    }
    
    // GPU offloading section - triggers SIMT path for GPU execution
    if (use_offload) {
        float gpu_sum = 0.0f;
        
        // Target offloading with SIMD - likely to trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(tofrom: gpu_sum) \
            reduction(+:gpu_sum)
        for (int i = 0; i < SIZE; i++) {
            // Indirect indexing complicates memory access pattern
            int idx = indices[i];
            data[idx] = data[idx] * 2.0f + (float)i * 0.01f;
            gpu_sum += data[idx];
            
            // Conditional operation inside SIMD loop
            if (data[idx] > 100.0f) {
                data[idx] = sqrtf(data[idx]); // Math function call
            }
        }
        
        printf("GPU offload sum: %f\n", gpu_sum);
    }
    
    // Additional complex SIMD construct with vector types
    v4sf vec_sum = {0.0f, 0.0f, 0.0f, 0.0f};
    #pragma omp simd reduction(+:vec_sum)
    for (int i = 0; i < SIZE/4; i++) {
        v4sf vec_data = *(v4sf*)&data[i*4];
        vec_sum += vec_data * 1.5f;
    }
    
    // Print results to prevent dead code elimination
    printf("Results:\n");
    printf("  Sum: %f\n", sum);
    printf("  Double sum: %lf\n", dbl_sum);
    printf("  Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data[0], data[100], data[500]);
    printf("  Strided sample: %f\n", strided_data[10]);
    
    // Compute and print vector sum
    float total_vec_sum = vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    printf("  Vector sum: %f\n", total_vec_sum);
    
    return 0;
}
