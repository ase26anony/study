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

/* Alignment hints for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence visibility and linkage */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Array sizes - known at compile time for vectorization */
#define N 1024
#define M 256

/* Global aligned arrays */
static float arr_f1[N] ALIGN_32;
static float arr_f2[N] ALIGN_32;
static double arr_d1[N] ALIGN_64;
static double arr_d2[N] ALIGN_64;
static int arr_i1[M] ALIGN_32;
static char src_buf[M] ALIGN_32;
static char dst_buf[M] ALIGN_32;

/* Simple random initialization to prevent compile-time computation */
static void init_data(void) {
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        arr_f1[i] = (float)rand() / RAND_MAX * 10.0f;
        arr_f2[i] = (float)rand() / RAND_MAX * 10.0f;
        arr_d1[i] = (double)rand() / RAND_MAX * 10.0;
        arr_d2[i] = (double)rand() / RAND_MAX * 10.0;
    }
    for (int i = 0; i < M; i++) {
        arr_i1[i] = rand() % 100;
        src_buf[i] = 'A' + (rand() % 26);
    }
}

/* ====================================================================
   Function 1: Math-intensive with explicit SIMD pragma
   Triggers vectorization of sinf/cosf builtins
   ==================================================================== */
USED_FUNC NOTHROW_FUNC static void math_intensive_vectorized(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple builtin calls in vectorizable loop */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* ====================================================================
   Function 2: Memory operations with builtin memcpy in loop
   ==================================================================== */
static void memory_operations_vectorized(void) {
    #pragma omp simd
    for (int i = 0; i < M - 32; i += 32) {
        /* Builtin memcpy in vectorizable context */
        __builtin_memcpy(&dst_buf[i], &src_buf[i], 32);
    }
    
    /* strlen in vectorizable loop */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < M/4; i++) {
        total_len += __builtin_strlen(&src_buf[i*4]);
    }
    (void)total_len; /* Prevent unused warning */
}

/* ====================================================================
   Function 3: Conditional architecture-specific paths
   Uses CPU dispatch to select vector/scalar paths
   ==================================================================== */
ALWAYS_INLINE static void process_with_sqrt(double* out, const double* in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        out[i] = sqrt(in[i]) + pow(in[i], 2.0);
    }
}

#ifdef __x86_64__
static int cpu_supports_avx2(void) {
    unsigned int eax, ebx, ecx, edx;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 5)) != 0;
}

static void avx2_vector_path(float* out, const float* in, int n) {
    /* AVX2 intrinsic usage - compiler may consider builtin alternatives */
    for (int i = 0; i < n; i += 8) {
        __m256 vec = _mm256_load_ps(&in[i]);
        __m256 result = _mm256_sqrt_ps(vec);
        _mm256_store_ps(&out[i], result);
    }
}
#endif

static void conditional_vectorization(void) {
    /* This conditional ensures both paths are analyzed */
    if (__builtin_cpu_supports("avx2")) {
#ifdef __x86_64__
        avx2_vector_path(arr_f2, arr_f1, N);
#endif
    } else {
        /* Fallback with scalar builtins in vectorizable loop */
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            arr_f2[i] = sqrtf(arr_f1[i]) + expf(arr_f1[i]);
        }
    }
    
    /* Dead code path that still contains vectorizable builtins */
    if (0) { /* Always false, but frontend processes declarations */
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            arr_d2[i] = log(arr_d1[i]) + __builtin_exp(arr_d1[i]);
        }
    }
}

/* ====================================================================
   Function 4: Hidden visibility helper with mixed operations
   ==================================================================== */
HIDDEN_VIS static void hidden_visibility_helper(double* result, int n) {
    /* Mixed data types and builtins */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Type conversions and multiple builtins */
        double val = arr_d1[i];
        result[i] = exp(val) + log(val + 1.0) + __builtin_fabs(val);
    }
    
    /* Integer builtins */
    #pragma omp simd
    for (int i = 0; i < M; i++) {
        arr_i1[i] = __builtin_abs(arr_i1[i]) + __builtin_ilogb(arr_f1[i % N]);
    }
}

/* ====================================================================
   Function 5: OpenMP declare simd function
   Creates SIMD variants of functions with builtins
   ==================================================================== */
#pragma omp declare simd
static float simd_math_function(float x) {
    return sinf(x) * cosf(x) + sqrtf(x);
}

static void test_declare_simd(void) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        arr_f2[i] = simd_math_function(arr_f1[i]);
    }
}

/* ====================================================================
   Function 6: Complex nested pragmas with reduction
   ==================================================================== */
static double complex_reduction_loop(void) {
    double sum = 0.0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 8; j++) {
            double idx = arr_d1[i] + j * 0.1;
            sum += exp(idx) + log(idx + 1.0);
        }
    }
    return sum;
}

/* ====================================================================
   Function 7: Type punning via union (may trigger memcpy vectorization)
   ==================================================================== */
typedef union {
    __m128 vec;
    float arr[4];
} vec_union ALIGN_32;

static void type_punning_operations(void) {
    vec_union u;
    
    #pragma omp simd
    for (int i = 0; i < N/4; i++) {
        /* Simulate type punning */
        u.vec = _mm_load_ps(&arr_f1[i*4]);
        __builtin_memcpy(&arr_f2[i*4], u.arr, sizeof(float) * 4);
    }
}

/* ====================================================================
   Function 8: Switch statement with multiple vectorization candidates
   ==================================================================== */
static float process_with_switch(int mode, float x) {
    float result = 0.0f;
    
    switch (mode) {
        case 0:
            result = sinf(x);
            break;
        case 1:
            result = cosf(x);
            break;
        case 2:
            result = sqrtf(x);
            break;
        case 3:
            result = expf(x);
            break;
        case 4:
            result = logf(x + 1.0f);
            break;
        default:
            result = __builtin_powf(x, 1.5f);
            break;
    }
    return result;
}

static void switch_based_vectorization(void) {
    /* Loop with switch inside - vectorizer analyzes all paths */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        int mode = i % 5;
        arr_f2[i] = process_with_switch(mode, arr_f1[i]);
    }
}

/* ====================================================================
   Main function: Orchestrates all test cases
   ==================================================================== */
int main(void) {
    double sum = 0.0;
    
    init_data();
    
    /* Test 1: Math intensive vectorization */
    math_intensive_vectorized(arr_f2, arr_f1, N);
    
    /* Test 2: Memory operations */
    memory_operations_vectorized();
    
    /* Test 3: Conditional CPU dispatch */
    conditional_vectorization();
    
    /* Test 4: Hidden visibility helper */
    hidden_visibility_helper(arr_d2, N);
    
    /* Test 5: OpenMP declare simd */
    test_declare_simd();
    
    /* Test 6: Complex reduction */
    sum = complex_reduction_loop();
    
    /* Test 7: Type punning */
#ifdef __x86_64__
    type_punning_operations();
#endif
    
    /* Test 8: Switch-based */
    switch_based_vectorization();
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        sum += arr_f2[i] + arr_d2[i];
    }
    
    printf("Result: %f\n", sum);
    return 0;
}
