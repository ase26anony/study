#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Define a vector type to test mixed data handling
typedef float v4sf __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE 2

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Initialize arrays with different patterns
    float data[SIZE * STRIDE];
    double dbl_data[SIZE];
    int indices[SIZE];
    v4sf vec_data[SIZE/4];
    
    // Initialize arrays
    for (int i = 0; i < SIZE * STRIDE; i++) {
        data[i] = (float)(i % 100) * 0.1f;
    }
    
    for (int i = 0; i < SIZE; i++) {
        dbl_data[i] = (double)(i % 50) * 0.2;
        indices[i] = (i * 3) % (SIZE * STRIDE);
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex loop with reduction and conditional break
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(32)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might break
            if (data[i * STRIDE] > 50.0f && i > SIZE/2) {
                // This break creates complexity for SIMT transformation
                // break; // Commented to avoid infinite loops in coverage runs
            }
            
            // Non-contiguous memory access with stride
            data[i * STRIDE] = data[i * STRIDE] * 2.0f + 1.0f;
            
            // Reduction operation
            sum += data[i * STRIDE];
            
            // Mixed data type operation
            dbl_data[i] = (double)data[i * STRIDE] * 0.5;
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            data[i * STRIDE] = data[i * STRIDE] * 2.0f + 1.0f;
            sum += data[i * STRIDE];
            dbl_data[i] = (double)data[i * STRIDE] * 0.5;
        }
    }
    
    // Unconditional SIMD loop with complex access pattern
    // This should always be processed by the SIMD lowering
    #pragma omp simd safelen(16) aligned(data:32)
    for (int i = 0; i < SIZE/2; i++) {
        // Indirect indexing - creates complex memory access pattern
        int idx = indices[i];
        if (idx < SIZE * STRIDE) {
            data[idx] = data[idx] * 3.0f - 2.0f;
        }
    }
    
    // Nested loops with SIMD on inner loop
    // Outer loop with teams/distribute, inner with SIMD
    if (use_offload) {
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:SIZE*STRIDE]) \
                map(tofrom: dbl_data[0:SIZE]) \
                reduction(+:dbl_sum)
        for (int i = 0; i < SIZE; i++) {
            // Complex computation with mixed types
            float temp = data[i * STRIDE] * 0.7f;
            
            // Conditional operation inside SIMD loop
            if (temp > 10.0f) {
                dbl_data[i] = dbl_data[i] + (double)temp;
            } else {
                dbl_data[i] = dbl_data[i] - (double)temp;
            }
            
            // Another reduction
            dbl_sum += dbl_data[i];
            
            // Vector type operation
            if (i % 4 == 0 && i + 3 < SIZE) {
                int vec_idx = i / 4;
                vec_data[vec_idx] = vec_data[vec_idx] * 2.0f;
            }
        }
    }
    
    // Additional SIMD loop with linear clause and conditional
    int linear_var = 0;
    #pragma omp simd linear(linear_var:1) reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        // Use linear variable in computation
        data[i] = data[i] + (float)linear_var * 0.01f;
        linear_var++;
        
        // Conditional that might affect SIMT lane masking
        if (i % 3 == 0) {
            sum += data[i] * 2.0f;
        } else {
            sum += data[i];
        }
    }
    
    // Print results to prevent dead code elimination
    printf("Results:\n");
    printf("  Sum: %f\n", sum);
    printf("  Double Sum: %lf\n", dbl_sum);
    printf("  Sample data[0]: %f\n", data[0]);
    printf("  Sample data[100]: %f\n", data[100]);
    printf("  Sample dbl_data[50]: %lf\n", dbl_data[50]);
    
    // Also print the use_simd flag to show which path was taken
    printf("  SIMD path used: %s\n", use_simd ? "YES" : "NO");
    printf("  Offload path used: %s\n", use_offload ? "YES" : "NO");
    
    return 0;
}
