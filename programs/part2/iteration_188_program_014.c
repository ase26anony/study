/* test_vectorization.c - Comprehensive test to trigger default_builtin_vectorized_function */
/* Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fopt-info-vec -ffast-math test_vectorization.c -lm -o test_vec */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#include <cpuid.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

#define SIZE 1024
#define ALIGN 32

/* Helper function to prevent compile-time computation */
static float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

/* Function with hidden visibility attribute - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden")))
static void hidden_visibility_math(float *restrict out, const float *restrict in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls that can be vectorized */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Function with nothrow and used attributes */
__attribute__((used, nothrow))
static void vectorized_mem_ops(char *restrict dst, const char *restrict src, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i += 64) {
        /* Using __builtin_memcpy in vectorizable context */
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

/* Always inline function with pow and exp calls */
__attribute__((always_inline))
static inline void power_operations(double *restrict out, const double *restrict in, int n) {
    #pragma omp simd reduction(+:out[:n])
    for (int i = 0; i < n; i++) {
        out[i] = pow(in[i], 2.5) + exp(in[i] * 0.5);
    }
}

/* Function using architecture-specific intrinsics */
static void avx_vector_path(float *restrict out, const float *restrict in, int n) {
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX intrinsic path - may trigger built-in vectorization */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(in + i);
            __m256 result = _mm256_add_ps(_mm256_sin_ps(vec), _mm256_cos_ps(vec));
            _mm256_store_ps(out + i, result);
        }
    } else
#endif
    {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sinf(in[i]) + cosf(in[i]);
        }
    }
}

/* Function with mixed data types and alignment hints */
static void mixed_type_operations(float *restrict fout, double *restrict dout, 
                                  const int *restrict iin, int n) {
    /* Aligned arrays as per requirement */
    float __attribute__((aligned(32))) temp[SIZE];
    double __attribute__((aligned(32))) dtemp[SIZE];
    
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        /* Type conversions and built-in calls */
        temp[i] = (float)__builtin_sqrt((double)iin[i]);
        dtemp[i] = log((double)iin[i] + 1.0);
    }
    
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        fout[i] = temp[i];
        dout[i] = dtemp[i];
    }
}

/* OpenMP declare simd function - creates SIMD variants */
#pragma omp declare simd uniform(b) linear(i:1)
float simd_multiply_add(float a, float b, int i) {
    return a * b + sinf((float)i * 0.1f);
}

/* Complex control flow with multiple vectorization candidates */
static void conditional_vector_path(int mode, float *restrict out, const float *restrict in, int n) {
    switch (mode) {
        case 0: {
            /* Path 0: sin/cos operations */
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sinf(in[i]) * cosf(in[i]);
            }
            break;
        }
        case 1: {
            /* Path 1: exp/log operations */
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = expf(in[i]) - logf(fabsf(in[i]) + 1.0f);
            }
            break;
        }
        case 2: {
            /* Path 2: pow operations */
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = powf(in[i], 1.5f);
            }
            break;
        }
        default: {
            /* Default path: multiple operations */
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sinf(in[i]) + cosf(in[i]) + expf(in[i] * 0.5f);
            }
            break;
        }
    }
    
    /* Dead code path that still gets analyzed */
    if (0) {  /* Always false, but frontend processes declarations */
        float __attribute__((aligned(32))) dummy[SIZE];
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            dummy[i] = tanf(in[i]) * atanf(in[i]);
        }
    }
}

/* Function using strlen in vectorizable context */
static void string_operations(const char **strings, int *lengths, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* __builtin_strlen may get vectorized */
        lengths[i] = __builtin_strlen(strings[i]);
    }
}

/* Main test function */
int main() {
    /* Aligned arrays as per requirement */
    float __attribute__((aligned(ALIGN))) fdata[SIZE];
    float __attribute__((aligned(ALIGN))) fresult[SIZE];
    double __attribute__((aligned(ALIGN))) ddata[SIZE];
    double __attribute__((aligned(ALIGN))) dresult[SIZE];
    int __attribute__((aligned(ALIGN))) idata[SIZE];
    char __attribute__((aligned(ALIGN))) src_data[SIZE * 2];
    char __attribute__((aligned(ALIGN))) dst_data[SIZE * 2];
    
    /* Initialize with pattern data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        float r = simple_rand(i);
        fdata[i] = r * 4.0f - 2.0f;  /* Range: -2 to 2 */
        ddata[i] = (double)r * 4.0 - 2.0;
        idata[i] = (int)(r * 1000.0f);
        src_data[i] = (char)(r * 255);
    }
    
    /* Fill string data */
    const char *test_strings[] = {"hello", "world", "test", "vector", "builtin"};
    int str_lengths[5];
    
    printf("Starting vectorization tests...\n");
    
    /* Test 1: Math-intensive function with OpenMP SIMD */
    hidden_visibility_math(fresult, fdata, SIZE);
    
    /* Test 2: Memory operations */
    vectorized_mem_ops(dst_data, src_data, SIZE);
    
    /* Test 3: Power operations with always_inline */
    power_operations(dresult, ddata, SIZE);
    
    /* Test 4: Architecture-specific path */
    avx_vector_path(fresult, fdata, SIZE);
    
    /* Test 5: Mixed type operations */
    mixed_type_operations(fresult, dresult, idata, SIZE);
    
    /* Test 6: Conditional vector paths */
    for (int mode = 0; mode < 4; mode++) {
        conditional_vector_path(mode, fresult, fdata, SIZE);
    }
    
    /* Test 7: String operations */
    string_operations(test_strings, str_lengths, 5);
    
    /* Test 8: OpenMP declare simd function in loop */
    float sum = 0.0f;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 8; j++) {
            sum += simd_multiply_add(fdata[i], 2.0f, j);
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    float total = 0.0f;
    double dtotal = 0.0;
    #pragma omp simd reduction(+:total, dtotal)
    for (int i = 0; i < SIZE; i++) {
        total += fresult[i];
        dtotal += dresult[i];
    }
    
    printf("Results: float_sum = %f, double_sum = %f, simd_sum = %f\n", 
           total, dtotal, sum);
    printf("String lengths: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", str_lengths[i]);
    }
    printf("\n");
    
    /* Type punning via union (as suggested in requirements) */
    union {
        __m128 vec;
        float arr[4];
    } pun;
    
    pun.vec = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    printf("Type punning test: %f %f %f %f\n", pun.arr[0], pun.arr[1], pun.arr[2], pun.arr[3]);
    
    return 0;
}
