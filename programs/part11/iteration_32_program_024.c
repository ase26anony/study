#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE 2

// Mixed data types to complicate vectorization
typedef float v4sf __attribute__((vector_size(16)));

void process_with_simd(float* data, int* indices, double* result, int n, int use_simd) {
    double sum = 0.0;
    
    // Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n]) map(to: indices[0:n]) \
            reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < n; i++) {
            // Data-dependent condition that might break
            if (data[i] > 1000.0f) {
                // This break complicates SIMD execution
                // and may trigger special handling
                data[i] = 1000.0f;
            }
            
            // Non-contiguous access pattern
            int idx = indices[i % (n/STRIDE)] * STRIDE;
            if (idx < n) {
                data[idx] = data[idx] * 2.0f + 1.0f;
                sum += data[idx];
            }
            
            // Mixed precision operations
            double temp = (double)data[i] * 1.5;
            data[i] = (float)(temp * 0.8);
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < n; i++) {
            if (data[i] > 1000.0f) {
                data[i] = 1000.0f;
            }
            int idx = indices[i % (n/STRIDE)] * STRIDE;
            if (idx < n) {
                data[idx] = data[idx] * 2.0f + 1.0f;
                sum += data[idx];
            }
            double temp = (double)data[i] * 1.5;
            data[i] = (float)(temp * 0.8);
        }
    }
    
    *result = sum;
}

void unconditional_simd_loop(float* data, int n) {
    // Always present SIMD loop with safelen clause
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < n; i += STRIDE) {
        // Non-unit stride access
        data[i] = data[i] * 3.0f;
        
        // Vector type usage within SIMD loop
        if (i + 3 < n) {
            v4sf* vec_ptr = (v4sf*)&data[i];
            v4sf scale = {1.1f, 1.2f, 1.3f, 1.4f};
            *vec_ptr = *vec_ptr * scale;
        }
    }
    
    // Nested loops with SIMD on inner loop
    #pragma omp parallel for simd collapse(2)
    for (int i = 0; i < n/4; i++) {
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < n) {
                data[idx] = data[idx] + (float)(i * j) * 0.01f;
            }
        }
    }
}

int main(int argc, char** argv) {
    // Use command line argument to control SIMD execution
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    // Medium-sized arrays
    float data[SIZE];
    int indices[SIZE/STRIDE];
    double result = 0.0;
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100) * 1.5f;
    }
    
    for (int i = 0; i < SIZE/STRIDE; i++) {
        indices[i] = i;
    }
    
    // Shuffle indices for non-contiguous access
    for (int i = 0; i < SIZE/STRIDE; i++) {
        int j = (i * 13) % (SIZE/STRIDE);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    
    printf("Starting computation with use_simd = %d\n", use_simd);
    
    // Process with conditional SIMD
    process_with_simd(data, indices, &result, SIZE, use_simd);
    
    // Always execute unconditional SIMD loop
    unconditional_simd_loop(data, SIZE);
    
    // Aggregate results to prevent dead code elimination
    double final_sum = 0.0;
    for (int i = 0; i < SIZE; i += SIZE/8) {
        final_sum += data[i];
    }
    
    printf("Result sum: %.2f\n", result);
    printf("Final aggregated sum: %.2f\n", final_sum);
    printf("Sample values: %.2f, %.2f, %.2f\n", 
           data[0], data[SIZE/2], data[SIZE-1]);
    
    return 0;
}
