#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to test SIMT handling
typedef float v4sf __attribute__((vector_size(16)));

// Function with conditional SIMD execution
void process_data(float* data, float* data2, int* indices, int use_simd, float* sum_result) {
    float sum = 0.0f;
    
    // Runtime conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and conditional break
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE], data2[0:SIZE]) \
            map(to: indices[0:SIZE]) \
            reduction(+:sum) \
            safelen(32)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that might break
            if (data[i] > 1000.0f && i > SIZE/2) {
                // Early exit - creates complex control flow
                data[i] = 0.0f;
            } else {
                // Mixed operations
                float temp = data[i] * 2.0f + 1.0f;
                data[i] = temp;
                
                // Non-contiguous access with stride
                if (i % 2 == 0) {
                    data2[i/2] = temp * 0.5f;
                }
                
                // Reduction with mixed types
                sum += (double)temp;  // Mixed float/double
            }
            
            // Indirect indexing
            int idx = indices[i] % SIZE;
            data[idx] = data[idx] * 1.1f;
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
        }
    }
    
    *sum_result = sum;
    
    // Unconditional SIMD loop with non-unit stride - always present
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(16) aligned(data:16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access
        data[i*2] = data[i*2] * 3.0f;
        
        // Vector type operations
        if (i % 4 == 0 && i + 4 < STRIDE_SIZE) {
            v4sf* vptr = (v4sf*)&data[i*2];
            v4sf mult = {1.5f, 1.5f, 1.5f, 1.5f};
            *vptr = *vptr * mult;
        }
    }
}

// Nested loop with inner SIMD
void nested_simd_processing(float* data, int width, int height) {
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Inner SIMD loop - may trigger SIMT transformation
            #pragma omp simd linear(x:1) reduction(+:data[y*width + x])
            for (int k = 0; k < 8; k++) {
                data[y*width + x] += (x + y + k) * 0.1f;
                
                // Conditional inside SIMD loop
                if (data[y*width + x] > 50.0f) {
                    data[y*width + x] = 50.0f;
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    // Use command line argument to control SIMD execution
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    // Allocate and initialize arrays
    float* data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float* data2 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
        data2[i] = (float)(i * 2);
        indices[i] = (i * 3) % SIZE;
    }
    
    float sum_result = 0.0f;
    
    // Process with conditional SIMD
    process_data(data, data2, indices, use_simd, &sum_result);
    
    // Also test nested SIMD
    int width = 32, height = 32;
    float* nested_data = (float*)calloc(width * height, sizeof(float));
    nested_simd_processing(nested_data, width, height);
    
    // Print results to prevent dead code elimination
    printf("Sum result: %f\n", sum_result);
    printf("Data[0], [100], [500]: %f, %f, %f\n", 
           data[0], data[100], data[500]);
    printf("Nested data[0], [100]: %f, %f\n",
           nested_data[0], nested_data[100]);
    
    // Cleanup
    free(data);
    free(data2);
    free(indices);
    free(nested_data);
    
    return 0;
}
