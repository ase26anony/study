#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones - forces generation of multiple versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((visibility("hidden")))
    __attribute__((optimize("O3")))
{
    /* Alignment hints for better vectorization */
    src = __builtin_assume_aligned(src, 32);
    dst = __builtin_assume_aligned(dst, 32);
    
    float sum = 0.0f;
    
    /* Loop 1: Multiple vectorizable built-ins on float data */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float temp = __builtin_sqrtf(src[i]);
        temp = __builtin_sinf(temp);
        temp = __builtin_expf(temp);
        temp = __builtin_fabsf(temp);
        dst[i] = temp;
        sum += temp;
    }
    
    /* Loop 2: Different built-ins to trigger more vectorized versions */
    for (int i = 0; i < n/2; i++) {
        float a = __builtin_powf(src[i*2], 1.5f);
        float b = __builtin_logf(src[i*2 + 1] + 1.0f);
        dst[i*2] = __builtin_cosf(a);
        dst[i*2 + 1] = __builtin_sinf(b);
        sum += dst[i*2] + dst[i*2 + 1];
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* dst, const double* src, int n)
    __attribute__((visibility("hidden")))
{
    src = __builtin_assume_aligned(src, 32);
    dst = __builtin_assume_aligned(dst, 32);
    
    double sum = 0.0;
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double temp = __builtin_sqrt(src[i]);
        temp = __builtin_sin(temp);
        temp = __builtin_exp(temp);
        temp = __builtin_fabs(temp);
        dst[i] = temp;
        sum += temp;
    }
    
    /* Mixed operations with ldexp */
    for (int i = 0; i < n; i += 2) {
        double a = __builtin_pow(src[i], 2.0);
        double b = __builtin_ldexp(src[i+1], 2);  /* src[i+1] * 4.0 */
        dst[i] = __builtin_cos(a);
        dst[i+1] = __builtin_sin(b);
        sum += dst[i] + dst[i+1];
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2")))
static float compute_vector_sse(float* dst, const float* src, int n)
{
    src = __builtin_assume_aligned(src, 16);
    dst = __builtin_assume_aligned(dst, 16);
    
    float sum = 0.0f;
    
    /* Different loop structure to trigger different vectorization */
    for (int i = 0; i < n; i += 4) {
        dst[i]   = __builtin_sqrtf(__builtin_fabsf(src[i]));
        dst[i+1] = __builtin_expf(__builtin_fabsf(src[i+1]));
        dst[i+2] = __builtin_sinf(__builtin_fabsf(src[i+2]));
        dst[i+3] = __builtin_cosf(__builtin_fabsf(src[i+3]));
        sum += dst[i] + dst[i+1] + dst[i+2] + dst[i+3];
    }
    
    return sum;
}

int main(void) {
    const int N = 1024;
    float* src_f = aligned_alloc(32, N * sizeof(float));
    float* dst_f = aligned_alloc(32, N * sizeof(float));
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    /* Call all vectorized versions */
    sum_f += compute_vector(dst_f, src_f, N);
    sum_d += compute_vector_double(dst_d, src_d, N);
    sum_f += compute_vector_sse(dst_f, src_f, N);
    
    /* Additional loops in main to trigger more vectorization contexts */
    {
        /* Vectorizable loop with built-ins directly in main */
        float local_src[N];
        float local_dst[N];
        
        for (int i = 0; i < N; i++) {
            local_src[i] = cosf(i * 0.05f);
        }
        
        for (int i = 0; i < N; i++) {
            local_dst[i] = __builtin_sqrtf(__builtin_fabsf(local_src[i]));
            sum_f += local_dst[i];
        }
    }
    
    /* Compute checksum */
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_f += dst_f[i];
        checksum_d += dst_d[i];
    }
    
    printf("Float checksum: %f\n", checksum_f);
    printf("Double checksum: %f\n", checksum_d);
    printf("Function sums: float=%f, double=%f\n", sum_f, sum_d);
    
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
