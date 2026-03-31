#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE 2

// Mixed data types to complicate SIMD lowering
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    // Runtime condition for conditional SIMD execution
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare arrays with different access patterns
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
        vec_data[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(16)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might break SIMD execution
            if (data[i] > 50.0f && i > SIZE/2) {
                // This break complicates SIMD lowering
                // sum += 100.0f;
                // continue;
            }
            
            // Non-contiguous memory access
            data[indices[i]] = data[indices[i]] * 2.0f + 1.0f;
            
            // Mixed data type operation
            dbl_data[i] = (double)data[i] * 0.5;
            
            // Reduction
            sum += data[i];
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < SIZE; i++) {
            data[indices[i]] = data[indices[i]] * 2.0f + 1.0f;
            dbl_data[i] = (double)data[i] * 0.5;
            sum += data[i];
        }
    }
    
    // Always present SIMD loop with complex pattern
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd aligned(data:16) safelen(8)
    for (int i = 0; i < SIZE/STRIDE; i++) {
        // Non-unit stride access
        data[i * STRIDE] = data[i * STRIDE] * 3.0f;
        
        // Vector type operation
        if (i < SIZE/(4*STRIDE)) {
            vec_data[i] = vec_data[i] * 2.0f;
        }
    }
    
    // GPU offloading path - may trigger SIMT transformation for GPU
    if (use_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE*STRIDE]) map(tofrom: sum) \
            reduction(+:dbl_sum)
        for (int i = 0; i < SIZE; i++) {
            // Complex GPU computation
            float temp = data[i];
            for (int j = 0; j < 4; j++) {
                temp = temp * 0.9f + 0.1f;
            }
            data[i] = temp;
            dbl_sum += (double)temp;
            
            // Conditional inside SIMD loop on GPU
            if (temp > 100.0f) {
                data[i] = 100.0f;
            }
        }
    }
    
    // Nested loops with SIMD on inner loop
    #pragma omp parallel for
    for (int i = 0; i < SIZE/16; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < SIZE) {
                data[idx] = data[idx] + (float)j * 0.01f;
                sum += data[idx];
            }
        }
    }
    
    // Prevent dead code elimination
    printf("Results: sum = %f, dbl_sum = %lf\n", sum, dbl_sum);
    printf("Sample data[0] = %f, data[100] = %f\n", data[0], data[100]);
    printf("Sample dbl_data[50] = %lf\n", dbl_data[50]);
    
    return 0;
}
