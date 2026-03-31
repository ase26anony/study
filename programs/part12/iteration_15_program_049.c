/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec -fno-inline -o vector_builtins vector_builtins.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")));

/* Helper function with explicit AVX2 target */
__attribute__((target("avx2")))
static void process_avx2(float* restrict dst, const float* restrict src, int n) {
    /* Assume aligned for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    /* Loop with multiple vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = src[i];
        val = __builtin_sqrtf(val);           /* sqrtf -> __builtin_sqrtf */
        val = __builtin_sinf(val);            /* sinf -> __builtin_sinf */
        val = __builtin_expf(val);            /* expf -> __builtin_expf */
        val = __builtin_fabsf(val);           /* fabsf -> __builtin_fabsf */
        dst[i] = val;
    }
}

/* Helper function with explicit SSE target */
__attribute__((target("sse4.2")))
static void process_sse(float* restrict dst, const float* restrict src, int n) {
    dst = (float*)__builtin_assume_aligned(dst, 16);
    src = (const float*)__builtin_assume_aligned(src, 16);
    
    /* Different built-in combination for SSE */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        val = __builtin_cosf(val);            /* cosf -> __builtin_cosf */
        val = __builtin_logf(val + 1.0f);     /* logf -> __builtin_logf */
        val = __builtin_powf(val, 0.5f);      /* powf -> __builtin_powf */
        dst[i] = val;
    }
}

/* Main target-cloned function */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    /* Process with different vector widths */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i += 4) {
        /* Vectorizable built-in calls */
        float val1 = __builtin_sinf(src[i]);
        float val2 = __builtin_cosf(src[i+1]);
        float val3 = __builtin_expf(src[i+2]);
        float val4 = __builtin_sqrtf(src[i+3]);
        
        /* Mix operations */
        dst[i]   = __builtin_fabsf(val1 * val2);
        dst[i+1] = __builtin_sqrtf(val2 * val3);
        dst[i+2] = __builtin_sinf(val3 + val4);
        dst[i+3] = __builtin_cosf(val4 - val1);
        
        sum += dst[i] + dst[i+1] + dst[i+2] + dst[i+3];
    }
    
    /* Call architecture-specific versions */
    if (n >= 256) {
        process_avx2(dst, src, n < 256 ? n : 256);
    }
    
    if (n >= 128) {
        process_sse(dst + 128, src + 128, n - 128 < 128 ? n - 128 : 128);
    }
    
    return sum;
}

/* Double precision version for AVX/AVX512 */
__attribute__((target("avx2")))
static double compute_double_vector(double* restrict dst, const double* restrict src, int n) {
    double sum = 0.0;
    
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (const double*)__builtin_assume_aligned(src, 32);
    
    for (int i = 0; i < n; i++) {
        /* Double precision built-ins */
        double val = src[i];
        val = __builtin_sin(val);             /* sin -> __builtin_sin */
        val = __builtin_sqrt(val);            /* sqrt -> __builtin_sqrt */
        val = __builtin_exp(val);             /* exp -> __builtin_exp */
        val = __builtin_fabs(val);            /* fabs -> __builtin_fabs */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function with mixed types and conversions */
__attribute__((target("avx2")))
static float mixed_types(float* restrict fout, int* restrict iout, 
                         const float* fin, const int* iin, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Type conversions with built-ins */
        float fval = fin[i];
        int ival = iin[i];
        
        /* Mixed operations that may trigger vectorization */
        fval = __builtin_ldexpf(fval, ival & 3);  /* ldexpf -> __builtin_ldexpf */
        fval = __builtin_sqrtf(__builtin_fabsf(fval));
        
        fout[i] = fval;
        iout[i] = (int)__builtin_floorf(fval);    /* floorf -> __builtin_floorf */
        
        sum += fval;
    }
    
    return sum;
}

int main(void) {
    const int N = 1024;
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst = aligned_alloc(32, N * sizeof(float));
    double* dsrc = aligned_alloc(32, N * sizeof(double));
    double* ddst = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        dsrc[i] = sin(i * 0.1);
    }
    
    /* Call target-cloned function (triggers vectorized built-in creation) */
    float sum1 = compute_vector(dst, src, N);
    
    /* Call double precision version */
    double sum2 = compute_double_vector(ddst, dsrc, N);
    
    /* Mixed types test */
    int* iarr = aligned_alloc(32, N * sizeof(int));
    int* iout = aligned_alloc(32, N * sizeof(int));
    for (int i = 0; i < N; i++) {
        iarr[i] = i % 16;
    }
    float sum3 = mixed_types(dst, iout, src, iarr, N);
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst[i] + ddst[i] + iout[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Sum1: %f, Sum2: %f, Sum3: %f\n", sum1, sum2, sum3);
    
    free(src);
    free(dst);
    free(dsrc);
    free(ddst);
    free(iarr);
    free(iout);
    
    return 0;
}
