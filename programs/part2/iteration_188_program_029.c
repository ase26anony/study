/* 
 * This program is designed to trigger GCC's vectorization of built-in functions,
 * specifically targeting the flag-setting block in default_builtin_vectorized_function
 * in targhooks.cc (lines 981-990).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Architecture-specific intrinsics */
#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Array sizes known at compile-time for vectorization */
#define SIZE 1024
#define BLOCK_SIZE 256

/* Aligned arrays */
static float arr_f1[SIZE] ALIGN_32;
static float arr_f2[SIZE] ALIGN_32;
static double arr_d1[SIZE] ALIGN_64;
static double arr_d2[SIZE] ALIGN_64;
static int arr_i1[SIZE] ALIGN_32;
static char src_str[BLOCK_SIZE] ALIGN_32;
static char dst_str[BLOCK_SIZE] ALIGN_32;

/* Simple random initialization to prevent compile-time computation */
static void init_data(void) {
    for (int i = 0; i < SIZE; i++) {
        arr_f1[i] = (float)(rand() % 1000) / 100.0f;
        arr_d1[i] = (double)(rand() % 1000) / 100.0;
        arr_i1[i] = rand() % 1000;
    }
    for (int i = 0; i < BLOCK_SIZE; i++) {
        src_str[i] = 'A' + (rand() % 26);
    }
    src_str[BLOCK_SIZE-1] = '\0';
}

/* ============================================================================
 * 1. Math-intensive function with OpenMP SIMD directive
 * ============================================================================
 */
USED_FUNC NOTHROW_FUNC static void math_intensive_vectorized(void) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Multiple built-in calls to trigger vectorization */
        arr_f2[i] = sinf(arr_f1[i]) + cosf(arr_f1[i]) + sqrtf(arr_f1[i]);
    }
}

/* ============================================================================
 * 2. Memory/copy function using __builtin_memcpy in a loop
 * ============================================================================
 */
ALWAYS_INLINE static inline void copy_blocks(void) {
    for (int i = 0; i < SIZE; i += 16) {
        /* Vectorized built-in memory copy */
        __builtin_memcpy(&dst_str[i], &src_str[i], 16);
    }
}

/* ============================================================================
 * 3. Conditional CPU dispatch with architecture-specific intrinsics
 * ============================================================================
 */
static void conditional_vectorization(void) {
    double sum = 0.0;
    
    /* Conditional path - both branches analyzed by compiler */
    if (__builtin_cpu_supports("avx2")) {
        /* AVX intrinsic path */
        #ifdef __x86_64__
        for (int i = 0; i < SIZE; i += 4) {
            __m256d vec = _mm256_load_pd(&arr_d1[i]);
            __m256d sqrt_vec = _mm256_sqrt_pd(vec);
            _mm256_store_pd(&arr_d2[i], sqrt_vec);
            sum += arr_d2[i] + arr_d2[i+1] + arr_d2[i+2] + arr_d2[i+3];
        }
        #endif
    } else {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            arr_d2[i] = sqrt(arr_d1[i]);  /* Built-in sqrt */
            sum += arr_d2[i];
        }
    }
    
    /* Prevent dead code elimination */
    arr_d2[0] = sum / SIZE;
}

/* ============================================================================
 * 4. Hidden visibility helper with mixed built-in calls
 * ============================================================================
 */
HIDDEN_VIS static void hidden_visibility_helper(void) {
    double temp[SIZE] ALIGN_64;
    
    /* Complex loop with multiple built-in functions */
    #pragma GCC ivdep
    for (int i = 0; i < SIZE; i++) {
        /* Multiple built-ins to increase vectorization opportunities */
        temp[i] = exp(arr_d1[i]) * log(fabs(arr_d1[i]) + 1.0);
        temp[i] += pow(arr_d1[i], 2.5);
    }
    
    /* Use temp to prevent elimination */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        arr_d2[i] += temp[i];
    }
}

/* ============================================================================
 * 5. Function with OpenMP declare simd for SIMD variant creation
 * ============================================================================
 */
#pragma omp declare simd
static double simd_math_function(double x) {
    return sin(x) * cos(x) + sqrt(fabs(x));
}

static void test_declare_simd(void) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        arr_d2[i] = simd_math_function(arr_d1[i]);
    }
}

/* ============================================================================
 * 6. Type-punning and mixed data type operations
 * ============================================================================
 */
static void mixed_type_operations(void) {
    union {
        float f;
        int i;
    } converter ALIGN_32;
    
    /* Loop with type conversions and built-ins */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Use __builtin_ilogb for integer result */
        arr_i1[i] = __builtin_ilogb(arr_f1[i]);
        
        /* Type punning through memory */
        converter.f = arr_f1[i];
        arr_i1[i] += converter.i & 0x7FFFFF;
    }
}

/* ============================================================================
 * 7. Dead code path with vectorizable built-ins (still processed by front-end)
 * ============================================================================
 */
static void dead_code_path(void) {
    /* This path is never executed but still analyzed */
    if (0) {  /* Always false */
        float dead_arr[SIZE] ALIGN_32;
        
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            /* Vectorizable built-ins in dead code */
            dead_arr[i] = sinf(arr_f1[i]) * cosf(arr_f1[i]);
            dead_arr[i] = expf(dead_arr[i]) + logf(dead_arr[i] + 1.0f);
        }
        
        /* Use __builtin_strlen in vectorizable context */
        int len = __builtin_strlen(src_str);
        dead_arr[0] = (float)len;
    }
}

/* ============================================================================
 * 8. Nested OpenMP pragmas for complex vectorization context
 * ============================================================================
 */
static void nested_omp_vectorization(void) {
    double total = 0.0;
    
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < SIZE; i++) {
        double local_sum = 0.0;
        
        #pragma omp simd reduction(+:local_sum)
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < SIZE) {
                /* Built-in calls in nested SIMD loop */
                local_sum += sin(arr_d1[idx]) + cos(arr_d1[idx]);
            }
        }
        
        total += local_sum;
    }
    
    arr_d2[0] = total;
}

/* ============================================================================
 * Main function: Orchestrates all test cases
 * ============================================================================
 */
int main(void) {
    double checksum = 0.0;
    
    /* Initialize data */
    init_data();
    
    /* Execute all vectorization test cases */
    math_intensive_vectorized();          /* Test case 1 */
    copy_blocks();                        /* Test case 2 */
    conditional_vectorization();          /* Test case 3 */
    hidden_visibility_helper();           /* Test case 4 */
    test_declare_simd();                  /* Test case 5 */
    mixed_type_operations();              /* Test case 6 */
    dead_code_path();                     /* Test case 7 */
    nested_omp_vectorization();           /* Test case 8 */
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += arr_f2[i] + arr_d2[i] + arr_i1[i];
    }
    checksum += dst_str[0] + src_str[0];
    
    printf("Result checksum: %f\n", checksum);
    return 0;
}
