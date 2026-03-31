/* 
 * Test program designed to trigger GCC's default_builtin_vectorized_function
 * and specifically execute the flag-setting block in targhooks.cc lines 981-990.
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fopt-info-vec -ffast-math
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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

/* Function attributes to interact with declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define NO_THROW __attribute__((nothrow))
#define FORCE_USED __attribute__((used))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Array sizes known at compile-time for vectorization */
#define SIZE 1024
#define ITER 100

/* Global aligned arrays */
static float arr_f1[SIZE] ALIGN_32;
static float arr_f2[SIZE] ALIGN_32;
static double arr_d1[SIZE] ALIGN_64;
static double arr_d2[SIZE] ALIGN_64;
static int arr_i1[SIZE] ALIGN_32;
static char src_str[SIZE] ALIGN_32;
static char dst_str[SIZE] ALIGN_32;

/* Simple random init to prevent compile-time computation */
static void init_data(void) {
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr_f1[i] = (float)rand() / RAND_MAX * 10.0f;
        arr_d1[i] = (double)rand() / RAND_MAX * 10.0;
        arr_i1[i] = rand() % 100;
        src_str[i] = 'A' + (rand() % 26);
    }
    src_str[SIZE-1] = '\0';
}

/* 
 * Test 1: Math-intensive function with OpenMP SIMD pragma
 * Triggers vectorization of sinf/cosf builtins
 */
#pragma omp declare simd
static ALWAYS_INLINE float math_func(float x) {
    return sinf(x) + cosf(x) * 0.5f;
}

static void test_math_vectorization(void) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        arr_f2[i] = math_func(arr_f1[i]);
    }
}

/* 
 * Test 2: Memory operations with __builtin_memcpy in loop
 * May trigger vectorized memcpy builtin
 */
static void test_memcpy_vectorization(void) {
    #pragma GCC ivdep
    for (int i = 0; i < SIZE - 64; i += 64) {
        __builtin_memcpy(&dst_str[i], &src_str[i], 64);
    }
}

/* 
 * Test 3: String length with __builtin_strlen in vectorizable context
 */
static int test_strlen_vectorization(void) {
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < ITER; i++) {
        total += __builtin_strlen(src_str + (i % (SIZE/2)));
    }
    return total;
}

/*
 * Test 4: Architecture-specific intrinsics with CPU dispatch
 * Forces compiler to consider both vector and scalar paths
 */
#ifdef __x86_64__
static void test_avx_intrinsics(void) {
    if (__builtin_cpu_supports("avx2")) {
        for (int i = 0; i < SIZE; i += 8) {
            __m256 vec = _mm256_load_ps(&arr_f1[i]);
            __m256 sqrt_vec = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&arr_f2[i], sqrt_vec);
        }
    } else {
        /* Fallback scalar path with sqrtf calls - vectorizable */
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            arr_f2[i] = sqrtf(arr_f1[i]);
        }
    }
}
#endif

/*
 * Test 5: Hidden visibility function with double precision math
 * DECL_VISIBILITY should be set to VISIBILITY_HIDDEN
 */
HIDDEN_VIS NO_THROW FORCE_USED
static void test_hidden_visibility(void) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        arr_d2[i] = exp(arr_d1[i]) + log(fabs(arr_d1[i]) + 1.0);
    }
}

/*
 * Test 6: Multiple builtins in conditional chain
 * Presents multiple vectorization candidates to compiler
 */
static inline ALWAYS_INLINE void pow_loop(double *out, const double *in) {
    #pragma GCC ivdep
    for (int i = 0; i < SIZE; i++) {
        out[i] = pow(in[i], 2.5);
    }
}

static inline ALWAYS_INLINE void exp_loop(double *out, const double *in) {
    #pragma GCC ivdep
    for (int i = 0; i < SIZE; i++) {
        out[i] = exp(in[i] * 0.5);
    }
}

static inline ALWAYS_INLINE void fabs_loop(double *out, const double *in) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        out[i] = fabs(in[i] - 5.0);
    }
}

static void test_multi_builtins(int mode) {
    switch (mode) {
        case 0:
            pow_loop(arr_d2, arr_d1);
            break;
        case 1:
            exp_loop(arr_d2, arr_d1);
            break;
        case 2:
            fabs_loop(arr_d2, arr_d1);
            break;
        default:
            /* Dead code path with vectorizable builtins */
            if (0) {
                #pragma omp simd
                for (int i = 0; i < SIZE; i++) {
                    arr_d2[i] = sin(arr_d1[i]) * cos(arr_d1[i]);
                }
            }
            break;
    }
}

/*
 * Test 7: Type-punning and mixed data types
 * May trigger vectorization of ilogb and type conversion builtins
 */
static void test_mixed_types(void) {
    union {
        float f;
        int i;
    } converter ALIGN_32;
    
    #pragma omp simd
    for (int j = 0; j < SIZE; j++) {
        converter.f = arr_f1[j];
        arr_i1[j] = __builtin_ilogb(converter.f) + converter.i % 10;
    }
}

/*
 * Test 8: Complex OpenMP context with reduction
 */
static double test_omp_reduction(void) {
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; j++) {
            sum += sqrt(arr_d1[i] + j) * 0.1;
        }
    }
    return sum;
}

/* Main function that runs all tests */
int main(void) {
    double total = 0.0;
    
    init_data();
    
    /* Run all vectorization tests */
    test_math_vectorization();
    test_memcpy_vectorization();
    total += test_strlen_vectorization();
    
    #ifdef __x86_64__
    test_avx_intrinsics();
    #endif
    
    test_hidden_visibility();
    
    /* Test multiple builtins through different paths */
    for (int mode = 0; mode < 4; mode++) {
        test_multi_builtins(mode);
        total += arr_d2[0];
    }
    
    test_mixed_types();
    total += test_omp_reduction();
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %f\n", total);
    printf("Sample values: %f, %f, %d\n", 
           arr_f2[0], arr_d2[0], arr_i1[0]);
    printf("String copy verify: %c%c\n", dst_str[0], dst_str[1]);
    
    return 0;
}
