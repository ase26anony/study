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
    float result[SIZE/2]; // For non-contiguous access
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        data2[i] = (float)i * 0.25f;
        indices[i] = (i * 3) % SIZE; // Non-linear indexing
    }
    
    float sum = 0.0f;
    float simd_sum = 0.0f;
    
    // Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex loop with reduction and conditional break
        #pragma omp simd reduction(+:simd_sum) safelen(16) aligned(data:16)
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            simd_sum += data[i];
            
            // Data-dependent condition that might affect SIMT execution
            if (data[i] > 1000.0f && i > SIZE/2) {
                // This could influence SIMT lane masking
                data[i] = 1000.0f;
            }
        }
        
        // Nested loop with inner SIMD
        #pragma omp parallel for simd collapse(2) reduction(+:simd_sum)
        for (int i = 0; i < SIZE/16; i++) {
            for (int j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                data2[idx] = data[idx] + data2[idx] * 0.5f;
                simd_sum += data2[idx];
            }
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            simd_sum += data[i];
        }
    }
    
    // Always present SIMD loop with non-unit stride - ensures SIMD constructs are parsed
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-contiguous memory access pattern
        result[i] = data[i*2] * 3.0f + data2[i*2 + 1];
        
        // Mixed operations that might require lane masking
        if (result[i] > 500.0f) {
            result[i] = 500.0f;
        }
    }
    
    // GPU offloading section - often uses SIMT model
    if (use_gpu_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(to: data2[0:SIZE]) \
            reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            float temp = data[i] * data2[i];
            data[i] = temp + (float)i * 0.1f;
            sum += data[i];
            
            // Complex conditional inside SIMD loop
            if (indices[i] % 4 == 0) {
                data[i] *= 0.9f;
            }
        }
        
        // Another offloaded loop with indirect indexing
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(to: indices[0:SIZE])
        for (int i = 0; i < SIZE/2; i++) {
            int idx = indices[i];
            data[idx] = data[idx] * 1.1f - (float)idx * 0.01f;
        }
    }
    
    // Use vector types with OpenMP SIMD
    v4sf vec_data[SIZE/4];
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = vec_data[i] * 2.0f + 1.0f;
    }
    
    // Linear clause usage
    int linear_counter = 0;
    #pragma omp simd linear(linear_counter:1)
    for (int i = 0; i < SIZE; i++) {
        data[i] += (float)linear_counter * 0.01f;
        linear_counter++;
    }
    
    // Prevent dead code elimination
    printf("Results: sum=%.2f, simd_sum=%.2f\n", sum, simd_sum);
    printf("Sample data[0]=%.2f, data[100]=%.2f, result[50]=%.2f\n", 
           data[0], data[100], result[50]);
    
    // Verify with a small check
    float verify_sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        verify_sum += data[i] + result[i/2];
    }
    printf("Verification sum: %.2f\n", verify_sum);
    
    return 0;
}
