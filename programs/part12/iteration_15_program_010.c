/* Vectorized built-in test for GCC target hooks coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force visibility control that might interact with DECL_VISIBILITY */
__attribute__((visibility("default")))
float compute_vector(float* dst, const float* src, int n) 
    __attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")));

/* Function with explicit target attributes to ensure vectorization attempts */
__attribute__((target("default")))
float compute_vector_default(float* dst, const float* src, int n) {
    return compute_vector(dst, src, n);
}

__attribute__((target("avx2")))
float compute_vector_avx2(float* dst, const float* src, int n) {
    return compute_vector(dst, src, n);
}

__attribute__((target("avx512f")))
float compute_vector_avx512(float* dst, const float* src, int n) {
    return compute_vector(dst, src, n);
}

__attribute__((target("sse4.2")))
float compute_vector_sse42(float* dst, const float* src, int n) {
    return compute_vector(dst, src, n);
}

/* Main vectorizable function with built-in calls */
__attribute__((noinline, optimize("O3")))
float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    float* __restrict dst_aligned = __builtin_assume_aligned(dst, 32);
    const float* __restrict src_aligned = __builtin_assume_aligned(src, 32);
    
    /* Loop 1: Vectorizable math built-ins on float */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable built-ins:
           sqrt -> sin -> fabs -> exp */
        float val = src_aligned[i];
        val = __builtin_sqrtf(val);          /* sqrtf vectorized version */
        val = __builtin_sinf(val);           /* sinf vectorized version */
        val = __builtin_fabsf(val);          /* fabsf vectorized version */
        val = __builtin_expf(val * 0.1f);    /* expf vectorized version */
        dst_aligned[i] = val;
        sum += val;
    }
    
    /* Loop 2: Different built-in combinations */
    for (int i = 0; i < n; i += 2) {
        /* Use powf built-in */
        float val1 = __builtin_powf(src_aligned[i], 1.5f);
        float val2 = __builtin_powf(src_aligned[i+1], 2.0f);
        dst_aligned[i] = val1 + val2;
        sum += dst_aligned[i];
    }
    
    /* Loop 3: Mixed precision operations */
    double dsum = 0.0;
    for (int i = 0; i < n && i < 512; i++) {
        /* Convert to double, use double built-ins, convert back */
        double dval = (double)src_aligned[i];
        dval = __builtin_sqrt(dval);         /* sqrt vectorized version */
        dval = __builtin_sin(dval);          /* sin vectorized version */
        dval = __builtin_exp(dval * 0.05);   /* exp vectorized version */
        dst_aligned[i] = (float)dval;
        dsum += dval;
    }
    sum += (float)dsum;
    
    /* Loop 4: Trigonometric built-ins */
    for (int i = n/2; i < n; i++) {
        float val = src_aligned[i];
        val = __builtin_cosf(val);           /* cosf vectorized version */
        val = __builtin_logf(val + 2.0f);    /* logf vectorized version */
        dst_aligned[i] = val;
        sum += val;
    }
    
    /* Loop 5: Use ldexp for type conversion built-in */
    for (int i = 0; i < n && i < 256; i++) {
        float val = src_aligned[i];
        int exp = i % 10;
        val = __builtin_ldexpf(val, exp);    /* ldexpf vectorized version */
        dst_aligned[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Another function with different target attribute to increase coverage */
__attribute__((target("avx"), noinline, visibility("hidden")))
void process_vector_hidden(float* arr, int n) {
    /* Different set of built-ins */
    for (int i = 0; i < n; i++) {
        arr[i] = __builtin_tanf(__builtin_fabsf(arr[i]));
        arr[i] = __builtin_asinf(arr[i] * 0.5f);
    }
}

/* Function with explicit AVX512 target */
__attribute__((target("avx512f"), noinline))
double compute_double_vector(double* dst, const double* src, int n) {
    double sum = 0.0;
    
    double* __restrict dst_aligned = __builtin_assume_aligned(dst, 64);
    const double* __restrict src_aligned = __builtin_assume_aligned(src, 64);
    
    for (int i = 0; i < n; i++) {
        double val = src_aligned[i];
        val = __builtin_sqrt(val);
        val = __builtin_sin(val);
        val = __builtin_cos(val);
        val = __builtin_exp(val);
        dst_aligned[i] = val;
        sum += val;
    }
    
    return sum;
}

int main() {
    const int N = 1024;
    float* src = aligned_alloc(64, N * sizeof(float));
    float* dst = aligned_alloc(64, N * sizeof(float));
    double* dsrc = aligned_alloc(64, N/2 * sizeof(double));
    double* ddst = aligned_alloc(64, N/2 * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f) + 1.0f;  /* Ensure positive for sqrt */
    }
    
    for (int i = 0; i < N/2; i++) {
        dsrc[i] = sin(i * 0.05) + 1.5;
    }
    
    /* Call target-cloned functions multiple times */
    float total_sum = 0.0f;
    
    /* Call different target versions */
    total_sum += compute_vector_default(dst, src, N);
    total_sum += compute_vector_avx2(dst, src, N);
    total_sum += compute_vector_avx512(dst, src, N);
    total_sum += compute_vector_sse42(dst, src, N);
    
    /* Call the target_clones function directly */
    total_sum += compute_vector(dst, src, N);
    
    /* Process with hidden visibility function */
    process_vector_hidden(dst, N);
    
    /* Process double precision with AVX512 target */
    double dsum = compute_double_vector(ddst, dsrc, N/2);
    total_sum += (float)dsum;
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Total sum: %f\n", total_sum);
    printf("Checksum: %f\n", checksum);
    
    /* Prevent dead code elimination */
    if (checksum > 1000.0f) {
        printf("Result is large\n");
    }
    
    free(src);
    free(dst);
    free(dsrc);
    free(ddst);
    
    return 0;
}
