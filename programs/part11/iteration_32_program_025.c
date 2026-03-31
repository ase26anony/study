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
    float data2[SIZE];
    int indices[SIZE];
    float result[SIZE/2];  // For strided access
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        data2[i] = (float)i * 0.25f;
        indices[i] = (i * 3) % SIZE;  // Non-linear indexing
    }
    
    float sum = 0.0f;
    float threshold = 250.0f;
    
    // Conditional SIMD execution based on runtime flag
    if (use_simd) {
        printf("Using SIMD execution path\n");
        
        if (use_gpu_offload) {
            // GPU offloading with SIMD - likely to trigger SIMT transformation
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:SIZE]) map(to: data2[0:SIZE]) \
                reduction(+:sum) if(target: use_gpu_offload)
            for (int i = 0; i < SIZE; i++) {
                // Data-dependent condition inside SIMD loop
                if (data[i] > threshold) {
                    // Early exit - creates complex control flow
                    data[i] = threshold;
                }
                data[i] = data[i] * 2.0f + data2[i];
                sum += data[i];
            }
        } else {
            // CPU SIMD with complex access pattern
            #pragma omp simd reduction(+:sum) safelen(16) linear(i:1)
            for (int i = 0; i < SIZE; i++) {
                // Indirect indexing - non-contiguous access
                int idx = indices[i];
                data[idx] = data[idx] * 3.0f - data2[i];
                
                // Conditional break - creates data dependency
                if (i > 0 && data[idx] > threshold * 2) {
                    // This may cause the compiler to generate SIMT conditional wrapper
                    data[idx] = threshold * 2;
                }
                sum += data[idx];
            }
        }
    } else {
        // Sequential fallback
        printf("Using sequential execution\n");
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
    }
    
    // Unconditional SIMD loop with non-unit stride and safelen clause
    // This ensures SIMD constructs are always parsed
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        result[i] = data[i * 2] * data2[i * 2 + 1];
        
        // Mixed operations that might require lane masking
        if (result[i] > 100.0f) {
            result[i] = 100.0f;
        }
    }
    
    // Nested loops with SIMD on inner loop
    // This creates more complex lowering scenarios
    float matrix[64][64];
    #pragma omp parallel for
    for (int i = 0; i < 64; i++) {
        #pragma omp simd aligned(matrix:64) linear(j:1)
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (float)(i * j) * 0.01f;
        }
    }
    
    // Use vector types with OpenMP SIMD
    v4sf vec_data[SIZE/4];
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        v4sf v1 = {data[i*4], data[i*4+1], data[i*4+2], data[i*4+3]};
        v4sf v2 = {0.1f, 0.2f, 0.3f, 0.4f};
        vec_data[i] = v1 * v2 + (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
    }
    
    // Print results to prevent dead code elimination
    printf("Sum: %f\n", sum);
    printf("Sample data[0]: %f, data[100]: %f\n", data[0], data[100]);
    printf("Sample result[10]: %f\n", result[10]);
    
    // Additional computation to ensure all paths are used
    float final_check = 0.0f;
    #pragma omp simd reduction(+:final_check)
    for (int i = 0; i < SIZE; i += 4) {
        // Strided access with conditional
        if (i % 8 == 0) {
            final_check += data[i] * 2.0f;
        } else {
            final_check += data[i] * 0.5f;
        }
    }
    
    printf("Final check: %f\n", final_check);
    
    return 0;
}
