/* test_vectorized_builtins.c
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -ffast-math -fopt-info-vec -o test test_vectorized_builtins.c -lm
 * For coverage: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints to engage vectorizer */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
static inline float math_func_1(float x) __attribute__((always_inline, nothrow));
static inline float math_func_2(float x) __attribute__((always_inline, nothrow));
static void process_with_hidden_visibility(float *arr, int n) __attribute__((visibility("hidden"), used));

/* Simple random generator to prevent compile-time computation */
static float random_float(void) {
    static unsigned int seed = 123456789;
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

/* Function 1: Math-intensive with explicit SIMD pragma */
void vectorized_math_operations(float *in, float *out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls that should be vectorized */
        out[i] = sinf(in[i]) * cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Function 2: Memory operations with builtins */
void vectorized_mem_operations(char *dst, char *src, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i += 64) {
        /* Use __builtin_memcpy in vectorizable context */
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

/* Function 3: Architecture-specific paths with CPU dispatch */
void conditional_vector_paths(double *arr, double *result, int n) {
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - compiler may still analyze scalar fallback */
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(&arr[i]);
            __m256d sqrt_vec = _mm256_sqrt_pd(vec);
            _mm256_store_pd(&result[i], sqrt_vec);
        }
    } else
    #endif
    {
        /* Scalar fallback with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            result[i] = sqrt(arr[i]) + log(fabs(arr[i]) + 1.0);
        }
    }
}

/* Function 4: Hidden visibility helper with mixed operations */
static void process_with_hidden_visibility(float *arr, int n) {
    double temp ALIGN_32;
    
    #pragma omp simd reduction(+:temp)
    for (int i = 0; i < n; i++) {
        /* Mixed built-in calls */
        arr[i] = powf(fabsf(arr[i]), 0.5f) * expf(arr[i] * 0.1f);
        temp += arr[i];
    }
    
    /* Prevent dead code elimination */
    if (temp < 0) printf("unreachable\n");
}

/* Function 5: Multiple small inline functions with different builtins */
static inline float math_func_1(float x) {
    return sinf(x) * cosf(x);
}

static inline float math_func_2(float x) {
    return sqrtf(x) * expf(x * 0.5f);
}

/* Function 6: Complex control flow with vectorization candidates */
void switch_based_vectorization(float *arr, int n, int mode) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = math_func_1(arr[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = math_func_2(arr[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = logf(fabsf(arr[i]) + 1.0f);
            }
            break;
        default:
            /* Dead code path that still gets analyzed */
            if (0) {
                #pragma GCC ivdep
                for (int i = 0; i < n; i++) {
                    arr[i] = __builtin_powif(arr[i], 2);
                }
            }
            break;
    }
}

/* Function 7: OpenMP parallel with SIMD reduction */
float parallel_simd_reduction(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += sinf(arr[i]) * cosf(arr[i]);
    }
    
    return sum;
}

/* Function 8: Type-punning and alignment testing */
void type_punning_operations(void *dst, void *src, int n) {
    union {
        float f[8];
        __m256 vec;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < n; i += 8) {
        /* Type punning that may trigger builtin vectorization */
        __builtin_memcpy(converter.f, (float*)src + i, 8 * sizeof(float));
        converter.f[0] = sqrtf(converter.f[0]);
        __builtin_memcpy((float*)dst + i, converter.f, 8 * sizeof(float));
    }
}

/* Function 9: String operation in vectorizable loop */
int vectorized_strlen_operations(char **strings, int count) {
    int total_len = 0;
    
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < count; i++) {
        /* __builtin_strlen in vectorizable context */
        total_len += __builtin_strlen(strings[i]);
    }
    
    return total_len;
}

/* Main function that exercises all paths */
int main(void) {
    const int N = 1024;
    const int M = 128;
    
    /* Aligned arrays as per requirements */
    float arr1[N] ALIGN_32;
    float arr2[N] ALIGN_32;
    double darr1[N] ALIGN_64;
    double darr2[N] ALIGN_64;
    char buffer1[N * 4] ALIGN_32;
    char buffer2[N * 4] ALIGN_32;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        arr1[i] = random_float() * 10.0f;
        darr1[i] = random_float() * 10.0;
        ((float*)buffer1)[i] = random_float();
        ((float*)buffer2)[i] = 0.0f;
    }
    
    /* Create string array for strlen test */
    char *strings[M];
    for (int i = 0; i < M; i++) {
        strings[i] = "test_string";
    }
    
    printf("Starting vectorized built-in tests...\n");
    
    /* Test 1: Math operations with explicit SIMD */
    vectorized_math_operations(arr1, arr2, N);
    
    /* Test 2: Memory operations */
    vectorized_mem_operations(buffer2, buffer1, N * 4);
    
    /* Test 3: Conditional architecture paths */
    conditional_vector_paths(darr1, darr2, N);
    
    /* Test 4: Hidden visibility function */
    process_with_hidden_visibility(arr1, N);
    
    /* Test 5: Switch-based vectorization (test all modes) */
    for (int mode = 0; mode < 3; mode++) {
        switch_based_vectorization(arr2, N, mode);
    }
    
    /* Test 6: OpenMP parallel SIMD reduction */
    float sum = parallel_simd_reduction(arr1, N);
    
    /* Test 7: Type punning operations */
    type_punning_operations(buffer1, buffer2, N);
    
    /* Test 8: Vectorized strlen */
    int total_len = vectorized_strlen_operations(strings, M);
    
    /* Aggregate results to prevent elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += arr1[i] + arr2[i] + (float)darr2[i];
    }
    
    printf("Results: sum=%.6f, total_len=%d, checksum=%.6f\n", 
           sum, total_len, checksum);
    
    return 0;
}
