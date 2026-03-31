/* 
 * Comprehensive test program to trigger GCC's default_builtin_vectorized_function
 * and specifically exercise the flag-setting block in targhooks.cc (lines 981-990)
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

/* Alignment attributes for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Simple random generator to prevent compile-time computation */
static float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7FFFFFFF) / 0x7FFFFFFF;
}

/* ============================================
   Function 1: Math-intensive with explicit SIMD
   ============================================ */
#pragma omp declare simd
static float math_func_simd(float x) {
    return sinf(x) * cosf(x) + sqrtf(fabsf(x));
}

__attribute__((visibility("hidden")))
void hidden_visibility_math(float* ALIGN_32 in, float* ALIGN_32 out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = math_func_simd(in[i]);
    }
}

/* ============================================
   Function 2: Memory operations with builtins
   ============================================ */
__attribute__((used, nothrow))
static void builtin_memcpy_loop(char* ALIGN_32 dst, char* ALIGN_32 src, int size, int chunks) {
    for (int i = 0; i < chunks; i++) {
        /* Using __builtin_memcpy to trigger vectorization */
        __builtin_memcpy(dst + i * 64, src + i * 64, 64);
    }
}

/* ============================================
   Function 3: Architecture-specific intrinsics
   ============================================ */
#ifdef __x86_64__
__attribute__((always_inline))
static inline void avx2_vector_sqrt(float* ALIGN_32 in, float* ALIGN_32 out, int n) {
    for (int i = 0; i < n; i += 8) {
        __m256 vec = _mm256_load_ps(&in[i]);
        __m256 result = _mm256_sqrt_ps(vec);
        _mm256_store_ps(&out[i], result);
    }
}
#endif

__attribute__((noinline))
void conditional_architecture_path(float* ALIGN_32 in, float* ALIGN_32 out, int n) {
    /* Complex control flow with both vector and scalar paths */
    int use_vector = 0;
    
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        use_vector = 1;
        avx2_vector_sqrt(in, out, n);
    }
    #endif
    
    if (!use_vector) {
        /* Fallback scalar path with built-in sqrtf */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sqrtf(in[i]);
        }
    }
}

/* ============================================
   Function 4: Multiple builtins in small functions
   ============================================ */
__attribute__((always_inline))
static inline void pow_loop(double* ALIGN_32 in, double* ALIGN_32 out, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        out[i] = pow(in[i], 2.5);
    }
}

__attribute__((always_inline))
static inline void exp_log_loop(double* ALIGN_32 in, double* ALIGN_32 out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = exp(log(in[i] + 1.0));
    }
}

__attribute__((always_inline))
static inline void ilogb_loop(float* ALIGN_32 in, int* ALIGN_32 out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = __builtin_ilogb(in[i]);
    }
}

/* ============================================
   Function 5: Type-punning and mixed operations
   ============================================ */
typedef union {
    __m128 vec;
    float arr[4];
} vec_union;

void type_punning_operations(float* ALIGN_32 data, int n) {
    vec_union u;
    for (int i = 0; i < n; i += 4) {
        /* Type punning that may trigger builtin vectorization */
        u.vec = _mm_load_ps(&data[i]);
        __builtin_memcpy(&data[i], &u.arr, sizeof(float) * 4);
    }
}

/* ============================================
   Function 6: Dead code path with vectorization
   ============================================ */
__attribute__((noinline))
void dead_code_path(float* ALIGN_32 data, int n) {
    /* This dead code still gets analyzed by the front-end */
    if (0) {  /* Always false */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            data[i] = sinf(data[i]) * cosf(data[i]);
        }
    }
}

/* ============================================
   Function 7: OpenMP parallel with SIMD reduction
   ============================================ */
double omp_simd_reduction(double* ALIGN_64 data, int n) {
    double sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += sqrt(data[i]) * exp(data[i] * 0.01);
    }
    
    return sum;
}

/* ============================================
   Function 8: Switch with multiple vectorization candidates
   ============================================ */
__attribute__((flatten))
void switch_vectorization(int mode, float* in, float* out, int n) {
    switch (mode) {
        case 0:
            pow_loop((double*)in, (double*)out, n/2);
            break;
        case 1:
            exp_log_loop((double*)in, (double*)out, n/2);
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sinf(in[i]) + cosf(in[i]);
            }
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sqrtf(fabsf(in[i]));
            }
    }
}

/* ============================================
   Main function: Orchestrates all test cases
   ============================================ */
int main() {
    const int N = 1024;
    const int MEM_CHUNKS = 16;
    
    /* Aligned arrays */
    float* data_f ALIGN_32 = (float*)aligned_alloc(32, N * sizeof(float));
    float* result_f ALIGN_32 = (float*)aligned_alloc(32, N * sizeof(float));
    double* data_d ALIGN_64 = (double*)aligned_alloc(64, N * sizeof(double));
    double* result_d ALIGN_64 = (double*)aligned_alloc(64, N * sizeof(double));
    char* mem_src ALIGN_32 = (char*)aligned_alloc(32, 1024);
    char* mem_dst ALIGN_32 = (char*)aligned_alloc(32, 1024);
    int* result_i ALIGN_32 = (int*)aligned_alloc(32, N * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        data_f[i] = simple_rand(i) * 10.0f + 0.1f;
        data_d[i] = (double)data_f[i];
        result_i[i] = 0;
    }
    
    for (int i = 0; i < 1024; i++) {
        mem_src[i] = (char)(i % 256);
    }
    
    double total_sum = 0.0;
    
    /* Test 1: Hidden visibility math function */
    hidden_visibility_math(data_f, result_f, N);
    for (int i = 0; i < 8; i++) total_sum += result_f[i];
    
    /* Test 2: Builtin memcpy in loop */
    builtin_memcpy_loop(mem_dst, mem_src, 64, MEM_CHUNKS);
    total_sum += mem_dst[10] + mem_dst[100];
    
    /* Test 3: Conditional architecture path */
    conditional_architecture_path(data_f, result_f, N);
    for (int i = 0; i < 8; i++) total_sum += result_f[i];
    
    /* Test 4: Integer logb */
    ilogb_loop(data_f, result_i, N);
    for (int i = 0; i < 8; i++) total_sum += result_i[i];
    
    /* Test 5: Type punning (x86 only) */
    #ifdef __x86_64__
    type_punning_operations(data_f, N);
    #endif
    
    /* Test 6: Dead code path (still analyzed) */
    dead_code_path(data_f, N);
    
    /* Test 7: OpenMP SIMD reduction */
    total_sum += omp_simd_reduction(data_d, N);
    
    /* Test 8: Switch with multiple paths */
    for (int mode = 0; mode < 4; mode++) {
        switch_vectorization(mode, data_f, result_f, N);
        for (int i = 0; i < 4; i++) total_sum += result_f[i];
    }
    
    /* Test 9: strlen builtin in vectorizable context */
    char test_str[] = "test_string_for_vectorization_consideration";
    int len_sum = 0;
    #pragma omp simd reduction(+:len_sum)
    for (int i = 0; i < 10; i++) {
        len_sum += __builtin_strlen(test_str + (i % 5));
    }
    total_sum += len_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    
    /* Cleanup */
    free(data_f);
    free(result_f);
    free(data_d);
    free(result_d);
    free(mem_src);
    free(mem_dst);
    free(result_i);
    
    return 0;
}
