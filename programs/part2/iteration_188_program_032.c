/* 
 * This program is designed to trigger GCC's vectorization of built-in functions,
 * specifically to exercise the flag-setting block in default_builtin_vectorized_function.
 * It combines multiple techniques: OpenMP SIMD pragmas, architecture-specific
 * intrinsics, visibility attributes, and complex control flow to maximize the
 * chances of the target hook being invoked.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Architecture-specific headers */
#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#include <cpuid.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Size known at compile-time for vectorization analysis */
#define N 1024
#define M 256

/* --------------------------------------------------------------------------
 * Helper functions with attributes that may interact with declaration creation
 * -------------------------------------------------------------------------- */

/* Hidden visibility function - matches DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
static void __attribute__((visibility("hidden"), used, nothrow))
process_hidden(float *restrict out, const float *restrict in, int n) {
    /* Loop with math built-ins - likely candidate for vectorization */
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        out[i] = sinf(in[i]) + cosf(in[i]);
    }
}

/* Static function with always_inline - forces analysis in multiple contexts */
static inline __attribute__((always_inline)) void
process_pow(float *restrict out, const float *restrict in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        out[i] = powf(in[i], 2.5f);
    }
}

static inline __attribute__((always_inline)) void
process_exp_log(double *restrict out, const double *restrict in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        out[i] = exp(in[i]) + log(fabs(in[i]) + 1.0);
    }
}

/* --------------------------------------------------------------------------
 * Functions using __builtin_ prefix for memory operations
 * -------------------------------------------------------------------------- */

static void __attribute__((used))
builtin_memcpy_loop(char *restrict dst, const char *restrict src, int n) {
    /* Using __builtin_memcpy in a vectorizable loop */
    for (int i = 0; i < n; i += 64) {
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

static int __attribute__((used))
builtin_strlen_sum(const char *strings[], int count) {
    int total = 0;
    /* __builtin_strlen in a loop - may trigger vectorized strlen */
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < count; ++i) {
        total += __builtin_strlen(strings[i]);
    }
    return total;
}

/* --------------------------------------------------------------------------
 * Architecture-specific paths with fallbacks
 * -------------------------------------------------------------------------- */

#ifdef __x86_64__
static int cpu_supports_avx2(void) {
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 5)) != 0;
}

static void avx2_sqrt_vector(float *restrict out, const float *restrict in, int n) {
    /* AVX2 intrinsic path */
    for (int i = 0; i < n; i += 8) {
        __m256 vec = _mm256_load_ps(&in[i]);
        __m256 sqrt_vec = _mm256_sqrt_ps(vec);
        _mm256_store_ps(&out[i], sqrt_vec);
    }
}
#endif

static void scalar_sqrt_vector(float *restrict out, const float *restrict in, int n) {
    /* Scalar fallback with sqrtf built-in */
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        out[i] = sqrtf(in[i]);
    }
}

/* --------------------------------------------------------------------------
 * Complex control flow with multiple vectorization candidates
 * -------------------------------------------------------------------------- */

typedef enum {
    OP_SIN_COS,
    OP_POW,
    OP_EXP_LOG,
    OP_SQRT,
    OP_MEMCPY
} operation_t;

static void __attribute__((noinline))
dispatch_operation(operation_t op, void *out, const void *in, int n) {
    /* Switch to ensure compiler analyzes all paths */
    switch (op) {
        case OP_SIN_COS:
            process_hidden((float*)out, (const float*)in, n);
            break;
        case OP_POW:
            process_pow((float*)out, (const float*)in, n);
            break;
        case OP_EXP_LOG:
            process_exp_log((double*)out, (const double*)in, n);
            break;
        case OP_SQRT:
            /* Conditional CPU dispatch - both paths analyzed */
            #ifdef __x86_64__
            if (cpu_supports_avx2()) {
                avx2_sqrt_vector((float*)out, (const float*)in, n);
            } else
            #endif
            {
                scalar_sqrt_vector((float*)out, (const float*)in, n);
            }
            break;
        case OP_MEMCPY:
            builtin_memcpy_loop((char*)out, (const char*)in, n);
            break;
    }
    
    /* Dead code path that still contains vectorizable built-ins */
    if (0) {
        float dead_out[N] ALIGN_32;
        const float dead_in[N] ALIGN_32 = {0};
        #pragma omp simd
        for (int i = 0; i < N; ++i) {
            dead_out[i] = sinf(dead_in[i]) * cosf(dead_in[i]);
        }
    }
}

/* --------------------------------------------------------------------------
 * OpenMP SIMD declared function
 * -------------------------------------------------------------------------- */

#pragma omp declare simd uniform(a) linear(i)
static float simd_multiply_add(float a, float b, int i) {
    /* Uses fmaf which may have vectorized version */
    return fmaf(a, b, (float)i);
}

static void omp_simd_reduction(float *restrict data, int n) {
    float sum = 0.0f;
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        sum += simd_multiply_add(data[i], 1.5f, i);
    }
    /* Use sum to prevent elimination */
    data[0] = sum / n;
}

/* --------------------------------------------------------------------------
 * Mixed data types and type-punning
 * -------------------------------------------------------------------------- */

typedef union {
    float f[4];
    int i[4];
    #ifdef __x86_64__
    __m128 v;
    #endif
} vector_union ALIGN_16;

static void mixed_type_operations(void) {
    float fa[N] ALIGN_32;
    double da[N] ALIGN_64;
    int ia[N] ALIGN_32;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        fa[i] = (float)i + 0.5f;
        da[i] = (double)i * 0.25;
        ia[i] = i * 2;
    }
    
    /* Mixed type loops with built-ins */
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        fa[i] = sqrtf(fa[i]) + (float)__builtin_ilogb(da[i]);
    }
    
    /* Type-punning with union */
    vector_union vu;
    vu.f[0] = 1.0f; vu.f[1] = 2.0f; vu.f[2] = 3.0f; vu.f[3] = 4.0f;
    
    #ifdef __x86_64__
    __m128 vec = vu.v;
    vec = _mm_sqrt_ps(vec);
    _mm_store_ps(vu.f, vec);
    #endif
    
    /* Use results */
    fa[0] = vu.f[0];
}

/* --------------------------------------------------------------------------
 * Main function - orchestrates all test cases
 * -------------------------------------------------------------------------- */

int main(void) {
    /* Aligned arrays as required for vectorization */
    float float_array[N] ALIGN_32;
    double double_array[N] ALIGN_64;
    char char_array[N] ALIGN_32;
    char char_src[N] ALIGN_32;
    const char *strings[M];
    
    /* Initialize with pseudo-random pattern */
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        float_array[i] = (float)rand() / RAND_MAX * 10.0f;
        double_array[i] = (double)rand() / RAND_MAX * 10.0;
        char_array[i] = 'A' + (rand() % 26);
        char_src[i] = 'a' + (rand() % 26);
    }
    
    for (int i = 0; i < M; ++i) {
        char *str = malloc(32);
        snprintf(str, 32, "String%d_%d", i, rand() % 100);
        strings[i] = str;
    }
    
    printf("Starting vectorization tests...\n");
    
    /* 1. Math-intensive function with OpenMP SIMD */
    process_hidden(float_array, float_array, N);
    printf("  Hidden visibility function completed\n");
    
    /* 2. Memory/copy function with __builtin_memcpy */
    builtin_memcpy_loop(char_array, char_src, N);
    printf("  Builtin memcpy loop completed\n");
    
    /* 3. Conditional CPU dispatch with sqrt */
    dispatch_operation(OP_SQRT, float_array, float_array, N);
    printf("  Conditional sqrt dispatch completed\n");
    
    /* 4. OpenMP SIMD reduction */
    omp_simd_reduction(float_array, N);
    printf("  OpenMP SIMD reduction completed\n");
    
    /* 5. Mixed type operations */
    mixed_type_operations();
    printf("  Mixed type operations completed\n");
    
    /* 6. Builtin strlen in loop */
    int total_len = builtin_strlen_sum(strings, M);
    printf("  Builtin strlen sum: %d\n", total_len);
    
    /* 7. Dispatch all operation types to ensure all paths are analyzed */
    for (int op = OP_SIN_COS; op <= OP_MEMCPY; ++op) {
        dispatch_operation((operation_t)op, float_array, float_array, 64);
    }
    printf("  All operations dispatched\n");
    
    /* Aggregate results to prevent dead code elimination */
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; ++i) {
        checksum += float_array[i] + (float)double_array[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    for (int i = 0; i < M; ++i) {
        free((char*)strings[i]);
    }
    
    return 0;
}
