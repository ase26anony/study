#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Function with target clones for different vector ISAs
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
float compute_vector(float* dst, const float* src, int n) {
    // Alignment hints for better vectorization
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    float sum = 0.0f;
    
    // Loop 1: Multiple vectorizable built-ins on float data
    for (int i = 0; i < n; i++) {
        // Chain of vectorizable math built-ins
        float val = src[i];
        val = __builtin_sqrtf(val);           // Vectorizable sqrt
        val = __builtin_sinf(val);            // Vectorizable sin
        val = __builtin_fabsf(val);           // Vectorizable fabs
        val = __builtin_expf(val * 0.5f);     // Vectorizable exp
        dst[i] = val;
        sum += val;
    }
    
    // Loop 2: Different built-in combinations
    for (int i = 0; i < n; i += 2) {
        // Use powf with constant exponent (often vectorizable)
        dst[i] = __builtin_powf(dst[i], 1.5f);
    }
    
    return sum;
}

// Another function with explicit AVX2 target
__attribute__((target("avx2")))
double compute_vector_double(double* dst, const double* src, int n) {
    src = (const double*)__builtin_assume_aligned(src, 32);
    dst = (double*)__builtin_assume_aligned(dst, 32);
    
    double sum = 0.0;
    
    // Loop with double-precision vectorizable built-ins
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);        // Double precision sqrt
        val = __builtin_sin(val);         // Double precision sin
        val = __builtin_fabs(val);        // Double precision fabs
        val = __builtin_exp(val * 0.5);   // Double precision exp
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

// Function with mixed types and conversions
__attribute__((target_clones("avx2", "sse4.2")))
float mixed_vector_ops(float* dst, const float* src, int n) {
    src = (const float*)__builtin_assume_aligned(src, 16);
    dst = (float*)__builtin_assume_aligned(dst, 16);
    
    float sum = 0.0f;
    
    // Mix of operations that might trigger different vectorized built-ins
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        // Trigonometric operations
        val = __builtin_sinf(val);
        val = __builtin_cosf(val);
        
        // Exponential and logarithmic
        val = __builtin_expf(val);
        val = __builtin_logf(val + 1.0f);
        
        // Power function
        val = __builtin_powf(val, 2.0f);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

// Main function with hidden visibility to interact with DECL_VISIBILITY
__attribute__((visibility("hidden")))
int main() {
    const int N = 1024;
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst = aligned_alloc(32, N * sizeof(float));
    double* src_double = aligned_alloc(32, N * sizeof(double));
    double* dst_double = aligned_alloc(32, N * sizeof(double));
    
    // Initialize arrays with trigonometric values
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        src_double[i] = sin(i * 0.1);
    }
    
    // Call target-cloned functions multiple times
    float sum1 = 0.0f, sum2 = 0.0f;
    double sum_double = 0.0;
    
    // Multiple calls to increase chance of vectorization
    for (int iter = 0; iter < 10; iter++) {
        sum1 += compute_vector(dst, src, N);
        sum2 += mixed_vector_ops(dst, src, N);
        sum_double += compute_vector_double(dst_double, src_double, N);
        
        // Modify source slightly each iteration
        for (int i = 0; i < N; i++) {
            src[i] += 0.01f;
            src_double[i] += 0.01;
        }
    }
    
    // Compute checksums
    float checksum_float = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_float += dst[i];
        checksum_double += dst_double[i];
    }
    
    printf("Float checksum: %f (sum1=%f, sum2=%f)\n", 
           checksum_float, sum1, sum2);
    printf("Double checksum: %lf (sum=%lf)\n", 
           checksum_double, sum_double);
    
    free(src);
    free(dst);
    free(src_double);
    free(dst_double);
    
    return 0;
}
