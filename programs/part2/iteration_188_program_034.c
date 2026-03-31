/* 
 * Comprehensive test program to trigger GCC's default_builtin_vectorized_function
 * Specifically targets the flag-setting block in targhooks.cc lines 981-990
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

/* Alignment attributes for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Simple random generator to prevent compile-time computation */
static inline float random_float(float min, float max) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

/* Function with hidden visibility - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float* ALIGN_32 out, const float* ALIGN_32 in, int n) {
    /* This function's hidden visibility may interact with the artificial declaration creation */
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; ++i) {
        /* Multiple built-in calls to trigger vectorization */
        out[i] = sinf(in[i]) * cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with always_inline - may trigger special declaration handling */
static inline __attribute__((always_inline)) 
void inline_vector_math(double* ALIGN_32 out, const double* ALIGN_32 in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* exp and log calls - common built-ins for vectorization */
        out[i] = exp(in[i]) * log(fabs(in[i]) + 1.0);
    }
}

/* Function using __builtin_memcpy in vectorizable context */
__attribute__((used))
static void builtin_memcpy_vectorization(char* ALIGN_32 dest, const char* ALIGN_32 src, int n) {
    /* Loop with __builtin_memcpy - GCC may vectorize this */
    #pragma omp simd
    for (int i = 0; i < n; i += 32) {
        int chunk = (n - i) < 32 ? (n - i) : 32;
        __builtin_memcpy(dest + i, src + i, chunk);
    }
}

/* Function with __builtin_strlen in loop - potential for vectorized strlen */
__attribute__((nothrow))
static int vectorized_strlen_sum(const char** strings, int count) {
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < count; ++i) {
        total += __builtin_strlen(strings[i]);
    }
    return total;
}

/* OpenMP declare simd function - creates SIMD variants */
#pragma omp declare simd
__attribute__((always_inline))
static inline float simd_math_function(float x) {
    return powf(x, 1.5f) + sinf(x * 0.5f);
}

/* Function using architecture-specific intrinsics with fallback */
__attribute__((used))
static void architecture_specific_vectorization(float* ALIGN_32 out, const float* ALIGN_32 in, int n) {
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path - compiler may consider vectorized built-in alternatives */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(in + i);
            __m256 result = _mm256_add_ps(vec, _mm256_set1_ps(1.0f));
            _mm256_store_ps(out + i, result);
        }
        return;
    }
#endif
    
    /* Fallback scalar path with built-in calls - will be analyzed for vectorization */
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        out[i] = sqrtf(in[i]) + sinf(in[i]) * cosf(in[i]);
    }
}

/* Multiple vectorization candidates in conditional chain */
__attribute__((noinline))
static float conditional_vectorization(int mode, float x) {
    float result = 0.0f;
    
    switch (mode) {
        case 0: {
            /* Loop with sin/cos - common vectorization target */
            float temp[8] ALIGN_32;
            #pragma omp simd
            for (int i = 0; i < 8; ++i) {
                temp[i] = sinf(x + i) * cosf(x + i);
            }
            for (int i = 0; i < 8; ++i) result += temp[i];
            break;
        }
        case 1: {
            /* Loop with exp/log */
            float temp[8] ALIGN_32;
            #pragma omp simd
            for (int i = 0; i < 8; ++i) {
                temp[i] = expf(x + i) - logf(fabsf(x + i) + 1.0f);
            }
            for (int i = 0; i < 8; ++i) result += temp[i];
            break;
        }
        case 2: {
            /* Loop with pow */
            float temp[8] ALIGN_32;
            #pragma omp simd
            for (int i = 0; i < 8; ++i) {
                temp[i] = powf(x + i, 2.0f);
            }
            for (int i = 0; i < 8; ++i) result += temp[i];
            break;
        }
    }
    
    return result;
}

/* Dead code path with vectorizable built-in calls */
__attribute__((used))
static void dead_code_vectorization(float* ALIGN_32 data, int n) {
    if (0) {  /* Dead code, but still processed by front-end */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            data[i] = sinf(data[i]) / cosf(data[i]);  /* tan via sin/cos */
        }
    }
}

/* Mixed data types and type-punning */
__attribute__((used))
static void mixed_type_vectorization(float* ALIGN_32 fdata, int* ALIGN_32 idata, int n) {
    /* Type-punning union - may invoke built-in vectorization for copies */
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Mixed built-in usage */
        converter.f = fdata[i];
        idata[i] = converter.i + __builtin_ilogb(fdata[i]);
    }
}

/* Main test driver */
int main() {
    const int N = 1024;
    const int N_STRINGS = 100;
    
    /* Aligned arrays as required */
    float array1[N] ALIGN_32;
    float array2[N] ALIGN_32;
    double darray1[N] ALIGN_64;
    double darray2[N] ALIGN_64;
    char char_buffer1[N] ALIGN_32;
    char char_buffer2[N] ALIGN_32;
    const char* strings[N_STRINGS];
    
    /* Initialize with random data */
    for (int i = 0; i < N; ++i) {
        array1[i] = random_float(-3.14f, 3.14f);
        darray1[i] = random_float(-3.14, 3.14);
        char_buffer1[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < N_STRINGS; ++i) {
        char* str = malloc(32);
        snprintf(str, 32, "String%d", i);
        strings[i] = str;
    }
    
    float total = 0.0f;
    
    /* 1. Math-intensive function with OpenMP SIMD */
    #pragma omp parallel for simd reduction(+:total)
    for (int i = 0; i < N; ++i) {
        array2[i] = sinf(array1[i]) + cosf(array1[i]);
        total += array2[i];
    }
    
    /* 2. Hidden visibility function */
    hidden_visibility_math(array2, array1, N);
    for (int i = 0; i < N; ++i) total += array2[i];
    
    /* 3. Inline vector math */
    inline_vector_math(darray2, darray1, N);
    for (int i = 0; i < N; ++i) total += darray2[i];
    
    /* 4. Builtin memcpy vectorization */
    builtin_memcpy_vectorization(char_buffer2, char_buffer1, N);
    for (int i = 0; i < N; ++i) total += char_buffer2[i];
    
    /* 5. Vectorized strlen */
    int length_sum = vectorized_strlen_sum(strings, N_STRINGS);
    total += length_sum;
    
    /* 6. Architecture-specific with fallback */
    architecture_specific_vectorization(array2, array1, N);
    for (int i = 0; i < N; ++i) total += array2[i];
    
    /* 7. Conditional vectorization - test all modes */
    for (int mode = 0; mode < 3; ++mode) {
        total += conditional_vectorization(mode, total);
    }
    
    /* 8. SIMD math function in loop */
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        array2[i] = simd_math_function(array1[i]);
        total += array2[i];
    }
    
    /* 9. Mixed type vectorization */
    int int_data[N] ALIGN_32;
    mixed_type_vectorization(array1, int_data, N);
    for (int i = 0; i < N; ++i) total += int_data[i];
    
    /* 10. Dead code path (still processed) */
    dead_code_vectorization(array1, N);
    
    /* Nested OpenMP for complex vectorization context */
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < N; ++i) {
        #pragma omp simd reduction(+:total)
        for (int j = 0; j < 16; ++j) {
            total += sqrtf(array1[i] + j) * expf(array1[i] - j);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < N_STRINGS; ++i) {
        free((char*)strings[i]);
    }
    
    printf("Total: %f\n", total);
    return 0;
}
