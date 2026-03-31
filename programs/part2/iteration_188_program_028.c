/* 
 * This program is specifically designed to trigger the flag-setting block
 * in GCC's default_builtin_vectorized_function (targhooks.cc lines 981-990)
 * by creating multiple scenarios where the vectorizer requests vectorized
 * versions of built-in functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Architecture-specific intrinsics */
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

/* Compiler attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Array sizes known at compile-time for vectorization */
#define SIZE 1024
#define ITERATIONS 100

/* Global aligned arrays */
static float arr_f1[SIZE] ALIGN_32;
static float arr_f2[SIZE] ALIGN_32;
static double arr_d1[SIZE] ALIGN_64;
static double arr_d2[SIZE] ALIGN_64;
static int arr_i1[SIZE] ALIGN_32;
static char src_str[SIZE] ALIGN_32;
static char dst_str[SIZE] ALIGN_32;

/* Simple random initialization to prevent compile-time computation */
static void init_data(void) {
    for (int i = 0; i < SIZE; i++) {
        arr_f1[i] = (float)((i * 13) % 100) / 10.0f;
        arr_d1[i] = (double)((i * 17) % 100) / 10.0;
        arr_i1[i] = (i * 19) % 100;
        src_str[i] = 'A' + (i % 26);
    }
    src_str[SIZE-1] = '\0';
}

/* ============================================================================
 * Function 1: Math-intensive with OpenMP SIMD pragma
 * Triggers vectorization of sinf/cosf built-ins
 * ============================================================================
 */
USED_FUNC NOTHROW_FUNC
static void math_intensive_vectorized(float *out1, float *out2, const float *in, int n) {
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; i++) {
        /* These built-in calls should trigger vectorized versions */
        out1[i] = sinf(in[i]);
        out2[i] = cosf(in[i]);
    }
}

/* ============================================================================
 * Function 2: Memory operations with builtin_memcpy in loop
 * Triggers vectorization of memcpy built-in
 * ============================================================================
 */
ALWAYS_INLINE
static inline void memory_ops_vectorized(char *dst, const char *src, int n) {
    for (int i = 0; i < n; i += 64) {
        /* Vectorized built-in memcpy */
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

/* ============================================================================
 * Function 3: Conditional architecture-specific paths
 * Both paths contain vectorizable built-in calls
 * ============================================================================
 */
static void conditional_vectorization(double *out, const double *in, int n) {
    /* Complex conditional to ensure both paths are analyzed */
    int use_vector_path = 0;
    
    #ifdef __x86_64__
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    use_vector_path = (ecx & bit_AVX) ? 1 : 0;
    #endif
    
    if (use_vector_path) {
        /* Vector path with intrinsics - may still trigger built-in vectorization */
        #ifdef __x86_64__
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(in + i);
            __m256d result = _mm256_sqrt_pd(vec);
            _mm256_store_pd(out + i, result);
        }
        #endif
    } else {
        /* Scalar fallback with sqrt built-in calls */
        #pragma GCC ivdep
        for (int i = 0; i < n; i++) {
            /* This should trigger vectorized sqrt built-in */
            out[i] = sqrt(in[i]);
        }
    }
}

/* ============================================================================
 * Function 4: Hidden visibility helper with mixed math operations
 * Aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN
 * ============================================================================
 */
HIDDEN_VIS USED_FUNC
static void hidden_visibility_helper(double *out, const double *in, int n) {
    #pragma omp simd reduction(+:out[:n])
    for (int i = 0; i < n; i++) {
        /* Multiple built-ins in one loop */
        out[i] = exp(in[i]) + log(fabs(in[i]) + 1.0);
    }
}

/* ============================================================================
 * Function 5: Multiple small inline functions with different built-ins
 * Structured to present multiple vectorization candidates
 * ============================================================================
 */
ALWAYS_INLINE static inline void process_pow(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);
    }
}

ALWAYS_INLINE static inline void process_exp(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

ALWAYS_INLINE static inline void process_fabs(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = fabsf(in[i]);
    }
}

/* Function that selects between different vectorizable operations */
static void multi_builtin_selector(int op_type, float *out, const float *in, int n) {
    switch (op_type) {
        case 0:
            process_pow(out, in, n);
            break;
        case 1:
            process_exp(out, in, n);
            break;
        case 2:
            process_fabs(out, in, n);
            break;
        default:
            /* Dead code path that still contains vectorizable built-ins */
            if (0) {  /* Always false, but compiler still analyzes */
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    out[i] = sinf(in[i]) * cosf(in[i]);
                }
            }
            break;
    }
}

/* ============================================================================
 * Function 6: Type-punning and mixed operations
 * Uses unions and builtin_memcpy for type conversion
 * ============================================================================
 */
static void type_punning_operations(float *out, const int *in, int n) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* Use __builtin_ilogb for integer log */
        int exp = __builtin_ilogb(in[i] + 1);
        
        /* Type punning via builtin_memcpy */
        converter.i = in[i];
        __builtin_memcpy(&out[i], &converter.f, sizeof(float));
        
        /* Additional math operation */
        out[i] = sqrtf(out[i] + exp);
    }
}

/* ============================================================================
 * Function 7: OpenMP parallel region with SIMD reduction
 * Complex vectorization context
 * ============================================================================
 */
#pragma omp declare simd
static float simd_reduction_kernel(const float *a, const float *b, int i) {
    return sinf(a[i]) + cosf(b[i]);
}

static float parallel_simd_reduction(const float *a, const float *b, int n) {
    float total = 0.0f;
    
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < n; i++) {
        #pragma omp simd reduction(+:total)
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < n) {
                total += simd_reduction_kernel(a, b, idx);
            }
        }
    }
    
    return total;
}

/* ============================================================================
 * Function 8: String operation with builtin_strlen
 * May trigger vectorized strlen
 * ============================================================================
 */
static int vectorized_strlen_operations(const char *str, int iterations) {
    int total_len = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Built-in strlen in a loop - may be vectorized */
        total_len += __builtin_strlen(str + (iter % (SIZE/2)));
    }
    
    return total_len;
}

/* ============================================================================
 * Main function: Orchestrates all test cases
 * ============================================================================
 */
int main(void) {
    double total_result = 0.0;
    
    /* Initialize data */
    init_data();
    
    printf("Starting vectorization tests...\n");
    
    /* Test 1: Math-intensive vectorization */
    math_intensive_vectorized(arr_f2, arr_f1, arr_f1, SIZE);
    total_result += arr_f2[SIZE/2] + arr_f1[SIZE/3];
    
    /* Test 2: Memory operations */
    memory_ops_vectorized(dst_str, src_str, SIZE);
    total_result += dst_str[SIZE/4];
    
    /* Test 3: Conditional vectorization */
    conditional_vectorization(arr_d2, arr_d1, SIZE);
    total_result += arr_d2[SIZE/2];
    
    /* Test 4: Hidden visibility helper */
    hidden_visibility_helper(arr_d2, arr_d1, SIZE);
    total_result += arr_d2[SIZE/3];
    
    /* Test 5: Multiple built-in selector */
    for (int op = 0; op < 3; op++) {
        multi_builtin_selector(op, arr_f2, arr_f1, SIZE);
        total_result += arr_f2[op];
    }
    
    /* Test 6: Type-punning operations */
    type_punning_operations(arr_f2, arr_i1, SIZE);
    total_result += arr_f2[SIZE/2];
    
    /* Test 7: Parallel SIMD reduction */
    float reduction_result = parallel_simd_reduction(arr_f1, arr_f2, SIZE);
    total_result += reduction_result;
    
    /* Test 8: Vectorized strlen */
    int str_len_total = vectorized_strlen_operations(src_str, ITERATIONS);
    total_result += str_len_total;
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total_result);
    printf("String length sum: %d\n", str_len_total);
    
    return 0;
}
