#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE 2

// Mixed data types to complicate vectorization
typedef float v4sf __attribute__((vector_size(16)));

// Function with conditional SIMD execution
void process_data(float* data, int* indices, double* result, int use_simd, int n) {
    double sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n]) map(to: indices[0:n]) \
            reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < n; i++) {
            // Data-dependent condition that might break SIMD execution
            if (data[i] > 1000.0f && i > n/2) {
                // This could force SIMT lane masking
                data[i] = 0.0f;
            }
            
            // Non-contiguous access with indirect indexing
            int idx = indices[i % (n/STRIDE)] * STRIDE;
            if (idx < n) {
                data[idx] = data[idx] * 2.0f + 1.0f;
                sum += data[idx];
            }
            
            // Mixed precision computation
            double temp = (double)data[i] * 1.5;
            data[i] = (float)(temp * 0.8);
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < n; i++) {
            if (data[i] > 1000.0f && i > n/2) {
                data[i] = 0.0f;
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
    
    // Unconditional SIMD loop with safelen clause
    // This ensures SIMD constructs are always parsed
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < n; i += STRIDE) {
        // Non-unit stride access
        data[i] = data[i] * 3.0f - data[(i + 1) % n];
    }
}

// Function with nested loops where inner loop is SIMD
void nested_simd(float* a, float* b, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        // Inner SIMD loop - candidate for SIMT transformation
        #pragma omp simd linear(j:1) reduction(+:a[i])
        for (int j = 0; j < n; j++) {
            a[i] += b[j] * (i + j);
        }
        
        // Vector type operations within SIMD region
        v4sf vec_a = {a[i], a[(i+1)%n], a[(i+2)%n], a[(i+3)%n]};
        v4sf vec_b = {1.0f, 2.0f, 3.0f, 4.0f};
        vec_a = vec_a * vec_b;
        
        // Store back - this creates complex data flow
        a[i] = vec_a[0] + vec_a[1] + vec_a[2] + vec_a[3];
    }
}

int main(int argc, char** argv) {
    // Use command line argument to control SIMD execution
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    int n = SIZE;
    
    // Allocate and initialize arrays
    float* data = (float*)aligned_alloc(16, n * sizeof(float));
    int* indices = (int*)malloc(n * sizeof(int));
    double result = 0.0;
    
    // Initialize with pattern that creates data dependencies
    for (int i = 0; i < n; i++) {
        data[i] = (float)(i % 100);
        indices[i] = (i * 7) % (n / STRIDE);  // Create non-linear pattern
    }
    
    printf("Initial data[0] = %f, data[100] = %f\n", data[0], data[100]);
    
    // Process with conditional SIMD
    process_data(data, indices, &result, use_simd, n);
    
    printf("After process_data: result = %f, data[0] = %f\n", result, data[0]);
    
    // Also call nested SIMD function
    float* b = (float*)aligned_alloc(16, n * sizeof(float));
    for (int i = 0; i < n; i++) {
        b[i] = (float)(i % 50);
    }
    
    nested_simd(data, b, n/4);  // Smaller size for faster execution
    
    printf("After nested_simd: data[0] = %f, data[10] = %f\n", data[0], data[10]);
    
    // Prevent dead code elimination
    volatile float check = data[n-1] + (float)result;
    printf("Final check: %f\n", check);
    
    free(data);
    free(indices);
    free(b);
    
    return 0;
}
