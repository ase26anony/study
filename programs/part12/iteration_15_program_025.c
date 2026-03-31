#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Function with target clones for different vector ISAs
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
float compute_vector(float* dst, const float* src, int n) {
    // Provide alignment hints to help vectorization
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    float sum = 0.0f;
    
    // Loop 1: Multiple vectorizable math built-ins on float
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        // Chain of vectorizable built-ins
        float val = src[i];
        val = __builtin_sqrtf(val);          // sqrtf has vectorized version
        val = __builtin_sinf(val);           // sinf has vectorized version
        val = __builtin_expf(val);           // expf has vectorized version
        val = __builtin_fabsf(val);          // fabsf has vectorized version
        dst[i] = val;
        sum += val;
    }
    
    // Loop 2: Different built-in combinations
    #pragma GCC unroll 2
    for (int i = 0; i < n; i += 2) {
        // Use powf which may have vectorized version
        float val1 = __builtin_powf(src[i], 1.5f);
        float val2 = __builtin_powf(src[i+1], 2.0f);
        dst[i] = val1 * val2;
        sum += dst[i];
    }
    
    return sum;
}

// AVX2-specific version with explicit target attribute
__attribute__((target("avx2,fma"), visibility("hidden")))
double compute_vector_avx2(double* dst, const double* src, int n) {
    src = (const double*)__builtin_assume_aligned(src, 32);
    dst = (double*)__builtin_assume_aligned(dst, 32);
    
    double sum = 0.0;
    
    // Double precision vectorizable built-ins
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);           // sqrt has vectorized version
        val = __builtin_cos(val);            // cos has vectorized version
        val = __builtin_log(val);            // log has vectorized version
        dst[i] = val;
        sum += val;
    }
    
    // Mix with ldexp for type conversion consideration
    for (int i = 0; i < n; i++) {
        int exp = i % 10;
        dst[i] = __builtin_ldexp(dst[i], exp);
        sum += dst[i];
    }
    
    return sum;
}

// SSE4.2-specific version
__attribute__((target("sse4.2"), visibility("default")))
float compute_vector_sse(float* dst, const float* src, int n) {
    src = (const float*)__builtin_assume_aligned(src, 16);
    dst = (float*)__builtin_assume_aligned(dst, 16);
    
    float sum = 0.0f;
    
    // Different built-in combinations
    for (int i = 0; i < n; i++) {
        float val = src[i];
        // Trigonometric functions that may have vectorized versions
        val = __builtin_sinf(val);
        val = __builtin_cosf(val);
        val = __builtin_tanf(val);
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

// Main test function with hidden visibility
__attribute__((visibility("hidden")))
int main() {
    const int N = 1024;
    float* src_f = (float*)aligned_alloc(32, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(32, N * sizeof(float));
    double* src_d = (double*)aligned_alloc(32, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(32, N * sizeof(double));
    
    // Initialize with trigonometric values
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    // Call target-cloned function (multiple versions will be generated)
    float sum1 = compute_vector(dst_f, src_f, N);
    
    // Call AVX2-specific version
    double sum2 = compute_vector_avx2(dst_d, src_d, N);
    
    // Call SSE-specific version
    float sum3 = compute_vector_sse(dst_f, src_f, N);
    
    // Compute checksum to prevent dead code elimination
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst_f[i] + (float)dst_d[i];
    }
    
    printf("Results: sum1=%.6f, sum2=%.6f, sum3=%.6f, checksum=%.6f\n",
           sum1, (float)sum2, sum3, checksum);
    
    // Additional test with different loop structures
    // Unrolled loop with multiple built-ins
    #pragma GCC unroll 8
    for (int i = 0; i < N; i++) {
        float x = src_f[i];
        // Multiple vectorizable operations
        dst_f[i] = __builtin_sinf(__builtin_sqrtf(__builtin_fabsf(x)));
    }
    
    // Another checksum
    float checksum2 = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum2 += dst_f[i];
    }
    printf("Checksum2: %.6f\n", checksum2);
    
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
