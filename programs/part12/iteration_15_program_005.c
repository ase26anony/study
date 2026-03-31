#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Function with target clones to force generation of vectorized built-ins
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
float compute_vector(float* dst, const float* src, int n) {
    // Provide alignment hints for better vectorization
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    float sum = 0.0f;
    
    // Loop 1: Multiple vectorizable built-ins on float data
    for (int i = 0; i < n; i++) {
        // Chain of vectorizable math built-ins
        float val = src[i];
        val = __builtin_sqrtf(val);           // Vectorizable sqrt
        val = __builtin_sinf(val);            // Vectorizable sin
        val = __builtin_expf(val);            // Vectorizable exp
        val = __builtin_fabsf(val);           // Vectorizable fabs
        dst[i] = val;
        sum += val;
    }
    
    // Loop 2: Different built-ins with type mixing
    for (int i = 0; i < n; i += 2) {
        // Use powf which has vectorized versions
        float val = __builtin_powf(src[i], 1.5f);
        // Mix with conversion operations
        val = __builtin_ldexpf(val, 2);       // Multiply by 4 (2^2)
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

// Separate function targeting specific architectures
__attribute__((target("avx2,fma")))
double compute_vector_double(double* dst, const double* src, int n) {
    src = (const double*)__builtin_assume_aligned(src, 32);
    dst = (double*)__builtin_assume_aligned(dst, 32);
    
    double sum = 0.0;
    
    // Double precision vectorizable built-ins
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);            // Double precision sqrt
        val = __builtin_cos(val);             // Double precision cos
        val = __builtin_log(val);             // Double precision log
        val = __builtin_fabs(val);            // Double precision fabs
        dst[i] = val;
        sum += val;
    }
    
    // Another loop with pow for double
    for (int i = 0; i < n; i++) {
        dst[i] = __builtin_pow(src[i], 2.0);
        sum += dst[i];
    }
    
    return sum;
}

// Function with hidden visibility to interact with DECL_VISIBILITY
__attribute__((visibility("hidden")))
__attribute__((target_clones("avx2", "sse4.2")))
float compute_hidden(float* dst, const float* src, int n) {
    src = (const float*)__builtin_assume_aligned(src, 16);
    dst = (float*)__builtin_assume_aligned(dst, 16);
    
    float sum = 0.0f;
    
    // Use trigonometric built-ins
    for (int i = 0; i < n; i++) {
        float val = src[i];
        val = __builtin_sinf(val);
        val = __builtin_cosf(val);
        val = __builtin_tanf(val);
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

int main() {
    const int N = 1024;  // Divisible by common vector widths
    
    // Allocate aligned memory
    float* src_f = (float*)aligned_alloc(32, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(32, N * sizeof(float));
    double* src_d = (double*)aligned_alloc(32, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with mathematical values
    for (int i = 0; i < N; i++) {
        src_f[i] = 0.1f * i + 0.001f;  // Avoid zero for sqrt/log
        src_d[i] = 0.1 * i + 0.001;
    }
    
    // Call target-cloned functions to trigger vectorized built-in generation
    float sum1 = compute_vector(dst_f, src_f, N);
    double sum2 = compute_vector_double(dst_d, src_d, N);
    float sum3 = compute_hidden(dst_f, src_f, N);
    
    // Compute checksum to prevent dead code elimination
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst_f[i] + (float)dst_d[i];
    }
    
    printf("Results: sum1=%.6f, sum2=%.6f, sum3=%.6f, checksum=%.6f\n",
           sum1, (float)sum2, sum3, checksum);
    
    // Clean up
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
