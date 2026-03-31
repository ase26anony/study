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
        // Complex offloading directive with SIMD - likely to trigger SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n], data2[0:n]) \
            map(to: indices[0:n]) \
            reduction(+:sum) \
            safelen(32)
        for (int i = 0; i < n; i++) {
            // Data-dependent condition inside SIMD loop
            if (data[i] > 100.0f) {
                // Early exit - creates complex control flow for SIMT
                data[i] = 100.0f;
            }
            
            // Non-contiguous access pattern
            float val = data[indices[i]] * 2.0f + data2[i];
            
            // Mixed operations
            data[i] = val * 1.5f;
            data2[i] = val * 0.5f;
            
            // Reduction with conditional
            if (val > 50.0f) {
                sum += val;
            }
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < n; i++) {
            if (data[i] > 100.0f) {
                data[i] = 100.0f;
            }
            float val = data[indices[i]] * 2.0f + data2[i];
            data[i] = val * 1.5f;
            data2[i] = val * 0.5f;
            if (val > 50.0f) {
                sum += val;
            }
        }
    }
    
    // Unconditional SIMD loop with non-unit stride - always present
    #pragma omp simd safelen(16) aligned(data:16) linear(i:1)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access
        data[i * 2] = data[i * 2] * 3.0f + (float)i * 0.1f;
    }
    
    // Nested loops with inner SIMD
    #pragma omp parallel for
    for (int i = 0; i < n/4; i++) {
        // Inner loop with SIMD - may trigger different lowering
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < n) {
                data2[idx] = data2[idx] + data[idx] * 0.25f;
                sum += data2[idx];
            }
        }
    }
    
    *sum_result = sum;
}

void process_with_vector_types(v4sf* vec_data, int n) {
    // Using explicit vector types with OpenMP
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        // Vector operations
        v4sf temp = vec_data[i] * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        vec_data[i] = temp + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    }
}

int main(int argc, char** argv) {
    // Runtime condition to control SIMD path
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    // Allocate and initialize arrays
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* data2 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    v4sf* vec_data = (v4sf*)aligned_alloc(16, (SIZE/4) * sizeof(v4sf));
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 1.5f;
        data2[i] = (float)i * 0.75f;
        indices[i] = (i * 3 + 1) % SIZE;  // Non-linear indexing
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){(float)i, (float)i+1, (float)i+2, (float)i+3};
    }
    
    float sum_result = 0.0f;
    
    // Process with conditional SIMD execution
    process_with_simd(data, data2, indices, SIZE, use_simd, &sum_result);
    
    // Process with vector types
    process_with_vector_types(vec_data, SIZE/4);
    
    // Aggregate results to prevent dead code elimination
    printf("Sum result: %f\n", sum_result);
    printf("Sample data[0]: %f, data[100]: %f\n", data[0], data[100]);
    printf("Sample data2[50]: %f\n", data2[50]);
    printf("Sample vec_data[0]: %f %f %f %f\n", 
           vec_data[0][0], vec_data[0][1], vec_data[0][2], vec_data[0][3]);
    
    // Cleanup
    free(data);
    free(data2);
    free(indices);
    free(vec_data);
    
    return 0;
}
