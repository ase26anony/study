#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to complicate SIMD lowering
typedef float v4sf __attribute__((vector_size(16)));

void process_with_simd(float* data, float* data2, int* indices, int n, float* sum) {
    float local_sum = 0.0f;
    
    // Complex SIMD loop with reduction and conditional break
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n], data2[0:n]) map(to: indices[0:n]) \
        reduction(+:local_sum) if(target:0)
    for (int i = 0; i < n; i++) {
        // Data-dependent condition that might cause early exit
        if (data[i] > 1000.0f) {
            // This break complicates SIMD execution
            // continue;
        }
        
        // Non-contiguous access pattern
        data[indices[i]] = data[indices[i]] * 2.0f + 1.0f;
        
        // Mixed data type operations
        float temp = (float)i * 0.5f;
        data2[i] = data[i] + temp;
        
        // Reduction operation
        local_sum += data[i];
        
        // Vector type usage within SIMD loop
        v4sf vec = {data[i], data[i+1], data[i+2], data[i+3]};
        if (i + 3 < n) {
            vec = vec * 2.0f;
        }
    }
    
    *sum = local_sum;
}

void process_sequential(float* data, float* data2, int* indices, int n, float* sum) {
    float local_sum = 0.0f;
    for (int i = 0; i < n; i++) {
        if (data[i] > 1000.0f) {
            continue;
        }
        data[indices[i]] = data[indices[i]] * 2.0f + 1.0f;
        float temp = (float)i * 0.5f;
        data2[i] = data[i] + temp;
        local_sum += data[i];
    }
    *sum = local_sum;
}

int main(int argc, char** argv) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    float data[SIZE];
    float data2[SIZE];
    int indices[SIZE];
    float sum = 0.0f;
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 1.5f;
        data2[i] = (float)i * 0.75f;
        indices[i] = (i * 3) % SIZE;  // Non-linear indexing pattern
    }
    
    // Conditional SIMD execution based on runtime flag
    if (use_simd) {
        printf("Using SIMD execution path\n");
        
        // Nested loops with inner SIMD - encourages SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            if(target:use_offload) map(tofrom: data[0:SIZE]) \
            reduction(+:sum) collapse(2)
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < SIZE) {
                    // Complex operation with potential data dependency
                    data[idx] = data[idx] * (float)(i + 1) / (float)(j + 1);
                    sum += data[idx];
                    
                    // Conditional that might affect SIMD lane execution
                    if (data[idx] > 500.0f) {
                        data[idx] = 500.0f;
                    }
                }
            }
        }
        
        // Call function with SIMD constructs
        process_with_simd(data, data2, indices, SIZE, &sum);
    } else {
        printf("Using sequential execution path\n");
        process_sequential(data, data2, indices, SIZE, &sum);
    }
    
    // Always present SIMD loop with non-unit stride and safelen clause
    // This ensures SIMD constructs are always parsed
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access - complicates SIMD lowering
        data[i * 2] = data[i * 2] * 3.0f + (float)i * 0.1f;
        
        // Additional operation with mixed types
        double temp = (double)data[i * 2] * 0.5;
        if (i * 2 + 1 < SIZE) {
            data[i * 2 + 1] = (float)temp;
        }
    }
    
    // Another SIMD loop with linear clause
    float result = 0.0f;
    #pragma omp simd linear(i:1) reduction(+:result)
    for (int i = 0; i < SIZE; i += 4) {
        // Vectorized operation
        result += data[i] * data2[i];
        
        // Conditional execution within SIMD
        if (i % 8 == 0) {
            data[i] = data[i] * 0.9f;
        }
    }
    
    // Print results to prevent dead code elimination
    printf("Sum: %f\n", sum);
    printf("Result: %f\n", result);
    printf("Sample values: %f, %f, %f\n", data[0], data[SIZE/2], data[SIZE-1]);
    
    return 0;
}
