/* Vectorized built-in test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force generation of vectorized built-in declarations */
#ifdef __GNUC__

/* Function with target clones - forces multiple versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline))
    __attribute__((visibility("hidden")));

/* Hidden visibility to interact with DECL_VISIBILITY */
__attribute__((visibility("hidden")))
static float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned for better vectorization */
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    /* Loop with multiple vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(x);          /* sqrtf -> __builtin_sqrtf */
        float z = __builtin_sinf(y);           /* sinf -> __builtin_sinf */
        float w = __builtin_expf(z);           /* expf -> __builtin_expf */
        float v = __builtin_fabsf(w);          /* fabsf -> __builtin_fabsf */
        float u = __builtin_powf(v, 0.5f);     /* powf -> __builtin_powf */
        
        dst[i] = u;
        sum += u;
    }
    
    /* Second loop with different built-ins */
    for (int i = 0; i < n; i += 2) {
        /* Use logf and cosf */
        float x = dst[i];
        float y = __builtin_logf(x + 1.0f);    /* logf -> __builtin_logf */
        float z = __builtin_cosf(y);           /* cosf -> __builtin_cosf */
        dst[i] = z;
        sum += z;
    }
    
    return sum;
}

/* AVX2-specific version with explicit target */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* dst, const double* src, int n) 
    __attribute__((noinline));

__attribute__((target("avx2,fma")))
static double compute_vector_double(double* dst, const double* src, int n) {
    double sum = 0.0;
    
    src = (const double*)__builtin_assume_aligned(src, 32);
    dst = (double*)__builtin_assume_aligned(dst, 32);
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double x = src[i];
        double y = __builtin_sqrt(x);          /* sqrt -> __builtin_sqrt */
        double z = __builtin_sin(y);           /* sin -> __builtin_sin */
        double w = __builtin_exp(z);           /* exp -> __builtin_exp */
        double v = __builtin_fabs(w);          /* fabs -> __builtin_fabs */
        double u = __builtin_pow(v, 0.5);      /* pow -> __builtin_pow */
        
        dst[i] = u;
        sum += u;
    }
    
    /* Mixed operations with ldexp */
    for (int i = 0; i < n; i++) {
        double x = dst[i];
        double y = __builtin_log(x + 1.0);     /* log -> __builtin_log */
        double z = __builtin_cos(y);           /* cos -> __builtin_cos */
        /* ldexp is vectorizable on some architectures */
        double w = __builtin_ldexp(z, 2);      /* ldexp -> __builtin_ldexp */
        dst[i] = w;
        sum += w;
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2")))
static float compute_vector_sse(float* dst, const float* src, int n) 
    __attribute__((noinline));

__attribute__((target("sse4.2")))
static float compute_vector_sse(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    src = (const float*)__builtin_assume_aligned(src, 16);
    dst = (float*)__builtin_assume_aligned(dst, 16);
    
    for (int i = 0; i < n; i++) {
        /* Different combination of built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(__builtin_fabsf(x));
        float z = __builtin_sinf(__builtin_expf(y));
        dst[i] = z;
        sum += z;
    }
    
    return sum;
}

#endif /* __GNUC__ */

int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src_f = aligned_alloc(32, N * sizeof(float));
    float* dst_f = aligned_alloc(32, N * sizeof(float));
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
#ifdef __GNUC__
    /* Call all vectorized versions */
    sum_f += compute_vector(dst_f, src_f, N);
    sum_d += compute_vector_double(dst_d, src_d, N);
    sum_f += compute_vector_sse(dst_f, src_f, N);
    
    /* Additional calls with different sizes to trigger different vectorization factors */
    sum_f += compute_vector(dst_f, src_f, N/2);
    sum_f += compute_vector(dst_f, src_f, N/4);
#endif
    
    /* Compute checksum to prevent dead code elimination */
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_f += dst_f[i];
        checksum_d += dst_d[i];
    }
    
    printf("Float checksum: %f (sum_f: %f)\n", checksum_f, sum_f);
    printf("Double checksum: %f (sum_d: %f)\n", checksum_d, sum_d);
    
    /* Clean up */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
