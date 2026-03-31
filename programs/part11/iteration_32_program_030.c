#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE 2

// Mixed data types for complex access patterns
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
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
    
    // Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and data-dependent condition
        #pragma omp simd reduction(+:sum) safelen(16) linear(i:1) aligned(data:32)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition inside SIMD loop
            if (data[i * STRIDE] > 50.0f) {
                // Early exit - creates complex control flow
                data[i * STRIDE] = 50.0f;
            }
            data[i * STRIDE] = data[i * STRIDE] * 2.0f + 1.0f;
            sum += data[i * STRIDE];
            
            // Mixed data type operation
            dbl_data[i] = (double)data[i * STRIDE] * 0.5;
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            if (data[i * STRIDE] > 50.0f) {
                data[i * STRIDE] = 50.0f;
            }
            data[i * STRIDE] = data[i * STRIDE] * 2.0f + 1.0f;
            sum += data[i * STRIDE];
            dbl_data[i] = (double)data[i * STRIDE] * 0.5;
        }
    }
    
    // Unconditional SIMD loop with non-unit stride and safelen
    // This ensures SIMD constructs are always parsed
    #pragma omp simd safelen(8)
    for (int i = 0; i < SIZE/2; i++) {
        // Non-contiguous memory access pattern
        data[i * STRIDE * 2] = data[i * STRIDE * 2] * 3.0f;
    }
    
    // Nested loops with SIMD on inner loop
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 64; j++) {
            int idx = i * 64 + j;
            #pragma omp simd
            for (int k = 0; k < 4; k++) {
                // Vector type operations
                vec_data[idx/4][k] += (float)(i + j + k) * 0.01f;
            }
        }
    }
    
    // GPU offloading with SIMD - likely to trigger SIMT transformation
    if (use_offload) {
        float offload_sum = 0.0f;
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE*STRIDE]) \
            map(to: indices[0:SIZE]) \
            reduction(+:offload_sum) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < SIZE; i++) {
            // Indirect indexing - complex memory pattern
            int idx = indices[i];
            if (idx < SIZE * STRIDE) {
                data[idx] = data[idx] * 1.5f - 0.5f;
                // Data-dependent operation
                if (data[idx] < 0.0f) {
                    data[idx] = 0.0f;
                }
                offload_sum += data[idx];
            }
        }
        
        sum += offload_sum;
    }
    
    // Additional complex SIMD loop with mixed operations
    #pragma omp simd reduction(+:dbl_sum)
    for (int i = 0; i < SIZE; i++) {
        // Type conversion within SIMD loop
        float temp = (float)dbl_data[i];
        dbl_data[i] = (i % 2 == 0) ? 
            dbl_data[i] * 0.75 : 
            dbl_data[i] * 1.25;
        dbl_sum += dbl_data[i];
        
        // Conditional store with stride
        if (i % 3 == 0) {
            data[i * STRIDE] += temp;
        }
    }
    
    // Prevent dead code elimination
    printf("Results: sum = %f, dbl_sum = %lf\n", sum, dbl_sum);
    printf("Sample data[0] = %f, data[100] = %f, data[200] = %f\n", 
           data[0], data[100], data[200]);
    printf("Sample dbl_data[50] = %lf, dbl_data[150] = %lf\n",
           dbl_data[50], dbl_data[150]);
    
    return 0;
}
