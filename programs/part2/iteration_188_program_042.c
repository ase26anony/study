/* 
 * This program is designed to trigger GCC's default_builtin_vectorized_function
 * to create vectorized built-in function declarations and set the flags in the
 * uncovered block (TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, etc.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Architecture-specific headers and fallbacks */
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

/* Visibility attributes to interact with DECL_VISIBILITY */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))

/* Always inline to ensure analysis */
#define FORCE_INLINE __attribute__((always_inline)) static inline

/* Size known at compile-time for vectorization */
#define SIZE 1024
#define ITER 100

/* Aligned arrays */
static float arr_f1[SIZE] ALIGN_32;
static float arr_f2[SIZE] ALIGN_32;
static double arr_d1[SIZE] ALIGN_64;
static double arr_d2[SIZE] ALIGN_64;
static int arr_i1[SIZE] ALIGN_32;
static char src_str[SIZE] ALIGN_32;
static char dst_str[SIZE] ALIGN_32;

/* ==================== MATH-INTENSIVE FUNCTIONS ==================== */

/* Function with OpenMP SIMD directive - will vectorize sinf/cosf */
USED_FUNC NOTHROW_FUNC
void math_intensive_loop(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls in loop */
        out[i] = sinf(in[i]) * cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Hidden visibility function - matches DECL_VISIBILITY=HIDDEN */
HIDDEN_VIS static void hidden_visibility_func(double* out, const double* in, int n) {
    #pragma GCC ivdep  /* Ignore vector dependencies */
    for (int i = 0; i < n; i++) {
        /* exp and log built-ins */
        out[i] = exp(in[i]) * log(fabs(in[i]) + 1.0);
    }
}

/* Always-inline helper with pow */
FORCE_INLINE void pow_loop(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.5f);
    }
}

/* Always-inline helper with exp */
FORCE_INLINE void exp_loop(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i] * 0.5f);
    }
}

/* ==================== MEMORY/COPY FUNCTIONS ==================== */

/* Function using __builtin_memcpy in a loop */
static void builtin_memcpy_loop(char* dst, const char* src, int n) {
    for (int i = 0; i < n; i += 64) {
        /* Vectorizable built-in memcpy */
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

/* Function using __builtin_strlen */
static int builtin_strlen_loop(const char* str, int chunks) {
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < chunks; i++) {
        total += __builtin_strlen(str + i * 16);
    }
    return total;
}

/* ==================== ARCHITECTURE-SPECIFIC PATHS ==================== */

/* Conditional path with CPU dispatch */
void architecture_specific_math(float* out, const float* in, int n) {
    /* This condition will be evaluated at compile-time or runtime */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - compiler may still create vectorized built-ins */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(in + i);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(out + i, result);
        }
    } else 
    #endif
    {
        /* Fallback scalar path with sqrtf calls - vectorizable */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sqrtf(in[i]);
        }
    }
}

/* Type-punning with union to trigger built-in vectorization */
union vector_pun {
    __m128 vec;
    float arr[4];
};

static void type_punning_test(float* out, const float* in, int n) {
    union vector_pun pun;
    for (int i = 0; i < n; i += 4) {
        /* This may trigger built-in vectorization for memory operations */
        pun.vec = _mm_load_ps(in + i);
        __builtin_memcpy(out + i, pun.arr, sizeof(pun.arr));
    }
}

/* ==================== COMPLEX CONTROL FLOW ==================== */

/* Multiple vectorization candidates in conditional chain */
static float complex_control_flow(int mode, float x) {
    float result = x;
    
    /* Switch ensures compiler analyzes all paths */
    switch (mode % 4) {
        case 0: {
            /* Dead code path that still gets analyzed */
            if (0) {  /* Never executed but analyzed */
                float temp[SIZE] ALIGN_32;
                #pragma omp simd
                for (int i = 0; i < SIZE; i++) {
                    temp[i] = sinf(arr_f1[i]) * cosf(arr_f1[i]);
                }
                result = temp[0];
            }
            break;
        }
        case 1:
            /* Call always-inline function with pow */
            pow_loop(arr_f2, arr_f1, SIZE);
            result = arr_f2[0];
            break;
        case 2:
            /* Call always-inline function with exp */
            exp_loop(arr_f2, arr_f1, SIZE);
            result = arr_f2[0];
            break;
        case 3:
            /* Direct built-in calls */
            result = fabsf(x) + __builtin_sqrtf(x);
            break;
    }
    return result;
}

/* OpenMP declare simd function - creates SIMD variants */
#pragma omp declare simd
float simd_variant_func(float x) {
    return sinf(x) * cosf(x);
}

/* Nested OpenMP pragmas for complex vectorization context */
static double nested_omp_pragmas(const double* data, int n) {
    double sum = 0.0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < ITER; j++) {
            /* Built-in calls in nested loop */
            sum += log(fabs(data[i]) + 1.0) * exp(data[i] * 0.01 * j);
        }
    }
    return sum;
}

/* ==================== MAIN EXECUTION FLOW ==================== */

/* Simple random init to prevent compile-time computation */
static void init_data(void) {
    for (int i = 0; i < SIZE; i++) {
        arr_f1[i] = (float)(rand() % 1000) / 100.0f + 0.1f;
        arr_d1[i] = (double)(rand() % 1000) / 100.0 + 0.1;
        arr_i1[i] = rand() % 100;
        src_str[i] = 'A' + (rand() % 26);
    }
    src_str[SIZE-1] = '\0';
}

int main(void) {
    double total_result = 0.0;
    
    init_data();
    
    printf("Starting vectorization tests...\n");
    
    /* 1. Math-intensive function with OpenMP SIMD */
    math_intensive_loop(arr_f2, arr_f1, SIZE);
    total_result += arr_f2[0];
    
    /* 2. Hidden visibility function */
    hidden_visibility_func(arr_d2, arr_d1, SIZE);
    total_result += arr_d2[0];
    
    /* 3. Memory/copy functions */
    builtin_memcpy_loop(dst_str, src_str, SIZE);
    total_result += dst_str[0];
    
    int len = builtin_strlen_loop(src_str, SIZE/16);
    total_result += len;
    
    /* 4. Architecture-specific conditional path */
    architecture_specific_math(arr_f2, arr_f1, SIZE);
    total_result += arr_f2[0];
    
    /* 5. Type-punning test */
    #ifdef __x86_64__
    type_punning_test(arr_f2, arr_f1, SIZE);
    total_result += arr_f2[0];
    #endif
    
    /* 6. Complex control flow */
    for (int i = 0; i < 10; i++) {
        total_result += complex_control_flow(i, arr_f1[i % SIZE]);
    }
    
    /* 7. SIMD variant function calls */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        arr_f2[i] = simd_variant_func(arr_f1[i]);
    }
    total_result += arr_f2[0];
    
    /* 8. Nested OpenMP pragmas */
    total_result += nested_omp_pragmas(arr_d1, SIZE/10);
    
    /* Prevent dead code elimination */
    printf("Total result: %f\n", total_result);
    
    return 0;
}
