#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Vector type for mixed data operations
typedef float v4sf __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_gpu_offload = argc > 2;
    
    // Arrays with different access patterns
    float data[SIZE];
    float data_stride[SIZE * 2];  // For non-unit stride access
    int indices[SIZE];
    double mixed_data[SIZE];  // Different type for mixed operations
    v4sf vector_data[SIZE/4];  // Explicit vector type
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
        indices[i] = (i * 3) % SIZE;
    }
    
    for (int i = 0; i < SIZE * 2; i++) {
        data_stride[i] = (float)i * 0.5f;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*4.0f, i*4.0f+1.0f, i*4.0f+2.0f, i*4.0f+3.0f};
    }
    
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        printf("Using SIMD path\n");
        
        if (use_gpu_offload) {
            // GPU offloading with SIMD - likely to trigger SIMT transformation
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:SIZE]) map(to: indices[0:SIZE]) \
                reduction(+:sum) if(target: use_gpu_offload)
            for (int i = 0; i < SIZE; i++) {
                // Data-dependent condition inside SIMD loop
                if (data[i] > 500.0f) {
                    // Early exit - creates complex control flow
                    data[i] = data[i] * 0.5f;
                } else {
                    data[i] = data[i] * 2.0f + 1.0f;
                }
                
                // Reduction with conditional
                if (i % 2 == 0) {
                    sum += data[i];
                }
                
                // Non-contiguous memory access
                data_stride[i*2] = data_stride[i*2] * 3.0f;
            }
        } else {
            // CPU SIMD with complex clauses
            #pragma omp simd reduction(+:sum) linear(i:1) safelen(16) \
                aligned(data:16) if(simd: use_simd)
            for (int i = 0; i < SIZE; i++) {
                // Mixed data type operations
                mixed_data[i] = (double)data[i] * 1.5;
                mixed_sum += mixed_data[i];
                
                // Indirect indexing
                data[indices[i]] = data[indices[i]] * 1.1f;
                
                // Conditional reduction
                if (data[i] < 200.0f) {
                    sum += data[i];
                }
            }
        }
    } else {
        // Sequential fallback
        printf("Using sequential path\n");
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
    }
    
    // Unconditional SIMD loop with non-unit stride - always present
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        data_stride[i*2] = data_stride[i*2] * 2.0f + data_stride[i*2 + 1];
    }
    
    // Nested loops with SIMD on inner loop
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 64; j++) {
            int idx = i * 64 + j;
            #pragma omp simd reduction(+:mixed_sum)
            for (int k = 0; k < 4; k++) {
                // Mixed operations with vector types
                vector_data[idx/4][k] = vector_data[idx/4][k] * 1.2f;
                mixed_sum += vector_data[idx/4][k];
            }
        }
    }
    
    // Prevent dead code elimination
    printf("Sum: %f\n", sum);
    printf("Mixed sum: %lf\n", mixed_sum);
    printf("Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data[0], data[100], data[500]);
    printf("Sample stride[0]: %f, stride[10]: %f\n", 
           data_stride[0], data_stride[10]);
    
    return 0;
}
