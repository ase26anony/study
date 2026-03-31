#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to complicate vectorization
typedef float v4sf __attribute__((vector_size(16)));

// Function with conditional SIMD execution
void process_data(float* data, int* indices, double* results, int use_simd, int n) {
    double sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n]) map(to: indices[0:n]) \
            map(tofrom: sum) reduction(+:sum) \
            num_teams(4) thread_limit(64)
        for (int i = 0; i < n; i++) {
            // Data-dependent condition that might break
            if (data[i] > 1000.0f) {
                // This creates complexity for SIMT execution
                data[i] = 1000.0f;
            }
            
            // Non-contiguous access with indirect indexing
            int idx = indices[i % STRIDE_SIZE];
            float val = data[idx % n];
            
            // Mixed precision computation
            data[i] = val * 2.0f + (float)i * 0.1f;
            
            // Reduction with mixed types
            sum += (double)data[i];
            
            // Another conditional that might affect SIMT lanes
            if (i > n/2 && data[i] < 0.0f) {
                data[i] = 0.0f;
            }
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < n; i++) {
            if (data[i] > 1000.0f) {
                data[i] = 1000.0f;
            }
            int idx = indices[i % STRIDE_SIZE];
            float val = data[idx % n];
            data[i] = val * 2.0f + (float)i * 0.1f;
            sum += (double)data[i];
            if (i > n/2 && data[i] < 0.0f) {
                data[i] = 0.0f;
            }
        }
    }
    
    results[0] = sum;
    
    // Always present SIMD construct with safelen and non-unit stride
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < n/2; i++) {
        // Non-unit stride access
        data[i*2] = data[i*2] * 3.0f + sinf((float)i) * 0.5f;
    }
    
    // Nested loop with inner SIMD - another candidate for SIMT transformation
    #pragma omp parallel for simd collapse(2) reduction(+:sum)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 64; j++) {
            int idx = i * 64 + j;
            if (idx < n) {
                data[idx] += (float)(i * j) * 0.01f;
                sum += data[idx];
            }
        }
    }
    
    results[1] = sum;
}

// Function using explicit vector types
void vector_operations(v4sf* vec_data, float* scalar_data, int n, int use_simd) {
    if (use_simd) {
        // SIMD loop with explicit vector types
        #pragma omp simd linear(i:1) aligned(vec_data, scalar_data:16)
        for (int i = 0; i < n/4; i++) {
            v4sf vec = vec_data[i];
            v4sf result = vec * (v4sf){2.0f, 1.5f, 1.0f, 0.5f};
            
            // Scatter operation - complex for SIMT
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < n) {
                    scalar_data[idx] = result[j] + scalar_data[idx];
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    // Runtime condition to control SIMD execution
    int use_simd = argc > 1;  // Enable SIMD if any argument provided
    
    // Allocate and initialize data
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int* indices = (int*)malloc(STRIDE_SIZE * sizeof(int));
    double results[2] = {0.0, 0.0};
    v4sf* vec_data = (v4sf*)aligned_alloc(16, (SIZE/4) * sizeof(v4sf));
    float* scalar_data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 1.5f;
        scalar_data[i] = (float)i * 0.5f;
        if (i < STRIDE_SIZE) {
            indices[i] = (i * 3) % SIZE;  // Non-linear pattern
        }
    }
    
    // Initialize vector data
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){(float)i, (float)i+1, (float)i+2, (float)i+3};
    }
    
    printf("Processing with SIMD %s\n", use_simd ? "enabled" : "disabled");
    
    // Process data with conditional SIMD execution
    process_data(data, indices, results, use_simd, SIZE);
    
    // Perform vector operations
    vector_operations(vec_data, scalar_data, SIZE, use_simd);
    
    // Aggregate and print results to prevent dead code elimination
    double total = results[0] + results[1];
    printf("Result sum: %f\n", total);
    printf("Sample values: data[0]=%f, data[100]=%f, data[500]=%f\n", 
           data[0], data[100], data[500]);
    printf("Scalar sample: scalar_data[50]=%f\n", scalar_data[50]);
    
    // Cleanup
    free(data);
    free(indices);
    free(vec_data);
    free(scalar_data);
    
    return 0;
}
