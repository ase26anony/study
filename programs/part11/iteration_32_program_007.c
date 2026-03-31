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
    int use_offload = argc > 2;
    
    // Declare arrays with different access patterns
    float data[SIZE];
    float data2[SIZE];
    int indices[SIZE];
    double mixed_data[SIZE];
    v4sf vector_data[SIZE/4];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
        data2[i] = (float)(SIZE - i);
        indices[i] = (i * 3) % SIZE;  // Non-linear indexing
        mixed_data[i] = (double)i * 0.5;
        if (i % 4 == 0) {
            vector_data[i/4] = (v4sf){i, i+1, i+2, i+3};
        }
    }
    
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex loop with reduction and conditional break
        #pragma omp simd reduction(+:sum) safelen(16) linear(i:1)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that could affect SIMD execution
            if (data[i] > 500.0f && i > 100) {
                // Early exit - complicates SIMD vectorization
                // This may trigger special SIMT handling
            }
            
            // Non-contiguous access pattern
            data[indices[i]] = data[indices[i]] * 2.0f + 1.0f;
            
            // Reduction operation
            sum += data[i];
            
            // Mixed data type operation
            mixed_data[i] = data[i] * 0.5;
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            mixed_data[i] = data[i] * 0.5;
        }
    }
    
    // Always present SIMD loop with non-unit stride
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access - tests SIMT lane management
        data2[i*2] = data2[i*2] * 3.0f + (float)i;
    }
    
    // Target offloading directive - often uses SIMT model
    if (use_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(to: indices[0:SIZE]) \
            reduction(+:mixed_sum)
        for (int i = 0; i < SIZE; i++) {
            // Complex access pattern for GPU offloading
            float temp = data[indices[i]];
            data[i] = temp * temp + (float)i;
            mixed_sum += mixed_data[i];
            
            // Conditional operation inside SIMD loop
            if (data[i] > 1000.0f) {
                data[i] = 1000.0f;
            }
        }
    }
    
    // Nested loops with inner SIMD - tests nested SIMT transformation
    #pragma omp parallel for
    for (int outer = 0; outer < 4; outer++) {
        #pragma omp simd aligned(data:16) linear(inner:1)
        for (int inner = 0; inner < SIZE/4; inner++) {
            int idx = outer * (SIZE/4) + inner;
            data[idx] = data[idx] / (idx + 1.0f);
        }
    }
    
    // Vector type operations with SIMD
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = vector_data[i] + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    }
    
    // Print results to prevent dead code elimination
    printf("Sum: %f\n", sum);
    printf("Mixed sum: %lf\n", mixed_sum);
    printf("Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data[0], data[100], data[500]);
    printf("Sample data2[10]: %f, data2[100]: %f\n", data2[10], data2[100]);
    
    return 0;
}
