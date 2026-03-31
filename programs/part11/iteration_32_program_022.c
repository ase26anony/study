#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Mixed data types to test SIMT handling
typedef float v4sf __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    // Runtime condition to control SIMD execution path
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Declare arrays with different access patterns
    float data[SIZE];
    double dbl_data[SIZE];
    int indices[SIZE];
    float strided_data[STRIDE_SIZE * 2]; // For non-unit stride access
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
        dbl_data[i] = (double)i * 0.5;
        indices[i] = (i * 3) % SIZE;
    }
    
    for (int i = 0; i < STRIDE_SIZE * 2; i++) {
        strided_data[i] = (float)i * 0.25f;
    }
    
    // Reduction variable
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    // Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex loop with reduction and conditional break
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(32)
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            
            // Data-dependent condition that might affect SIMT execution
            if (data[i] > 1000.0f && i > SIZE/2) {
                // This could influence SIMT lane masking
                data[i] = 1000.0f;
            }
        }
        
        // Mixed data type operations
        #pragma omp simd reduction(+:dbl_sum)
        for (int i = 0; i < SIZE; i++) {
            dbl_data[i] = dbl_data[i] * 1.5 + data[i];
            dbl_sum += dbl_data[i];
        }
    } else {
        // Sequential version
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
            sum += data[i];
            if (data[i] > 1000.0f && i > SIZE/2) {
                data[i] = 1000.0f;
            }
        }
        
        for (int i = 0; i < SIZE; i++) {
            dbl_data[i] = dbl_data[i] * 1.5 + data[i];
            dbl_sum += dbl_data[i];
        }
    }
    
    // Unconditional SIMD loop with non-unit stride - always present
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(16)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-contiguous memory access pattern
        strided_data[i * 2] = strided_data[i * 2] * 3.0f;
    }
    
    // Indirect indexing pattern - complex memory access
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        int idx = indices[i];
        if (idx < SIZE) {
            data[idx] = data[idx] * 0.5f;
        }
    }
    
    // Nested loops with SIMD on inner loop
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        #pragma omp simd
        for (int j = 0; j < 256; j++) {
            int idx = i * 256 + j;
            if (idx < SIZE) {
                data[idx] += (float)j * 0.1f;
            }
        }
    }
    
    // GPU offloading section - may trigger SIMT transformation for GPU
    if (use_offload) {
        float offload_data[SIZE];
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = (float)i;
        }
        
        // Target offloading directive - often uses SIMT model
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: offload_data[0:SIZE]) reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            offload_data[i] = offload_data[i] * 2.0f - 1.0f;
            sum += offload_data[i];
            
            // Conditional inside offloaded loop
            if (offload_data[i] < 0) {
                offload_data[i] = 0.0f;
            }
        }
        
        // Print some results from offloaded computation
        printf("Offload result[0]=%f, [%d]=%f\n", 
               offload_data[0], SIZE-1, offload_data[SIZE-1]);
    }
    
    // Use vector types explicitly
    v4sf vec_data[SIZE/4];
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){i*4, i*4+1, i*4+2, i*4+3};
    }
    
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = vec_data[i] * 2.0f;
    }
    
    // Print results to prevent dead code elimination
    printf("Results: sum=%f, dbl_sum=%lf\n", sum, dbl_sum);
    printf("Sample data[0]=%f, data[100]=%f, data[%d]=%f\n", 
           data[0], data[100], SIZE-1, data[SIZE-1]);
    printf("Strided[0]=%f, strided[10]=%f\n", 
           strided_data[0], strided_data[20]);
    
    return 0;
}
