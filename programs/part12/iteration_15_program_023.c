#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Function with target clones to force generation of multiple vectorized versions
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) {
    // Assume aligned for better vectorization
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    // Loop with multiple vectorizable built-in calls
    for (int i = 0; i < n; i++) {
        // Chain of vectorizable math built-ins
        float val = src[i];
        val = __builtin_sqrtf(val);          // Vectorizable sqrt
        val = __builtin_sinf(val);           // Vectorizable sin
        val = __builtin_fabsf(val);          // Vectorizable fabs
        val = __builtin_expf(val * 0.5f);    // Vectorizable exp
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

// Another function with different target attribute for double precision
__attribute__((target("avx2")))
static double compute_vector_double(double* dst, const double* src, int n) {
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    // Double precision vectorizable built-ins
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);           // Double precision sqrt
        val = __builtin_cos(val);            // Double precision cos
        val = __builtin_fabs(val);           // Double precision fabs
        val = __builtin_pow(val, 1.5);       // Vectorizable pow
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

// Function with mixed types and conversions
__attribute__((target("avx512f")))
static float compute_mixed(float* dst, const float* src, int n) {
    dst = __builtin_assume_aligned(dst, 64);
    src = __builtin_assume_aligned(src, 64);
    
    float sum = 0.0f;
    
    // Mix of operations that might trigger different vectorized versions
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        // Trigonometric functions
        val = __builtin_sinf(val);
        val = __builtin_cosf(val);
        
        // Exponential and logarithmic
        val = __builtin_expf(val);
        val = __builtin_logf(val + 1.0f);
        
        // Power function
        val = __builtin_powf(val, 0.75f);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

// Function with visibility attribute to interact with DECL_VISIBILITY
__attribute__((visibility("hidden"), target("sse4.2")))
static float compute_hidden(float* dst, const float* src, int n) {
    dst = __builtin_assume_aligned(dst, 16);
    src = __builtin_assume_aligned(src, 16);
    
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        // Use hyperbolic functions which also have vectorized versions
        float val = src[i];
        val = __builtin_sinhf(val);
        val = __builtin_coshf(val);
        val = __builtin_tanhf(val);
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

int main() {
    const int N = 1024;
    
    // Allocate aligned memory
    float* src = aligned_alloc(64, N * sizeof(float));
    float* dst1 = aligned_alloc(64, N * sizeof(float));
    float* dst2 = aligned_alloc(64, N * sizeof(float));
    float* dst3 = aligned_alloc(64, N * sizeof(float));
    
    double* src_double = aligned_alloc(64, N * sizeof(double));
    double* dst_double = aligned_alloc(64, N * sizeof(double));
    
    // Initialize with trigonometric values
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        src_double[i] = sin(i * 0.1);
    }
    
    // Call all vectorized functions
    float sum1 = compute_vector(dst1, src, N);
    float sum2 = compute_mixed(dst2, src, N);
    float sum3 = compute_hidden(dst3, src, N);
    double sum_double = compute_vector_double(dst_double, src_double, N);
    
    // Compute checksums to prevent dead code elimination
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i];
        checksum_double += dst_double[i];
    }
    
    printf("Float checksum: %f (sums: %f, %f, %f)\n", 
           checksum, sum1, sum2, sum3);
    printf("Double checksum: %lf (sum: %lf)\n", 
           checksum_double, sum_double);
    
    // Free memory
    free(src);
    free(dst1);
    free(dst2);
    free(dst3);
    free(src_double);
    free(dst_double);
    
    return 0;
}
