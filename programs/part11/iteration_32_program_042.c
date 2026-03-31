#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define STRIDE_SIZE 512

// Vector type for mixed data operations
typedef float v4sf __attribute__((vector_size(16)));

// Function to introduce data-dependent condition
static inline int should_continue(float val, float threshold) {
    return val < threshold;
}

int main(int argc, char *argv[]) {
    // Runtime control for conditional SIMD execution
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    // Arrays with different access patterns
    float data[SIZE];
    float data_stride[SIZE * 2];  // For non-unit stride access
    int indices[SIZE];
    double mixed_data[SIZE];      // Different type for mixed operations
    v4sf vector_data[SIZE/4];     // Explicit vector type
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i / SIZE;
        data_stride[i*2] = (float)i / SIZE;
        indices[i] = (i * 3) % SIZE;  // Non-linear indexing
        mixed_data[i] = (double)i / SIZE;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        vector_data[i] = (v4sf){i*4, i*4+1, i*4+2, i*4+3};
    }
    
    float threshold = 0.5f;
    float sum = 0.0f;
    double mixed_sum = 0.0;
    
    // Conditional SIMD execution path - this may trigger IFN_GOMP_USE_SIMT
    if (use_simd) {
        // Complex SIMD loop with reduction and data-dependent condition
        #pragma omp simd reduction(+:sum) linear(i:1) safelen(16) aligned(data:32)
        for (int i = 0; i < SIZE; i++) {
            // Data-dependent condition that could affect SIMD execution
            if (data[i] > threshold && i % 8 == 0) {
                // This creates control flow within SIMD loop
                data[i] = sqrtf(data[i]);
            } else {
                data[i] = data[i] * 2.0f;
            }
            
            // Reduction operation
            sum += data[i];
            
            // Additional complexity with mixed data types
            mixed_data[i] = (double)data[i] * 1.5;
        }
    } else {
        // Sequential fallback
        for (int i = 0; i < SIZE; i++) {
            if (data[i] > threshold && i % 8 == 0) {
                data[i] = sqrtf(data[i]);
            } else {
                data[i] = data[i] * 2.0f;
            }
            sum += data[i];
            mixed_data[i] = (double)data[i] * 1.5;
        }
    }
    
    // Unconditional SIMD loop with non-unit stride - always present
    // This ensures SIMD constructs are parsed regardless of runtime condition
    #pragma omp simd safelen(8)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        // Non-unit stride access pattern
        data_stride[i*2] = data_stride[i*2] * 3.0f + sinf((float)i * 0.1f);
    }
    
    // Nested loops where inner loop uses SIMD
    // This creates more complex lowering scenarios
    for (int outer = 0; outer < 4; outer++) {
        #pragma omp simd
        for (int inner = 0; inner < SIZE/4; inner++) {
            int idx = outer * (SIZE/4) + inner;
            if (idx < SIZE) {
                // Indirect indexing pattern
                data[indices[idx]] += 0.1f;
            }
        }
    }
    
    // GPU offloading section - may trigger SIMT transformation for GPU
    if (use_offload) {
        float offload_sum = 0.0f;
        
        // Target offloading with SIMD - likely to use SIMT model
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) reduction(+:offload_sum) \
            num_teams(4) thread_limit(128)
        for (int i = 0; i < SIZE; i++) {
            // Complex computation with conditional
            if (i % 3 == 0) {
                data[i] = cosf(data[i]) * 2.0f;
            } else {
                data[i] = sinf(data[i]) * 1.5f;
            }
            offload_sum += data[i];
            
            // Early exit simulation (data-dependent)
            if (data[i] > 100.0f) {
                // In SIMT, this becomes lane masking
                data[i] = 100.0f;
            }
        }
        
        sum += offload_sum;
    }
    
    // Mixed vector type operations with SIMD
    #pragma omp simd
    for (int i = 0; i < SIZE/4; i++) {
        // Vector operations
        v4sf temp = vector_data[i] * (v4sf){2.0f, 1.5f, 1.0f, 0.5f};
        vector_data[i] = temp + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    }
    
    // Final reduction and output to prevent optimization
    double final_sum = (double)sum;
    for (int i = 0; i < SIZE; i++) {
        final_sum += (double)data[i] + mixed_data[i];
    }
    
    // Print results to prevent dead code elimination
    printf("Result: sum=%.4f, data[0]=%.4f, data[%d]=%.4f\n", 
           final_sum, data[0], SIZE-1, data[SIZE-1]);
    printf("SIMD used: %s, Offload used: %s\n", 
           use_simd ? "yes" : "no", use_offload ? "yes" : "no");
    
    return 0;
}
