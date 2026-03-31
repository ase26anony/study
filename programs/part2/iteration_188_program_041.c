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

/* Alignment attributes for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Visibility and linkage attributes to interact with DECL flags */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))

/* Prevent compiler from optimizing away computations */
static volatile int sink = 0;

/* ==================== HELPER FUNCTIONS WITH ATTRIBUTES ==================== */

/* Function with hidden visibility - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
HIDDEN_VIS static void process_hidden(float* data, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls to trigger vectorization */
        data[i] = sinf(data[i]) * cosf(data[i]) + sqrtf(fabsf(data[i]));
    }
}

/* Function with used attribute - aligns with TREE_USED(t) = 1 */
USED_FUNC static void process_used(double* data, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        data[i] = exp(data[i]) * log(fabs(data[i]) + 1.0);
    }
}

/* Function with nothrow attribute - aligns with TREE_NOTHROW(t) = 1 */
NOTHROW_FUNC static void process_nothrow(float* data, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        data[i] = powf(data[i], 2.5f) + expf(data[i] * 0.5f);
    }
}

/* ==================== VECTORIZATION WITH BUILTINS ==================== */

/* Test 1: Math-intensive loop with OpenMP SIMD */
static void test_math_builtins(float* arr, int n) {
    #pragma omp simd reduction(+:sink)
    for (int i = 0; i < n; i++) {
        /* Multiple built-in function calls */
        float val = arr[i];
        val = sinf(val) + cosf(val);
        val = sqrtf(fabsf(val));
        val = logf(val + 1.0f);
        arr[i] = val;
        sink += (int)val;
    }
}

/* Test 2: Memory operations with __builtin_memcpy in vectorizable context */
static void test_mem_builtins(char* dst, char* src, int n) {
    /* Use __builtin_memcpy in a way that could be vectorized */
    for (int i = 0; i < n; i += 64) {
        __builtin_memcpy(dst + i, src + i, 64);
    }
    
    /* Also test __builtin_strlen in a loop */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < n; i += 64) {
        total_len += __builtin_strlen(dst + i);
    }
    sink += total_len;
}

/* Test 3: Conditional architecture-specific paths */
static void test_conditional_vectorization(float* data, int n) {
    /* This conditional ensures both paths are analyzed */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - compiler may still consider builtin vectorization */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(&data[i]);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&data[i], result);
        }
    } else 
    #endif
    {
        /* Fallback scalar path with built-in calls - should trigger vectorization */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            data[i] = sqrtf(data[i]);
        }
    }
}

/* Test 4: Mixed data types and type punning */
static void test_mixed_types(double* dbl_arr, float* flt_arr, int* int_arr, int n) {
    /* Type punning through union */
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Mixed built-in calls */
        dbl_arr[i] = sin(dbl_arr[i]) * cos(dbl_arr[i]);
        flt_arr[i] = sqrtf(flt_arr[i] + 1.0f);
        int_arr[i] = __builtin_ilogb(dbl_arr[i]);
        
        /* Type punning that might use builtin memcpy */
        converter.f = flt_arr[i];
        int_arr[i] ^= converter.i;
    }
}

/* Test 5: OpenMP declare simd function */
#pragma omp declare simd uniform(a, b) linear(i)
static float simd_poly(float x, float a, float b, int i) {
    return a * powf(x, b) + sinf(x * i);
}

static void test_omp_declare_simd(float* data, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        data[i] = simd_poly(data[i], 1.5f, 2.0f, i);
    }
}

/* ==================== COMPLEX CONTROL FLOW ==================== */

/* Multiple small functions with always_inline */
static inline __attribute__((always_inline)) 
void process_pow(float* val, int i) {
    *val = powf(*val, 1.5f);
}

static inline __attribute__((always_inline))
void process_exp(float* val, int i) {
    *val = expf(*val * 0.1f * i);
}

static inline __attribute__((always_inline))
void process_fabs(float* val, int i) {
    *val = fabsf(*val) + i;
}

/* Function with switch to ensure multiple paths analyzed */
static void test_switch_vectorization(float* data, int n, int mode) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                process_pow(&data[i], i);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                process_exp(&data[i], i);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                process_fabs(&data[i], i);
            }
            break;
        default:
            /* Dead code path that still gets analyzed */
            if (0) {
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    data[i] = sinf(data[i]) * cosf(data[i]);
                }
            }
            break;
    }
}

/* ==================== MAIN EXECUTION FLOW ==================== */

int main() {
    const int N = 1024;
    const int ITERS = 10;
    
    /* Aligned arrays as recommended */
    float    arr1[N] ALIGN_32;
    double   arr2[N] ALIGN_64;
    float    arr3[N] ALIGN_32;
    char     mem1[N * 4] ALIGN_32;
    char     mem2[N * 4] ALIGN_32;
    int      int_arr[N] ALIGN_32;
    
    /* Initialize with pattern data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)(rand() % 1000) / 100.0f;
        arr2[i] = (double)(rand() % 1000) / 100.0;
        arr3[i] = (float)(rand() % 1000) / 100.0f;
        int_arr[i] = rand() % 1000;
    }
    
    /* Fill memory buffers */
    for (int i = 0; i < N * 4; i++) {
        mem1[i] = 'A' + (rand() % 26);
        mem2[i] = 0;
    }
    mem1[N * 4 - 1] = '\0';
    
    printf("Starting vectorization tests...\n");
    
    /* Execute all test functions in sequence */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Test 1: Math builtins with OpenMP SIMD */
        test_math_builtins(arr1, N);
        
        /* Test 2: Memory builtins */
        test_mem_builtins(mem2, mem1, N * 4);
        
        /* Test 3: Conditional vectorization */
        test_conditional_vectorization(arr3, N);
        
        /* Test 4: Mixed types */
        test_mixed_types(arr2, arr1, int_arr, N);
        
        /* Test 5: OpenMP declare simd */
        test_omp_declare_simd(arr3, N);
        
        /* Test 6: Switch-based vectorization */
        test_switch_vectorization(arr1, N, iter % 4);
        
        /* Test 7: Functions with special attributes */
        process_hidden(arr1, N);
        process_used(arr2, N);
        process_nothrow(arr3, N);
        
        /* Prevent optimization */
        sink += (int)arr1[0] + (int)arr2[0] + (int)arr3[0];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: arr1[0]=%.3f, arr2[0]=%.3f, arr3[0]=%.3f\n", 
           arr1[0], arr2[0], arr3[0]);
    printf("Sink value: %d\n", sink);
    printf("Memory test: strlen(mem2)=%zu\n", strlen(mem2));
    
    return 0;
}
