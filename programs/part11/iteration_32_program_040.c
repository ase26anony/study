#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to test SIMT handling
typedef float v4sf __attribute__((vector_size(16)));

void process_with_simd(float* data, float* data2, int* indices, int n, int use_simd, float* sum_result) {
    float sum = 0.0f;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n], data2[0:n]) \
            map(to: indices[0:n]) \
            reduction(+:sum) \
            safelen(16)
        for (int i = 0; i < n; i++) {
            // Data-dependent condition inside SIMD loop
            if (data[i] > 1000.0f) {
                // This break makes SIMT transformation more complex
                // continue;
            }
            
            // Non-contiguous access pattern
            float val = data[indices[i]] * 2.0f + 1.0f;
            data2[i] = val;
            
            // Reduction operation
            sum += val;
            
            // Mixed precision operations
            data[i] = (float)((double)val * 0.5);
        }
        
        *sum_result = sum;
    } else {
        // Sequential fallback
        for (int i = 0; i < n; i++) {
            float val = data[indices[i]] * 2.0f + 1.0f;
            data2[i] = val;
            data[i] = val * 0.5f;
            sum += val;
        }
        *sum_result = sum;
    }
}

void nested_simd_loop(float* data, int n) {
    // Nested loops with SIMD on inner loop
    #pragma omp parallel for
    for (int i = 0; i < n/16; i++) {
        // Inner loop with SIMD directive - may trigger SIMT transformation
        #pragma omp simd aligned(data:16) linear(i:16) safelen(8)
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < n) {
                // Vector-style operation
                data[idx] = data[idx] * 3.0f - 2.0f;
            }
        }
    }
}

void stride_access_pattern(float* data, int n) {
    // Unconditional SIMD loop with non-unit stride
    // This ensures SIMD constructs are always parsed
    #pragma omp simd safelen(4)
    for (int i = 0; i < n/2; i++) {
        // Non-unit stride access pattern
        data[i * 2] = data[i * 2] * 1.5f + 0.5f;
    }
}

void vector_type_operations(v4sf* vec_data, int n) {
    // Using explicit vector types with OpenMP
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        // Vector operations that may require lane management
        vec_data[i] = vec_data[i] + vec_data[i] * (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    }
}

int main(int argc, char** argv) {
    // Runtime condition to control SIMD path
    int use_simd = (argc > 1) ? atoi(argv[1]) : 1;
    
    // Initialize arrays
    float data[SIZE];
    float data2[SIZE];
    int indices[SIZE];
    v4sf vec_data[SIZE/4];
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100);
        data2[i] = 0.0f;
        indices[i] = (i * 3) % SIZE;  // Non-linear indexing pattern
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    float sum_result = 0.0f;
    
    // Process with conditional SIMD execution
    process_with_simd(data, data2, indices, SIZE, use_simd, &sum_result);
    
    // Always execute these to ensure SIMD constructs are parsed
    nested_simd_loop(data, SIZE);
    stride_access_pattern(data, SIZE);
    vector_type_operations(vec_data, SIZE/4);
    
    // Prevent dead code elimination
    printf("Sum result: %f\n", sum_result);
    printf("Sample data[0]: %f, data[100]: %f\n", data[0], data[100]);
    printf("Sample data2[50]: %f\n", data2[50]);
    printf("Sample vec_data[0]: %f %f %f %f\n", 
           vec_data[0][0], vec_data[0][1], vec_data[0][2], vec_data[0][3]);
    
    return 0;
}
