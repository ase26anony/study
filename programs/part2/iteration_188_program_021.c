/* test_vectorized_builtins.c */
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

/* Alignment attributes for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
static inline float math_func_1(float x) __attribute__((always_inline));
static inline float math_func_2(float x) __attribute__((always_inline));
static void hidden_visibility_func(double *arr, int n) __attribute__((visibility("hidden"), used, nothrow));

/* Global aligned arrays */
static float arr_f1[1024] ALIGN_32;
static float arr_f2[1024] ALIGN_32;
static double arr_d1[1024] ALIGN_64;
static double arr_d2[1024] ALIGN_64;
static int arr_i1[1024] ALIGN_32;
static char str_buf[4096] ALIGN_32;

/* Simple random initialization to prevent compile-time computation */
static void init_data(void) {
    srand(time(NULL));
    for (int i = 0; i < 1024; i++) {
        arr_f1[i] = (float)rand() / RAND_MAX * 10.0f;
        arr_f2[i] = (float)rand() / RAND_MAX * 10.0f;
        arr_d1[i] = (double)rand() / RAND_MAX * 10.0;
        arr_d2[i] = (double)rand() / RAND_MAX * 10.0;
        arr_i1[i] = rand() % 100;
    }
    for (int i = 0; i < 4095; i++) {
        str_buf[i] = 'A' + (rand() % 26);
    }
    str_buf[4095] = '\0';
}

/* Function 1: Math-intensive with OpenMP SIMD pragma */
#pragma omp declare simd
static float math_func_1(float x) {
    return sinf(x) * cosf(x);
}

#pragma omp declare simd
static float math_func_2(float x) {
    return sqrtf(fabsf(x)) + logf(x + 1.0f);
}

static float test_math_vectorization(void) {
    float sum = 0.0f;
    
    /* Loop with multiple built-in calls and explicit SIMD pragma */
    #pragma omp simd reduction(+:sum) aligned(arr_f1, arr_f2:32)
    for (int i = 0; i < 1024; i++) {
        float val1 = math_func_1(arr_f1[i]);
        float val2 = math_func_2(arr_f2[i]);
        sum += val1 + val2;
    }
    
    /* Another loop with different math built-ins */
    #pragma GCC ivdep
    for (int i = 0; i < 1024; i++) {
        arr_f1[i] = powf(arr_f1[i], 1.5f) + expf(arr_f2[i]);
    }
    
    return sum;
}

/* Function 2: Memory operations with builtins */
static size_t test_mem_builtins(void) {
    size_t total_len = 0;
    char local_buf[4096] ALIGN_32;
    
    /* Use __builtin_memcpy in a vectorizable context */
    for (int i = 0; i < 1024; i += 128) {
        __builtin_memcpy(&local_buf[i], &str_buf[i], 128);
    }
    
    /* strlen in a loop - may trigger vectorized strlen */
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < 1024; i++) {
        total_len += __builtin_strlen(&str_buf[i * 4]);
    }
    
    return total_len;
}

/* Function 3: Architecture-specific intrinsics with fallback */
static double test_arch_specific(void) {
    double result = 0.0;
    
    /* Conditional path with CPU feature detection */
    if (__builtin_cpu_supports("avx2")) {
        /* x86_64 AVX2 path */
        #ifdef __x86_64__
        __m256d sum_vec = _mm256_setzero_pd();
        for (int i = 0; i < 1024; i += 4) {
            __m256d vec = _mm256_load_pd(&arr_d1[i]);
            __m256d sqrt_vec = _mm256_sqrt_pd(vec);
            sum_vec = _mm256_add_pd(sum_vec, sqrt_vec);
        }
        double temp[4];
        _mm256_store_pd(temp, sum_vec);
        result = temp[0] + temp[1] + temp[2] + temp[3];
        #endif
    } else {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd reduction(+:result)
        for (int i = 0; i < 1024; i++) {
            result += __builtin_sqrt(arr_d1[i]);
        }
    }
    
    /* Dead code path that still contains vectorizable builtins */
    if (0) {  /* Never executed but still parsed */
        #pragma GCC ivdep
        for (int i = 0; i < 1024; i++) {
            arr_d2[i] = __builtin_exp(arr_d1[i]) + __builtin_log(arr_d2[i]);
        }
    }
    
    return result;
}

/* Function 4: Hidden visibility helper with mixed types */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_func(double *arr, int n) {
    /* Union for type punning - may trigger vectorized memcpy */
    union {
        double d;
        long long ll;
    } converter;
    
    /* Loop with math builtins */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = exp(arr[i]) * log(fabs(arr[i]) + 1.0);
        
        /* Type punning using builtin memcpy */
        converter.d = arr[i];
        __builtin_memcpy(&arr_i1[i], &converter.ll, sizeof(long long));
    }
}

/* Function 5: Switch statement with different vectorization candidates */
static double test_switch_vectorization(int mode) {
    double result = 0.0;
    
    switch (mode) {
        case 0:
            #pragma omp simd reduction(+:result)
            for (int i = 0; i < 1024; i++) {
                result += sin(arr_d1[i]) + cos(arr_d2[i]);
            }
            break;
            
        case 1:
            #pragma omp simd reduction(+:result)
            for (int i = 0; i < 1024; i++) {
                result += __builtin_pow(arr_d1[i], 2.5) + __builtin_exp(arr_d2[i]);
            }
            break;
            
        case 2:
            #pragma omp simd reduction(+:result)
            for (int i = 0; i < 1024; i++) {
                result += __builtin_fabs(arr_d1[i]) + __builtin_log10(arr_d2[i] + 1.0);
            }
            break;
            
        default:
            /* Mixed integer/floating builtins */
            #pragma omp simd reduction(+:result)
            for (int i = 0; i < 1024; i++) {
                result += __builtin_ilogb(arr_d1[i]) + __builtin_sqrt(arr_d2[i]);
            }
            break;
    }
    
    return result;
}

/* Main function with OpenMP parallel region */
int main(void) {
    init_data();
    
    double total_result = 0.0;
    
    /* OpenMP parallel region with SIMD loops */
    #pragma omp parallel for reduction(+:total_result)
    for (int iter = 0; iter < 10; iter++) {
        /* Test 1: Math vectorization */
        float math_res = test_math_vectorization();
        total_result += math_res;
        
        /* Test 2: Memory builtins */
        size_t mem_res = test_mem_builtins();
        total_result += mem_res;
        
        /* Test 3: Architecture-specific */
        double arch_res = test_arch_specific();
        total_result += arch_res;
        
        /* Test 4: Hidden visibility function */
        hidden_visibility_func(arr_d1, 1024);
        
        /* Test 5: Switch-based vectorization */
        double switch_res = test_switch_vectorization(iter % 4);
        total_result += switch_res;
        
        /* Nested SIMD pragma */
        #pragma omp simd reduction(+:total_result)
        for (int i = 0; i < 1024; i++) {
            total_result += __builtin_sin(arr_d1[i]) * __builtin_cos(arr_d2[i]);
        }
    }
    
    printf("Total result: %f\n", total_result);
    
    /* Prevent dead code elimination */
    volatile double sink = total_result;
    
    return 0;
}
