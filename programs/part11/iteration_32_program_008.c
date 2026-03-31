#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to complicate SIMD lowering
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_gpu_offload = argc > 2;
    
    // Declare arrays with different access patterns
    float data[SIZE];
    double dbl_data[SIZE];
    int indices[SIZE];
    float strided_data[STRIDE_SIZE * 2]; // For non-unit stride access
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
        dbl_data[i] = (double)i * 0.5;
        indices[i] = (i * 3) % SIZE; // Non-linear indexing
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.25f;
    }
    
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        printf("Using SIMD path\n");
        
        if (use_gpu_offload) {
            // GPU offloading with SIMD - likely to trigger SIMT transformation
            #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:SIZE], dbl_data[0:SIZE]) \
                map(to: indices[0:SIZE]) \
                reduction(+:sum, dbl_sum) \
                safelen(32)
            for (int i = 0; i < SIZE; i++) {
                // Data-dependent condition inside SIMD loop
                if (data[i] > 500.0f) {
                    // Early exit - complicates SIMD lowering
                    data[i] = 500.0f;
                }
                
                // Mixed data type operations
                data[i] = data[i] * 2.0f + (float)dbl_data[i];
                dbl_data[i] = dbl_data[i] * 1.5 + (double)data[i];
                
                // Reduction with complex expression
                sum += data[i] * 0.1f;
                dbl_sum += dbl_data[i] * 0.05;
                
                // Non-contiguous access using indirect indexing
                int idx = indices[i];
                if (idx < SIZE) {
                    data[idx] += 0.5f;
                }
            }
        } else {
            // CPU SIMD with complex clauses
            #pragma omp simd reduction(+:sum) linear(i:1) aligned(data:16) safelen(16)
            for (int i = 0; i < SIZE; i++) {
                // Vector operations using explicit vector types
                v4sf* vptr = (v4sf*)(&data[i & ~3]);
                if ((i % 4) == 0 && i + 3 < SIZE) {
                    v4sf v = *vptr;
                    v = v * (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
                    *vptr = v;
                }
                
                data[i] = data[i] + (float)i * 0.01f;
                sum += data[i];
                
                // Conditional break - creates control flow in SIMD
                if (i > 100 && data[i] > 1000.0f) {
                    // This may force SIMT lane masking
                    data[i] = 1000.0f;
                }
            }
        }
    } else {
        printf("Using sequential path\n");
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
    }
    
    // Always present SIMD loop with non-unit stride - ensures SIMD constructs are parsed
    // This loop has a stride of 2, complicating SIMD lowering
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        strided_data[i * 2] = strided_data[i * 2] * 3.0f + (float)i;
    }
    
    // Another SIMD loop with reduction and conditional
    float conditional_sum = 0.0f;
    #pragma omp simd reduction(+:conditional_sum)
    for (int i = 0; i < SIZE; i++) {
        // Data-dependent operation
        if (data[i] > 250.0f) {
            conditional_sum += data[i];
            data[i] = data[i] * 0.9f;
        }
    }
    
    // Nested loops where inner loop is SIMD
    #pragma omp parallel for
    for (int outer = 0; outer < 4; outer++) {
        #pragma omp simd
        for (int inner = 0; inner < SIZE/4; inner++) {
            int idx = outer * (SIZE/4) + inner;
            if (idx < SIZE) {
                dbl_data[idx] = dbl_data[idx] * 1.1 + outer * 0.1;
            }
        }
    }
    
    // Print results to prevent dead code elimination
    printf("Results:\n");
    printf("  Sum: %f\n", sum);
    printf("  Double Sum: %lf\n", dbl_sum);
    printf("  Conditional Sum: %f\n", conditional_sum);
    printf("  Sample data[0]: %f, data[100]: %f, data[500]: %f\n", 
           data[0], data[100], data[500]);
    printf("  Strided sample: %f, %f\n", strided_data[0], strided_data[10]);
    
    return 0;
}
