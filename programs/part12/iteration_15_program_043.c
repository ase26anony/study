#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* AVX2-specific version */
__attribute__((target("avx2,fma"), noinline, visibility("hidden")))
static float compute_vector_avx2(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    const float* __restrict s = __builtin_assume_aligned(src, 32);
    float* __restrict d = __builtin_assume_aligned(dst, 32);
    
    /* Loop with vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = s[i];
        val = __builtin_sqrtf(val);           /* sqrtf -> __builtin_sqrtf */
        val = __builtin_sinf(val);            /* sinf -> __builtin_sinf */
        val = __builtin_expf(val);            /* expf -> __builtin_expf */
        val = __builtin_fabsf(val);           /* fabsf -> __builtin_fabsf */
        val = __builtin_logf(val + 1.0f);     /* logf -> __builtin_logf */
        d[i] = val;
        sum += val;
    }
    
    /* Second loop with powf */
    for (int i = 0; i < n; i += 2) {
        d[i] = __builtin_powf(d[i], 1.5f);    /* powf -> __builtin_powf */
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2"), noinline, visibility("hidden")))
static float compute_vector_sse(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    const float* __restrict s = __builtin_assume_aligned(src, 16);
    float* __restrict d = __builtin_assume_aligned(dst, 16);
    
    for (int i = 0; i < n; i++) {
        float val = s[i];
        val = __builtin_sqrtf(val);
        val = __builtin_cosf(val);            /* cosf -> __builtin_cosf */
        val = __builtin_fabsf(val);
        d[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Default version with mixed precision */
__attribute__((target("default"), noinline, visibility("hidden")))
static float compute_vector_default(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Mix float and double operations */
    for (int i = 0; i < n; i++) {
        double dval = (double)src[i];
        dval = __builtin_sqrt(dval);          /* sqrt -> __builtin_sqrt */
        dval = __builtin_sin(dval);           /* sin -> __builtin_sin */
        dst[i] = (float)dval;
        sum += dst[i];
    }
    
    /* Loop with ldexp for type conversion built-in */
    for (int i = 0; i < n; i++) {
        int exp;
        float frac = __builtin_frexpf(dst[i], &exp);  /* frexpf -> __builtin_frexpf */
        dst[i] = __builtin_ldexpf(frac, exp);         /* ldexpf -> __builtin_ldexpf */
    }
    
    return sum;
}

/* Main target-cloned function */
static float compute_vector(float* dst, const float* src, int n) {
    /* This will be cloned based on target_clones attribute */
    float sum = 0.0f;
    
    /* Use different built-ins to trigger various vectorized versions */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Complex expression with multiple vectorizable built-ins */
        val = __builtin_sinf(__builtin_sqrtf(val));
        val = __builtin_expf(val);
        val = __builtin_powf(val, 0.5f);
        
        dst[i] = val;
        sum += val;
    }
    
    /* Additional loop with different built-in */
    for (int i = 0; i < n; i += 4) {
        dst[i] = __builtin_ceilf(dst[i]);     /* ceilf -> __builtin_ceilf */
    }
    
    return sum;
}

/* Function with AVX512 target */
__attribute__((target("avx512f"), noinline, visibility("hidden")))
static void compute_double_vector(double* dst, const double* src, int n) {
    /* Double precision vectorization */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);
        val = __builtin_sin(val);
        val = __builtin_exp(val);
        val = __builtin_fabs(val);
        dst[i] = val;
    }
}

int main(void) {
    const int N = 1024;
    float* src = aligned_alloc(64, N * sizeof(float));
    float* dst = aligned_alloc(64, N * sizeof(float));
    double* dsrc = aligned_alloc(64, N * sizeof(double));
    double* ddst = aligned_alloc(64, N * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        dsrc[i] = sin(i * 0.1);
    }
    
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    
    /* Call different target versions */
    sum1 = compute_vector(dst, src, N);
    sum2 = compute_vector_avx2(dst, src, N);
    sum3 = compute_vector_sse(dst, src, N);
    
    /* Call double precision version */
    compute_double_vector(ddst, dsrc, N);
    
    /* Compute checksum to prevent optimization */
    float checksum = 0.0f;
    double dchecksum = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
        dchecksum += ddst[i];
    }
    
    checksum += sum1 + sum2 + sum3;
    
    printf("Checksum: %f\n", checksum);
    printf("Double checksum: %f\n", dchecksum);
    
    free(src);
    free(dst);
    free(dsrc);
    free(ddst);
    
    return 0;
}
