/* 
 * This program is designed to trigger GCC's vectorization of built-in functions,
 * specifically targeting the flag-setting block in default_builtin_vectorized_function
 * in targhooks.cc (lines 981-990).
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
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* OpenMP for SIMD directives */
#include <omp.h>

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Size known at compile-time for vectorization analysis */
#define SIZE 1024
#define ITERATIONS 100

/* ========== Function Declarations with Various Attributes ========== */

/* Hidden visibility function - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden")))
static void hidden_visibility_math(float *out, const float *in, int n);

/* Used attribute - may interact with TREE_USED(t) = 1 */
__attribute__((used))
static void used_function_with_builtins(double *arr, int n);

/* Nothrow attribute - aligns with TREE_NOTHROW(t) = 1 */
__attribute__((nothrow))
static void nothrow_vector_memcpy(char *dest, const char *src, size_t n);

/* Always inline to ensure analysis */
__attribute__((always_inline))
static inline void inline_math_function(float *result, const float *input, int count);

/* ========== Vectorization Helper Functions ========== */

/* Function with #pragma omp declare simd to create SIMD variants */
#pragma omp declare simd
static float simd_math_wrapper(float x) {
    return sinf(x) * cosf(x) + sqrtf(fabsf(x));
}

/* Static function with vectorizable built-in calls */
static void vectorized_math_loop(float *output, const float *input, int n) {
    /* Ignore vector dependencies */
    #pragma GCC ivdep
    /* Explicit SIMD vectorization */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls to increase vectorization candidates */
        output[i] = sinf(input[i]) + cosf(input[i]) + sqrtf(input[i]);
    }
}

/* Function with mixed data types and alignment */
static void mixed_type_vectorization(float *farr, double *darr, int *iarr, int n) {
    /* Aligned arrays for better vectorization */
    float temp_f ALIGN_32[SIZE];
    double temp_d ALIGN_32[SIZE];
    
    /* First loop: float operations with built-ins */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        temp_f[i] = sinf(farr[i]) * expf(farr[i]);
    }
    
    /* Second loop: double operations with built-ins */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        temp_d[i] = log(darr[i] + 1.0) * pow(darr[i], 0.5);
    }
    
    /* Third loop: integer built-in */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        iarr[i] = __builtin_popcount(iarr[i]);
    }
    
    /* Copy results back */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        farr[i] = temp_f[i];
        darr[i] = temp_d[i];
    }
}

/* ========== Architecture-Specific Paths ========== */

/* Conditional compilation based on CPU support */
static void architecture_specific_vectorization(float *data, int n) {
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path - compiler may vectorize built-ins for this target */
        #pragma omp simd
        for (int i = 0; i < n; i += 8) {
            /* Using __builtin_ prefixed functions */
            __m256 vec = _mm256_load_ps(&data[i]);
            /* This may trigger vectorized built-in generation */
            for (int j = 0; j < 8 && (i + j) < n; j++) {
                data[i + j] = __builtin_sqrtf(data[i + j]) + 
                             __builtin_sinf(data[i + j]);
            }
        }
    } else 
    #endif
    {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            data[i] = sqrtf(data[i]) + sinf(data[i]) + logf(fabsf(data[i]) + 1.0f);
        }
    }
}

/* ========== Memory Built-in Vectorization ========== */

/* Function to trigger vectorization of memory built-ins */
static void vectorized_memory_operations(char *buf1, char *buf2, int n) {
    /* Loop with __builtin_memcpy - may be vectorized */
    for (int i = 0; i < n; i += 64) {
        int chunk = (n - i) < 64 ? (n - i) : 64;
        __builtin_memcpy(&buf2[i], &buf1[i], chunk);
    }
    
    /* strlen in a vectorizable context */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < n; i += 128) {
        char temp[128];
        __builtin_memcpy(temp, &buf1[i], 128);
        temp[127] = '\0';
        total_len += __builtin_strlen(temp);
    }
}

/* ========== Complex Control Flow ========== */

/* Multiple small functions with different built-ins */
__attribute__((always_inline)) 
static inline void pow_loop(double *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = pow(arr[i], 1.5);
    }
}

__attribute__((always_inline))
static inline void exp_loop(double *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = exp(arr[i]);
    }
}

__attribute__((always_inline))
static inline void log_loop(double *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = log(arr[i] + 1.0);
    }
}

/* Function with switch to ensure analysis of all paths */
static void switch_builtin_vectorization(int mode, double *arr, int n) {
    switch (mode) {
        case 0:
            pow_loop(arr, n);
            break;
        case 1:
            exp_loop(arr, n);
            break;
        case 2:
            log_loop(arr, n);
            break;
        default:
            /* Dead code path that still gets analyzed */
            if (0) {  /* Always false, but compiler analyzes */
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    arr[i] = __builtin_sqrt(arr[i]) + __builtin_sin(arr[i]);
                }
            }
            break;
    }
}

/* ========== OpenMP Parallel SIMD ========== */

static void openmp_parallel_simd(float *data, int n) {
    double sum = 0.0;
    
    /* Nested pragmas for complex vectorization context */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; j++) {
            float val = data[i] + j * 0.1f;
            sum += sinf(val) * cosf(val);
        }
    }
    
    /* Prevent dead code elimination */
    data[0] = (float)sum;
}

/* ========== Hidden Visibility Implementation ========== */

__attribute__((visibility("hidden")))
static void hidden_visibility_math(float *out, const float *in, int n) {
    /* This function's hidden visibility may interact with the flag-setting */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]) * logf(fabsf(in[i]) + 1.0f);
    }
}

/* ========== Main Test Function ========== */

int main(void) {
    /* Initialize with random data to prevent compile-time computation */
    srand(time(NULL));
    
    /* Aligned arrays as recommended */
    float float_array ALIGN_32[SIZE];
    double double_array ALIGN_32[SIZE];
    int int_array ALIGN_32[SIZE];
    char buffer1 ALIGN_64[SIZE * 2];
    char buffer2 ALIGN_64[SIZE * 2];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        float_array[i] = (float)rand() / RAND_MAX * 10.0f;
        double_array[i] = (double)rand() / RAND_MAX * 10.0;
        int_array[i] = rand();
        buffer1[i] = 'A' + (rand() % 26);
    }
    buffer1[SIZE-1] = '\0';
    
    printf("Starting vectorization tests...\n");
    
    /* Run multiple test functions to trigger various vectorization paths */
    
    /* 1. Math-intensive function with OpenMP SIMD */
    vectorized_math_loop(float_array, float_array, SIZE);
    
    /* 2. Mixed data types */
    mixed_type_vectorization(float_array, double_array, int_array, SIZE);
    
    /* 3. Architecture-specific paths */
    architecture_specific_vectorization(float_array, SIZE);
    
    /* 4. Memory operations with built-ins */
    vectorized_memory_operations(buffer1, buffer2, SIZE);
    
    /* 5. Switch with multiple built-in paths */
    for (int mode = 0; mode < 3; mode++) {
        switch_builtin_vectorization(mode, double_array, SIZE);
    }
    
    /* 6. Hidden visibility function */
    hidden_visibility_math(float_array, float_array, SIZE);
    
    /* 7. OpenMP parallel SIMD */
    openmp_parallel_simd(float_array, SIZE);
    
    /* 8. SIMD wrapper function */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        float_array[i] = simd_math_wrapper(float_array[i]);
    }
    
    /* Aggregate results to prevent dead code elimination */
    float sum = 0.0f;
    double dsum = 0.0;
    #pragma omp simd reduction(+:sum, dsum)
    for (int i = 0; i < SIZE; i++) {
        sum += float_array[i];
        dsum += double_array[i];
    }
    
    printf("Final sums: float=%f, double=%f\n", sum, dsum);
    printf("Buffer2 starts with: %c%c%c\n", buffer2[0], buffer2[1], buffer2[2]);
    
    return 0;
}
