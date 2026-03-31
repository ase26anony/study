/* test_vectorized_builtins.c */
/* Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fopt-info-vec -fno-math-errno test_vectorized_builtins.c -lm -o test_vectorized_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <omp.h>

#define SIZE 1024
#define ALIGN __attribute__((aligned(32)))

/* Helper to prevent compile-time computation */
static inline float simple_rand(int seed) {
    return (seed * 0.5f) - 0.25f;
}

/* Function with hidden visibility containing vectorizable built-ins */
static void ALIGN __attribute__((visibility("hidden"), used, nothrow))
process_hidden(float *restrict out, const float *restrict in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]) + cosf(in[i]);
    }
}

/* Always-inline function with multiple built-ins */
static inline ALIGN __attribute__((always_inline, used))
void process_pow_exp(float *restrict out, const float *restrict in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 1.5f) + expf(in[i]);
    }
}

/* Function with OpenMP declare simd */
#pragma omp declare simd uniform(a) linear(i)
static float ALIGN process_element(float a, int i) {
    return sqrtf(a + i) * logf(fabsf(a) + 1.0f);
}

/* Memory-intensive function using builtin memcpy */
static void ALIGN __attribute__((used))
process_memcpy(float *restrict dst, const float *restrict src, int n) {
    const int chunk = 16;
    for (int i = 0; i < n; i += chunk) {
        int size = (n - i) > chunk ? chunk : (n - i);
        __builtin_memcpy(dst + i, src + i, size * sizeof(float));
    }
}

/* Conditional AVX2 path with fallback */
static void ALIGN process_conditional(double *restrict out, const double *restrict in, int n) {
    if (__builtin_cpu_supports("avx2")) {
        /* Vector intrinsic path - compiler may still consider builtin vectorization */
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(in + i);
            __m256d result = _mm256_sqrt_pd(vec);
            _mm256_store_pd(out + i, result);
        }
    } else {
        /* Scalar fallback with builtin calls */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sqrt(in[i]);
        }
    }
}

/* Dead code path that still gets analyzed */
static void ALIGN dead_code_path(float *restrict out, const float *restrict in, int n) {
    if (0) {  /* Dead code, but frontend still processes */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sinf(in[i]) * cosf(in[i]) + sqrtf(in[i]);
        }
    }
}

/* Function with mixed types and type punning */
static void ALIGN process_mixed(float *restrict fout, int *restrict iout, 
                                const float *restrict fin, int n) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp parallel for simd reduction(+:converter.i)
    for (int i = 0; i < n; i++) {
        fout[i] = sinf(fin[i]);
        converter.f = fin[i];
        iout[i] = converter.i + __builtin_ilogb(fin[i]);
    }
}

/* Switch between different vectorization candidates */
static void ALIGN process_switch(int mode, float *restrict out, 
                                 const float *restrict in, int n) {
    switch (mode) {
        case 0:
            process_hidden(out, in, n);
            break;
        case 1: {
            /* Inline expansion of builtins */
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = tanf(in[i]) + asinf(in[i] * 0.5f);
            }
            break;
        }
        case 2:
            process_pow_exp(out, in, n);
            break;
        default:
            /* Default path with strlen-like pattern */
            char buffer[256];
            for (int i = 0; i < n && i < 256; i++) {
                buffer[i] = (char)(in[i] * 100);
            }
            buffer[n < 256 ? n : 255] = '\0';
            int len = __builtin_strlen(buffer);
            out[0] = (float)len;
            break;
    }
}

/* Main test driver */
int main(void) {
    /* Aligned arrays */
    float ALIGN src_float[SIZE];
    float ALIGN dst_float[SIZE];
    double ALIGN src_double[SIZE];
    double ALIGN dst_double[SIZE];
    int ALIGN dst_int[SIZE];
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        src_float[i] = simple_rand(i);
        src_double[i] = (double)simple_rand(i) * 2.0;
    }
    
    float checksum = 0.0f;
    
    /* Test 1: Math-intensive with OpenMP SIMD */
    #pragma omp parallel for simd reduction(+:checksum)
    for (int i = 0; i < SIZE; i++) {
        dst_float[i] = sinf(src_float[i]) + cosf(src_float[i]);
        checksum += dst_float[i];
    }
    
    /* Test 2: Hidden visibility function */
    process_hidden(dst_float, src_float, SIZE);
    for (int i = 0; i < SIZE; i++) checksum += dst_float[i];
    
    /* Test 3: Memory operations with builtin memcpy */
    process_memcpy(dst_float, src_float, SIZE);
    for (int i = 0; i < SIZE; i++) checksum += dst_float[i];
    
    /* Test 4: Conditional AVX2 path */
    process_conditional(dst_double, src_double, SIZE);
    for (int i = 0; i < SIZE; i++) checksum += (float)dst_double[i];
    
    /* Test 5: Mixed types */
    process_mixed(dst_float, dst_int, src_float, SIZE);
    for (int i = 0; i < SIZE; i++) checksum += dst_float[i] + (float)dst_int[i];
    
    /* Test 6: Switch through different modes */
    for (int mode = 0; mode < 3; mode++) {
        process_switch(mode, dst_float, src_float, SIZE);
        for (int i = 0; i < SIZE; i++) checksum += dst_float[i];
    }
    
    /* Test 7: OpenMP declare simd function */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        dst_float[i] = process_element(src_float[i], i);
        checksum += dst_float[i];
    }
    
    /* Dead code path (not executed but analyzed) */
    dead_code_path(dst_float, src_float, SIZE);
    
    printf("Checksum: %f\n", checksum);
    printf("Sample values: %f, %f, %f\n", dst_float[0], dst_float[SIZE/2], dst_float[SIZE-1]);
    
    return 0;
}
