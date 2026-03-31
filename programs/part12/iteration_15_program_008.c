/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec -fno-inline -fdump-tree-vect -o vector_builtins vector_builtins.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SIZE 1024
#define ALIGNMENT 64

/* Function with target clones to force generation of vectorized built-ins */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")));

/* Hidden visibility to interact with DECL_VISIBILITY logic */
__attribute__((visibility("hidden")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    src = __builtin_assume_aligned(src, ALIGNMENT);
    dst = __builtin_assume_aligned(dst, ALIGNMENT);
    
    /* Loop 1: Multiple vectorizable math built-ins on float */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable built-ins - GCC should create vector versions */
        float val = src[i];
        val = __builtin_sqrtf(val);           /* sqrtf vectorization */
        val = __builtin_sinf(val);            /* sinf vectorization */
        val = __builtin_expf(val);            /* expf vectorization */
        val = __builtin_fabsf(val);           /* fabsf vectorization */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n) {
    double sum = 0.0;
    
    src = __builtin_assume_aligned(src, ALIGNMENT);
    dst = __builtin_assume_aligned(dst, ALIGNMENT);
    
    /* Loop 2: Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);            /* sqrt vectorization */
        val = __builtin_sin(val);             /* sin vectorization */
        val = __builtin_cos(val);             /* cos vectorization */
        val = __builtin_pow(val, 1.5);        /* pow vectorization */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2")))
static float compute_vector_mixed(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    src = __builtin_assume_aligned(src, 16);  /* SSE alignment */
    dst = __builtin_assume_aligned(dst, 16);
    
    /* Loop 3: Mixed operations including type conversions */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        val = __builtin_sqrtf(val);
        
        /* Use ldexp for conversion-like operation */
        int exp = (int)(val * 10) % 10;
        val = __builtin_ldexpf(val, exp);     /* ldexp vectorization */
        
        val = __builtin_logf(val + 1.0f);     /* logf vectorization */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Default version without specific vector extensions */
__attribute__((target("default")))
static float compute_vector_default(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    /* Simple loop that should still vectorize on most targets */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        val = __builtin_sqrtf(val);
        val = __builtin_sinf(val);
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

int main(void) {
    /* Aligned allocations for better vectorization */
    float* src_f = aligned_alloc(ALIGNMENT, SIZE * sizeof(float));
    float* dst_f = aligned_alloc(ALIGNMENT, SIZE * sizeof(float));
    double* src_d = aligned_alloc(ALIGNMENT, SIZE * sizeof(double));
    double* dst_d = aligned_alloc(ALIGNMENT, SIZE * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with mathematical values to avoid NaNs/Infs */
    for (int i = 0; i < SIZE; i++) {
        src_f[i] = 0.1f + 0.01f * (i % 100);  /* Range: 0.1 to 1.09 */
        src_d[i] = 0.1 + 0.01 * (i % 100);
    }
    
    float checksum = 0.0f;
    
    /* Call all versions to ensure they're compiled */
    checksum += compute_vector(dst_f, src_f, SIZE);
    checksum += compute_vector_default(dst_f, src_f, SIZE);
    checksum += compute_vector_mixed(dst_f, src_f, SIZE);
    
    /* Call double precision version */
    double dsum = compute_vector_double(dst_d, src_d, SIZE);
    checksum += (float)dsum;
    
    /* Additional loop in main to increase vectorization opportunities */
    {
        float* tmp = aligned_alloc(ALIGNMENT, SIZE * sizeof(float));
        if (tmp) {
            tmp = __builtin_assume_aligned(tmp, ALIGNMENT);
            
            /* Another vectorizable loop with powf */
            for (int i = 0; i < SIZE; i++) {
                tmp[i] = __builtin_powf(src_f[i], 2.0f);
            }
            
            /* Use the result */
            for (int i = 0; i < SIZE; i++) {
                checksum += tmp[i];
            }
            
            free(tmp);
        }
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %f\n", checksum);
    
    /* Verify some results */
    printf("Sample results: dst_f[0]=%f, dst_f[100]=%f\n", dst_f[0], dst_f[100]);
    printf("Double results: dst_d[0]=%f, dst_d[100]=%f\n", dst_d[0], dst_d[100]);
    
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
