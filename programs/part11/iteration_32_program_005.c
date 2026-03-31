#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Mixed data types and vector operations
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare arrays with different access patterns
    float data[SIZE];
    float data2[SIZE];
    int indices[SIZE];
    float result[SIZE/2]; // For non-contiguous access
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 1.5f;
        data2[i] = (float)i * 0.5f;
        indices[i] = (i * 3) % SIZE; // Non-linear indexing
    }
    
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    // Conditional SIMD execution based on runtime flag
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(32)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might break SIMD execution
            if (data[i] > 1500.0f) {
                // This break creates complexity for SIMT transformation
                // but we'll ensure it doesn't execute with our data range
            }
            data[i] = data[i] * 2.0f + data2[i];
            sum += data[i];
            
            // Mixed data type operation
            mixed_sum += (double)data[i] * 0.1;
        }
        
        // Nested loop with inner SIMD
        #pragma omp parallel for simd collapse(2) reduction(+:sum)
        for (int i = 0; i < SIZE/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < SIZE) {
                    data[idx] = data[idx] * (1.0f + (float)j * 0.5f);
                    sum += data[idx];
                }
            }
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + data2[i];
            sum += data[i];
            mixed_sum += (double)data[i] * 0.1;
        }
    }
    
    // Unconditional SIMD loop with non-contiguous memory access
    // This should always be processed for SIMD/SIMT transformation
    #pragma omp simd safelen(16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        result[i] = data[i * 2] * 3.0f + data2[i * 2 + 1];
        
        // Indirect indexing - complex access pattern
        if (i < SIZE/4) {
            data[indices[i]] += result[i] * 0.1f;
        }
    }
    
    // GPU offloading section - triggers SIMT for GPU execution
    if (use_offload) {
        float offload_data[SIZE];
        
        // Initialize offload data
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = (float)i;
        }
        
        // Target offloading with SIMD - likely to trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: offload_data[0:SIZE]) reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            // Complex computation with conditional
            if (offload_data[i] > 500.0f) {
                offload_data[i] = offload_data[i] * 1.5f;
            } else {
                offload_data[i] = offload_data[i] * 2.5f;
            }
            sum += offload_data[i];
            
            // Vector type operation within SIMD loop
            if (i % 4 == 0 && i + 4 < SIZE) {
                v4sf vec_data;
                for (int k = 0; k < 4; k++) {
                    vec_data[k] = offload_data[i + k];
                }
                vec_data = vec_data * 1.1f;
                for (int k = 0; k < 4; k++) {
                    offload_data[i + k] = vec_data[k];
                }
            }
        }
        
        // Print some results from offloaded computation
        printf("Offload results: %f, %f, %f\n", 
               offload_data[0], offload_data[SIZE/2], offload_data[SIZE-1]);
    }
    
    // Additional SIMD construct with linear clause
    // This creates another opportunity for SIMT transformation
    int linear_var = 0;
    #pragma omp simd linear(linear_var:1)
    for (int i = 0; i < SIZE; i++) {
        data[i] += (float)linear_var * 0.01f;
        linear_var++;
    }
    
    // Print results to prevent dead code elimination
    printf("Final sum: %f\n", sum);
    printf("Mixed sum: %lf\n", mixed_sum);
    printf("Sample data: %f, %f, %f\n", data[0], data[SIZE/2], data[SIZE-1]);
    printf("Sample result: %f, %f\n", result[0], result[STRIDE_SIZE-1]);
    
    return 0;
}
