/* 
 * Comprehensive test program to trigger GCC's default_builtin_vectorized_function
 * and specifically cover the flag-setting block in targhooks.cc (lines 981-990)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Architecture-specific intrinsics */
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

/* Function with hidden visibility attribute - aligns with DECL_VISIBILITY setting */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float *restrict out, const float *restrict in, int n) {
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; ++i) {
        /* Multiple built-in math functions in vectorizable loop */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with used attribute - may interact with DECL_ARTIFICIAL */
static __attribute__((used)) void static_used_function(double *restrict arr, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Built-in math operations */
        arr[i] = exp(arr[i]) * log(fabs(arr[i]) + 1.0);
    }
}

/* Function with OpenMP declare simd - creates SIMD variants */
#pragma omp declare simd uniform(n) linear(i:1) simdlen(4)
__attribute__((always_inline)) 
inline float simd_math_function(float x, int n) {
    float result = x;
    for (int i = 0; i < n; ++i) {
        /* Chain of built-in calls */
        result = sinf(result) + cosf(result);
    }
    return result;
}

/* Memory operation function using __builtin_memcpy */
__attribute__((noinline)) 
void builtin_memcpy_operations(char *restrict dst, const char *restrict src, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i += 64) {
        /* Vectorized memory copy using builtin */
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

/* String operation with __builtin_strlen */
__attribute__((used)) 
int builtin_strlen_operations(const char **strings, int count) {
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < count; ++i) {
        /* Vectorizable strlen calls */
        total += __builtin_strlen(strings[i]);
    }
    return total;
}

/* Architecture-specific path with fallback */
__attribute__((target_clones("avx2", "default")))
void architecture_specific_math(float *restrict out, const float *restrict in, int n) {
    /* Conditional check for CPU features */
    if (__builtin_cpu_supports("avx2")) {
        #ifdef __x86_64__
        /* AVX2 intrinsic path */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(in + i);
            __m256 sin_vec = _mm256_sin_ps(vec);  /* Compiler may use builtin */
            __m256 cos_vec = _mm256_cos_ps(vec);  /* Compiler may use builtin */
            __m256 result = _mm256_add_ps(sin_vec, cos_vec);
            _mm256_store_ps(out + i, result);
        }
        #endif
    } else {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            out[i] = sinf(in[i]) + cosf(in[i]);
        }
    }
}

/* Function with mixed data types and type punning */
union FloatVector {
    float f[8];
    #ifdef __x86_64__
    __m256 v;
    #endif
};

void mixed_type_operations(float *restrict out, int *restrict int_out, 
                          const float *restrict in, int n) {
    /* Type punning through union */
    union FloatVector vec ALIGN_32;
    
    #pragma omp parallel for simd
    for (int i = 0; i < n; ++i) {
        /* Mixed built-in operations */
        out[i] = powf(fabsf(in[i]), 2.5f);
        int_out[i] = __builtin_ilogb(in[i]);
    }
}

/* Dead code path that still gets analyzed */
__attribute__((noinline))
void dead_code_path(float *arr, int n) {
    if (0) {  /* Dead code, but still parsed */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            /* Built-in calls in dead code */
            arr[i] = tanf(arr[i]) * asinf(arr[i] * 0.5f);
        }
    }
}

/* Switch statement with multiple vectorization candidates */
__attribute__((flatten))
void switch_builtins(float *restrict out, const float *restrict in, 
                     int n, int mode) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = sinf(in[i]) * cosf(in[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = sqrtf(fabsf(in[i])) + expf(in[i] * 0.1f);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = logf(fabsf(in[i]) + 1.0f) * powf(in[i], 1.5f);
            }
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = in[i];
            }
    }
}

/* Main test driver */
int main() {
    const int N = 1024;
    const int ITERATIONS = 100;
    
    /* Aligned arrays as recommended */
    float array1 ALIGN_32[N];
    float array2 ALIGN_32[N];
    float array3 ALIGN_32[N];
    int int_array ALIGN_32[N];
    char src_data ALIGN_64[N * 4];
    char dst_data ALIGN_64[N * 4];
    const char *strings[] = {"test1", "test12", "test123", "test1234"};
    
    /* Initialize with random data */
    for (int i = 0; i < N; ++i) {
        array1[i] = random_float(-3.14f, 3.14f);
        array2[i] = random_float(0.1f, 10.0f);
        src_data[i] = (char)(i % 256);
    }
    
    double total = 0.0;
    
    /* Test 1: Math-intensive function with OpenMP SIMD */
    #pragma omp parallel for reduction(+:total)
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        hidden_visibility_math(array3, array1, N);
        for (int i = 0; i < N; ++i) {
            total += array3[i];
        }
    }
    
    /* Test 2: Static used function */
    double double_array ALIGN_32[N];
    for (int i = 0; i < N; ++i) {
        double_array[i] = random_float(0.5f, 2.0f);
    }
    static_used_function(double_array, N);
    
    /* Test 3: SIMD math function */
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < N; ++i) {
        total += simd_math_function(array1[i], 3);
    }
    
    /* Test 4: Built-in memcpy operations */
    builtin_memcpy_operations(dst_data, src_data, N);
    
    /* Test 5: Built-in strlen operations */
    int str_total = builtin_strlen_operations(strings, 4);
    total += str_total;
    
    /* Test 6: Architecture-specific path */
    architecture_specific_math(array3, array1, N);
    for (int i = 0; i < N; ++i) {
        total += array3[i];
    }
    
    /* Test 7: Mixed type operations */
    mixed_type_operations(array3, int_array, array2, N);
    for (int i = 0; i < N; ++i) {
        total += array3[i] + int_array[i];
    }
    
    /* Test 8: Dead code path (still analyzed) */
    dead_code_path(array1, N);
    
    /* Test 9: Switch with multiple built-ins */
    for (int mode = 0; mode < 4; ++mode) {
        switch_builtins(array3, array1, N, mode);
        for (int i = 0; i < N; ++i) {
            total += array3[i];
        }
    }
    
    /* Prevent dead code elimination */
    printf("Total: %f\n", total);
    printf("String total: %d\n", str_total);
    
    return 0;
}
