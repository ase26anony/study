#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to test SIMT handling
typedef float v4sf __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_gpu_offload = argc > 2;
    
    // Arrays with different access patterns
    float data[SIZE];
    float data_stride[STRIDE_SIZE * 2]; // For non-unit stride access
    int indices[SIZE];
    double mixed_data[SIZE]; // Mixed data type
    v4sf vector_data[SIZE/4]; // Explicit vector type
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
        indices[i] = (i * 3) % SIZE; // Non-linear indexing
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        data_stride[i] = (float)i * 0.5f;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    // Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex loop with reduction and data-dependent condition
        #pragma omp simd reduction(+:sum) linear(i:1) aligned(data:16) safelen(32)
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            
            // Data-dependent condition inside SIMD loop
            if (data[i] > 1000.0f && i > SIZE/2) {
                // This creates complexity for SIMT transformation
                data[i] = data[i] * 0.5f;
            }
        }
        
        // Nested loop with inner SIMD
        if (use_gpu_offload) {
            // GPU offloading with SIMD - likely to trigger SIMT path
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:SIZE]) map(tofrom: mixed_data[0:SIZE]) \
                reduction(+:mixed_sum) collapse(2)
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < SIZE/16; j++) {
                    int idx = i * (SIZE/16) + j;
                    mixed_data[idx] = data[idx] * 1.5;
                    mixed_sum += mixed_data[idx];
                    
                    // Indirect indexing adds complexity
                    data[indices[idx]] += 0.1f;
                }
            }
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
    }
    
    // Unconditional SIMD loop with non-unit stride and safelen clause
    // This ensures SIMD constructs are always parsed
    #pragma omp simd safelen(16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        data_stride[i*2] = data_stride[i*2] * 3.0f + data_stride[i*2 + 1];
    }
    
    // Another SIMD loop with mixed data types
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        // Operation mixing float and double
        data[i] = (float)(data[i] + mixed_data[i] * 0.5);
    }
    
    // Vector type operations with SIMD
    v4sf vec_sum = {0.0f, 0.0f, 0.0f, 0.0f};
    #pragma omp simd reduction(+:vec_sum)
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = vector_data[i] * 2.0f;
        vec_sum += vector_data[i];
    }
    
    // Print results to prevent dead code elimination
    printf("Sum: %f\n", sum);
    printf("Mixed sum: %lf\n", mixed_sum);
    printf("Vector sum: %f %f %f %f\n", 
           vec_sum[0], vec_sum[1], vec_sum[2], vec_sum[3]);
    printf("Sample data[0], [100], [500]: %f %f %f\n", 
           data[0], data[100], data[500]);
    
    // Additional complexity: SIMD with early exit simulation
    int found = 0;
    float threshold = 500.0f;
    
    #pragma omp simd reduction(||:found) safelen(8)
    for (int i = 0; i < SIZE; i++) {
        if (data[i] > threshold) {
            found = 1;
            // This creates control flow divergence for SIMT
        }
    }
    
    printf("Found value > %f: %s\n", threshold, found ? "yes" : "no");
    
    return 0;
}
