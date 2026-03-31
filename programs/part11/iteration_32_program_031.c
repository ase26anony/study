#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to encourage complex SIMT handling
typedef float v4sf __attribute__((vector_size(16)));

// Function with conditional SIMD execution
void conditional_simd_computation(float* data, int* indices, double* result, int use_simd) {
    double sum = 0.0;
    
    if (use_simd) {
        // This conditional block may trigger the SIMT wrapper generation
        // Target offloading directive with SIMD - likely to use SIMT lowering
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(to: indices[0:SIZE]) \
            reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < SIZE; i++) {
            // Complex data-dependent condition inside SIMD loop
            if (data[i] > 100.0f) {
                // Early exit condition - creates control flow divergence
                data[i] = 0.0f;
            } else {
                // Non-contiguous memory access pattern
                float temp = data[indices[i % STRIDE_SIZE]];
                data[i] = data[i] * 2.0f + temp * 0.5f;
            }
            // Reduction operation
            sum += data[i];
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < SIZE; i++) {
            if (data[i] > 100.0f) {
                data[i] = 0.0f;
            } else {
                float temp = data[indices[i % STRIDE_SIZE]];
                data[i] = data[i] * 2.0f + temp * 0.5f;
            }
            sum += data[i];
        }
    }
    
    *result = sum;
}

// Function with unconditional SIMD containing complex patterns
void unconditional_simd_operations(float* data, v4sf* vector_data) {
    // SIMD loop with safelen clause and non-unit stride
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        data[i * 2] = data[i * 2] * 3.0f + (float)i * 0.1f;
    }
    
    // Mixed data type operations
    #pragma omp simd
    for (int i = 0; i < STRIDE_SIZE/4; i++) {
        // Vector type operations
        v4sf v = {1.0f, 2.0f, 3.0f, 4.0f};
        vector_data[i] = vector_data[i] + v * 0.5f;
    }
    
    // SIMD loop with linear clause and conditional
    int counter = 0;
    #pragma omp simd linear(counter:1)
    for (int i = 0; i < SIZE; i++) {
        // Data-dependent condition that may cause lane masking
        if (data[i] < 50.0f) {
            data[i] += 10.0f;
            counter++;
        }
    }
}

// Nested loops with SIMD on inner loop
void nested_simd_loop(float* a, float* b, float* c) {
    #pragma omp parallel for
    for (int i = 0; i < 64; i++) {
        // Inner SIMD loop - may trigger SIMT transformation
        #pragma omp simd reduction(+:a[i])
        for (int j = 0; j < 16; j++) {
            a[i] += b[i * 16 + j] * c[j];
            // Complex condition inside inner SIMD loop
            if (a[i] > 1000.0f) {
                a[i] = 1000.0f;
            }
        }
    }
}

int main(int argc, char** argv) {
    // Runtime condition to control SIMD path
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    // Allocate and initialize arrays
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* data2 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    v4sf* vector_data = (v4sf*)aligned_alloc(16, STRIDE_SIZE/4 * sizeof(v4sf));
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100);
        data2[i] = (float)(i % 50);
        indices[i] = (i * 3) % SIZE;
    }
    
    for (int i = 0; i < STRIDE_SIZE/4; i++) {
        for (int j = 0; j < 4; j++) {
            vector_data[i][j] = (float)(i * 4 + j);
        }
    }
    
    double result = 0.0;
    
    // Call conditional SIMD function - main trigger for uncovered code
    conditional_simd_computation(data, indices, &result, use_simd);
    
    // Always execute unconditional SIMD operations
    unconditional_simd_operations(data2, vector_data);
    
    // Execute nested SIMD loop
    nested_simd_loop(data, data2, data);
    
    // Prevent dead code elimination
    printf("Result: %f\n", result);
    printf("Data[0]: %f, Data[100]: %f, Data2[50]: %f\n", 
           data[0], data[100], data2[50]);
    
    // Cleanup
    free(data);
    free(data2);
    free(indices);
    free(vector_data);
    
    return 0;
}
