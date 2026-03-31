/* 
 * This program is designed to trigger GCC's vectorizer to request
 * vectorized versions of built-in functions, specifically targeting
 * the flag-setting block in default_builtin_vectorized_function.
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

/* OpenMP for SIMD directives */
#include <omp.h>

/* Alignment hints for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define NO_THROW __attribute__((nothrow))
#define FORCE_USED __attribute__((used))
#define ALWAYS_INLINE __attribute__((always_inline))

/* ==================== Helper Functions with Attributes ==================== */

/* Hidden visibility function - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
static HIDDEN_VIS NO_THROW FORCE_USED
void hidden_visibility_math(float* dest, const float* src, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Multiple built-in math functions in vectorizable loop */
        dest[i] = sinf(src[i]) + cosf(src[i]) + sqrtf(fabsf(src[i]));
    }
}

/* Always inline function with exp/log operations */
static ALWAYS_INLINE NO_THROW
void inline_exp_log(double* dest, const double* src, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        dest[i] = exp(src[i]) * log(fabs(src[i]) + 1.0);
    }
}

/* Function with __builtin_memcpy in loop */
static NO_THROW
void builtin_memcpy_loop(char* dest, const char* src, int n, int chunk) {
    for (int i = 0; i < n; i += chunk) {
        /* Vectorizable built-in memory operation */
        __builtin_memcpy(dest + i, src + i, chunk);
    }
}

/* Function using strlen in vectorizable context */
static NO_THROW
int builtin_strlen_loop(const char* strings[], int count) {
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < count; ++i) {
        total += __builtin_strlen(strings[i]);
    }
    return total;
}

/* OpenMP declare simd function - creates SIMD variants */
#pragma omp declare simd
float simd_math_function(float x) {
    return sinf(x) * cosf(x) + sqrtf(x + 1.0f);
}

/* ==================== Architecture-Specific Paths ==================== */

#ifdef __x86_64__
/* AVX2 path using intrinsics */
static NO_THROW
void avx2_math_path(float* dest, const float* src, int n) {
    for (int i = 0; i < n; i += 8) {
        __m256 vec = _mm256_load_ps(src + i);
        /* Simulate math operations - in real code would use actual math intrinsics */
        __m256 result = _mm256_add_ps(vec, _mm256_set1_ps(1.0f));
        _mm256_store_ps(dest + i, result);
    }
}

/* CPU feature detection */
static int supports_avx2(void) {
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 5)) != 0; /* AVX2 bit */
}
#endif

/* Conditional function with both vector and scalar paths */
static NO_THROW
void conditional_math_path(float* dest, const float* src, int n) {
    #ifdef __x86_64__
    if (supports_avx2()) {
        /* Vector intrinsic path */
        avx2_math_path(dest, src, n);
        return;
    }
    #endif
    
    /* Fallback scalar path with built-in math */
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        dest[i] = sqrtf(src[i]) + sinf(src[i]) * cosf(src[i]);
    }
}

/* ==================== Complex Control Flow ==================== */

/* Multiple small functions with different built-ins */
static ALWAYS_INLINE NO_THROW
float func_pow(float x) { return powf(x, 2.0f); }

static ALWAYS_INLINE NO_THROW
float func_exp(float x) { return expf(x); }

static ALWAYS_INLINE NO_THROW
float func_fabs(float x) { return fabsf(x); }

/* Switch between different vectorization candidates */
static NO_THROW
void switch_builtins(float* dest, const float* src, int n, int mode) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                dest[i] = func_pow(src[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                dest[i] = func_exp(src[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                dest[i] = func_fabs(src[i]);
            }
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                dest[i] = src[i];
            }
    }
}

/* Dead code path that still gets analyzed */
static NO_THROW
void dead_code_path(float* dest, const float* src, int n) {
    if (0) { /* Dead condition */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            /* These built-in calls should still be analyzed */
            dest[i] = sinf(src[i]) * cosf(src[i]) / tanf(src[i] + 1.0f);
        }
    }
}

/* ==================== Mixed Data Types ==================== */

/* Union for type punning */
typedef union {
    __m128 vec;
    float arr[4];
} vec_union;

/* Function with mixed types and alignment */
static NO_THROW
void mixed_type_operations(float* farr, double* darr, int* iarr, int n) {
    /* Aligned arrays */
    float ALIGN_32 temp_f[256];
    double ALIGN_64 temp_d[256];
    
    /* Type conversions and built-ins */
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Mixed type operations */
        darr[i] = (double)farr[i] * 2.0;
        farr[i] = sqrtf(fabsf(farr[i]));
        iarr[i] = __builtin_ilogb(darr[i]);
    }
    
    /* Type punning through union */
    vec_union u;
    u.vec = _mm_setzero_ps();
    __builtin_memcpy(temp_f, u.arr, sizeof(float) * 4);
}

/* ==================== OpenMP Parallel SIMD ==================== */

static NO_THROW
double parallel_simd_reduction(const float* arr, int n) {
    double sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        /* Built-in math in parallel SIMD loop */
        sum += sinf(arr[i]) + cosf(arr[i]);
    }
    
    return sum;
}

/* ==================== Main Test Function ==================== */

int main(void) {
    const int N = 1024;
    const int N_DOUBLE = 512;
    const int CHUNK = 64;
    
    /* Aligned arrays with initialization */
    float ALIGN_32 src_floats[N];
    float ALIGN_32 dest_floats[N];
    double ALIGN_64 src_doubles[N_DOUBLE];
    double ALIGN_64 dest_doubles[N_DOUBLE];
    int ALIGN_32 ints[N];
    char ALIGN_32 src_chars[N];
    char ALIGN_32 dest_chars[N];
    
    /* String array for strlen test */
    const char* strings[] = {
        "test1", "test12", "test123", "test1234",
        "test1", "test12", "test123", "test1234"
    };
    const int string_count = sizeof(strings) / sizeof(strings[0]);
    
    /* Initialize with pattern data (prevents compile-time computation) */
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        src_floats[i] = (float)rand() / RAND_MAX * 10.0f;
        src_chars[i] = 'A' + (rand() % 26);
        ints[i] = rand() % 100;
    }
    
    for (int i = 0; i < N_DOUBLE; ++i) {
        src_doubles[i] = (double)rand() / RAND_MAX * 10.0;
    }
    
    double total_sum = 0.0;
    
    /* 1. Math-intensive function with OpenMP SIMD */
    hidden_visibility_math(dest_floats, src_floats, N);
    for (int i = 0; i < 10; ++i) {
        total_sum += dest_floats[i];
    }
    
    /* 2. Memory/copy function with __builtin_memcpy */
    builtin_memcpy_loop(dest_chars, src_chars, N, CHUNK);
    total_sum += dest_chars[0] + dest_chars[N-1];
    
    /* 3. Conditional function with CPU feature detection */
    conditional_math_path(dest_floats, src_floats, N);
    for (int i = 0; i < 10; ++i) {
        total_sum += dest_floats[i];
    }
    
    /* 4. Hidden visibility helper with double operations */
    inline_exp_log(dest_doubles, src_doubles, N_DOUBLE);
    for (int i = 0; i < 10; ++i) {
        total_sum += dest_doubles[i];
    }
    
    /* 5. Switch between different built-in functions */
    for (int mode = 0; mode < 3; ++mode) {
        switch_builtins(dest_floats, src_floats, N, mode);
        total_sum += dest_floats[0];
    }
    
    /* 6. Dead code path (should still be analyzed) */
    dead_code_path(dest_floats, src_floats, N);
    
    /* 7. Mixed data types with alignment */
    mixed_type_operations(src_floats, src_doubles, ints, 100);
    total_sum += src_floats[0] + src_doubles[0] + ints[0];
    
    /* 8. OpenMP parallel SIMD reduction */
    total_sum += parallel_simd_reduction(src_floats, N);
    
    /* 9. Built-in strlen in vectorizable context */
    int str_len_total = builtin_strlen_loop(strings, string_count);
    total_sum += str_len_total;
    
    /* 10. SIMD function via OpenMP declare simd */
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        dest_floats[i] = simd_math_function(src_floats[i]);
    }
    total_sum += dest_floats[0];
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    printf("String length total: %d\n", str_len_total);
    
    return 0;
}
