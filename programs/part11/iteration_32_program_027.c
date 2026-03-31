#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Mixed data types and vector operations
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare arrays with different access patterns
    float data[SIZE];
    float data2[SIZE];
    int indices[SIZE];
    v4sf vector_data[SIZE/4];
    double double_data[SIZE];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 0.5f;
        data2[i] = (float)i * 1.5f;
        indices[i] = (i * 3) % SIZE;  // Non-linear indexing
        double_data[i] = (double)i * 0.25;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*4.0f, i*4.0f+1.0f, i*4.0f+2.0f, i*4.0f+3.0f};
    }
    
    float sum = 0.0f;
    double double_sum = 0.0;
    
    // Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(16)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that could break SIMD execution
            if (data[i] > 1000.0f) {
                // This break creates complexity for SIMT transformation
                // break;
            }
            
            // Non-contiguous memory access with stride
            if (i % 2 == 0) {
                data[i] = data[i] * 2.0f + data2[indices[i]];
            } else {
                data[i] = data[i] * 3.0f - data2[indices[i]];
            }
            
            // Reduction operation
            sum += data[i];
            
            // Mixed data type operation
            double_data[i] = (double)data[i] * 0.5;
        }
        
        // Nested loop with inner SIMD - another candidate for SIMT
        #pragma omp parallel for simd collapse(2) reduction(+:double_sum)
        for (int i = 0; i < SIZE/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                double_data[idx] = double_data[idx] * 1.1 + (double)j;
                double_sum += double_data[idx];
            }
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < SIZE; i++) {
            if (i % 2 == 0) {
                data[i] = data[i] * 2.0f + data2[indices[i]];
            } else {
                data[i] = data[i] * 3.0f - data2[indices[i]];
            }
            sum += data[i];
            double_data[i] = (double)data[i] * 0.5;
        }
    }
    
    // Unconditional SIMD loop with non-unit stride - always present
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(8) aligned(data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        data[i*2] = data[i*2] * 3.0f + (float)i * 0.1f;
    }
    
    // Vector type operations with SIMD
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        v4sf temp = vector_data[i] * (v4sf){2.0f, 1.5f, 1.0f, 0.5f};
        vector_data[i] = temp + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    }
    
    // GPU offloading directive - strong candidate for SIMT transformation
    if (use_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(to: data2[0:SIZE]) \
            reduction(+:sum) collapse(2)
        for (int i = 0; i < SIZE/4; i++) {
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                // Complex computation with conditional
                float val = data[idx];
                if (val > 500.0f) {
                    data[idx] = val * 0.5f + data2[(idx * 7) % SIZE];
                } else {
                    data[idx] = val * 2.0f - data2[(idx * 3) % SIZE];
                }
                sum += data[idx];
            }
        }
    }
    
    // Additional SIMD loop with indirect indexing
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        int idx = indices[i];
        data[idx] = data[idx] * 1.01f;
    }
    
    // Prevent dead code elimination
    printf("Results: sum = %f, double_sum = %f\n", sum, double_sum);
    printf("Sample data[0] = %f, data[100] = %f, data[500] = %f\n", 
           data[0], data[100], data[500]);
    
    // Verify vector operations
    v4sf test_vec = vector_data[0];
    printf("Vector result: %f %f %f %f\n", 
           test_vec[0], test_vec[1], test_vec[2], test_vec[3]);
    
    return 0;
}
