/* 
 * Comprehensive test program to trigger GCC's default_builtin_vectorized_function
 * and cover the flag-setting block in targhooks.cc (lines 981-990)
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

/* Compile-time alignment for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* OpenMP SIMD declarations */
#pragma omp declare simd uniform(a, b) linear(i)
double simd_multiply_add(double a, double b, int i);

/* Simple random generator to prevent compile-time computation */
static float simple_rand(int seed) {
    static unsigned int state = 0;
    if (seed) state = (unsigned int)seed;
    state = state * 1103515245 + 12345;
    return (float)(state % 1000) / 1000.0f;
}

/* ========== TEST FUNCTION 1: Math-intensive with OpenMP SIMD ========== */
/* This should trigger vectorization of sinf/cosf builtins */
HIDDEN_VIS USED_FUNC NOTHROW_FUNC
static void math_intensive_vectorized(float* ALIGN_32 in, float* ALIGN_32 out1, 
                                      float* ALIGN_32 out2, int n) {
    #pragma omp simd safelen(16)
    for (int i = 0; i < n; i++) {
        /* Multiple builtin calls in vectorized loop */
        out1[i] = sinf(in[i]) * cosf(in[i]);
        out2[i] = sqrtf(fabsf(in[i])) + logf(fabsf(in[i]) + 1.0f);
    }
}

/* ========== TEST FUNCTION 2: Memory operations with builtins ========== */
ALWAYS_INLINE static inline void memory_operations(char* ALIGN_32 dst, 
                                                   const char* ALIGN_32 src, 
                                                   int n) {
    for (int i = 0; i < n; i += 64) {
        /* Use __builtin_memcpy in vectorizable context */
        __builtin_memcpy(dst + i, src + i, 64);
        
        /* strlen in loop - may get vectorized */
        int len = __builtin_strlen(src + i);
        dst[i] = (char)(len & 0xFF);
    }
}

/* ========== TEST FUNCTION 3: Conditional architecture paths ========== */
/* Complex control flow with both vector and scalar paths */
double conditional_vectorization(double* ALIGN_32 arr, int n, int mode) {
    double sum = 0.0;
    
    /* Switch with multiple vectorization candidates */
    switch (mode) {
        case 0: {
            /* Path with pow/exp builtins */
            #pragma GCC ivdep
            for (int i = 0; i < n; i++) {
                sum += pow(arr[i], 2.5) + exp(arr[i] * 0.5);
            }
            break;
        }
            
        case 1: {
            /* Path with log/fabs builtins */
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; i++) {
                sum += log(fabs(arr[i]) + 1.0) * __builtin_sqrt(arr[i] + 1.0);
            }
            break;
        }
            
        case 2: {
            /* Dead code path that still gets analyzed */
            if (0) { /* Always false, but declarations are processed */
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    sum += __builtin_ilogb(arr[i]) * __builtin_exp(arr[i]);
                }
            }
            /* Real computation path */
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; i++) {
                sum += arr[i] * arr[i];
            }
            break;
        }
    }
    
    return sum;
}

/* ========== TEST FUNCTION 4: Architecture-specific intrinsics ========== */
#ifdef __x86_64__
static float avx_vectorized_math(float* ALIGN_32 data, int n) {
    float sum = 0.0f;
    
    /* Conditional on CPU support */
    if (__builtin_cpu_supports("avx2")) {
        /* AVX intrinsic path */
        __m256 sum_vec = _mm256_setzero_ps();
        for (int i = 0; i < n; i += 8) {
            __m256 data_vec = _mm256_load_ps(&data[i]);
            /* Simulate builtin operations with intrinsics */
            __m256 sin_approx = _mm256_mul_ps(data_vec, data_vec);
            sum_vec = _mm256_add_ps(sum_vec, sin_approx);
        }
        /* Horizontal add */
        sum_vec = _mm256_hadd_ps(sum_vec, sum_vec);
        sum = ((float*)&sum_vec)[0] + ((float*)&sum_vec)[4];
    } else {
        /* Fallback scalar path with builtin calls */
        #pragma omp simd reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += sinf(data[i]) + cosf(data[i]) + sqrtf(data[i]);
        }
    }
    
    return sum;
}
#endif

/* ========== TEST FUNCTION 5: Mixed data types and unions ========== */
typedef union {
    float f[8];
    int i[8];
    #ifdef __x86_64__
    __m256 v;
    #endif
} mixed_data_t ALIGN_32;

static void type_punning_operations(mixed_data_t* ALIGN_32 data, int n) {
    for (int i = 0; i < n; i++) {
        /* Type punning through memcpy - may trigger builtin vectorization */
        float temp[8];
        __builtin_memcpy(temp, data[i].f, sizeof(temp));
        
        /* Mixed type operations */
        #pragma omp simd
        for (int j = 0; j < 8; j++) {
            data[i].f[j] = temp[j] * __builtin_sqrt(temp[j]);
            data[i].i[j] = __builtin_ilogb(data[i].f[j]);
        }
    }
}

/* ========== TEST FUNCTION 6: Nested OpenMP with SIMD ========== */
#pragma omp declare simd
double simd_multiply_add(double a, double b, int i) {
    return a * b + i;
}

static double nested_openmp_simd(double* ALIGN_32 a, double* ALIGN_32 b, int n) {
    double total = 0.0;
    
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < n; i++) {
        #pragma omp simd reduction(+:total)
        for (int j = 0; j < 16; j++) {
            total += simd_multiply_add(a[i], b[j], i * j);
        }
    }
    
    return total;
}

/* ========== MAIN FUNCTION ========== */
int main() {
    const int N = 1024;
    const int M = 128;
    
    /* Aligned arrays with different types */
    float* fdata1 = (float*)aligned_alloc(32, N * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(32, N * sizeof(float));
    float* fout1 = (float*)aligned_alloc(32, N * sizeof(float));
    float* fout2 = (float*)aligned_alloc(32, N * sizeof(float));
    double* ddata = (double*)aligned_alloc(64, N * sizeof(double));
    char* cdata1 = (char*)aligned_alloc(32, M * sizeof(char));
    char* cdata2 = (char*)aligned_alloc(32, M * sizeof(char));
    mixed_data_t* mdata = (mixed_data_t*)aligned_alloc(32, 16 * sizeof(mixed_data_t));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        fdata1[i] = simple_rand(i);
        fdata2[i] = simple_rand(i + N);
        ddata[i] = (double)simple_rand(i * 2) * 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        cdata1[i] = (char)(rand() % 256);
        cdata2[i] = 0;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            mdata[i].f[j] = simple_rand(i * 8 + j);
        }
    }
    
    /* Execute all test functions to trigger vectorization */
    printf("Starting vectorization tests...\n");
    
    /* Test 1: Math-intensive vectorization */
    math_intensive_vectorized(fdata1, fout1, fout2, N);
    printf("Test 1 complete: Math vectorization\n");
    
    /* Test 2: Memory operations */
    memory_operations(cdata2, cdata1, M);
    printf("Test 2 complete: Memory builtins\n");
    
    /* Test 3: Conditional paths */
    double sum1 = conditional_vectorization(ddata, N, 0);
    double sum2 = conditional_vectorization(ddata, N, 1);
    double sum3 = conditional_vectorization(ddata, N, 2);
    printf("Test 3 complete: Conditional paths (sums: %.2f, %.2f, %.2f)\n", sum1, sum2, sum3);
    
    /* Test 4: Architecture-specific */
    #ifdef __x86_64__
    float avx_sum = avx_vectorized_math(fdata1, N);
    printf("Test 4 complete: AVX math (sum: %.2f)\n", avx_sum);
    #endif
    
    /* Test 5: Type punning */
    type_punning_operations(mdata, 16);
    printf("Test 5 complete: Type punning\n");
    
    /* Test 6: Nested OpenMP SIMD */
    double nested_sum = nested_openmp_simd(ddata, ddata + 16, N - 16);
    printf("Test 6 complete: Nested OpenMP SIMD (sum: %.2f)\n", nested_sum);
    
    /* Prevent dead code elimination */
    volatile float check = fout1[N/2] + fout2[N/2];
    check += (float)cdata2[M/2];
    check += (float)sum1;
    
    printf("All tests completed. Final check value: %f\n", (double)check);
    
    /* Cleanup */
    free(fdata1);
    free(fdata2);
    free(fout1);
    free(fout2);
    free(ddata);
    free(cdata1);
    free(cdata2);
    free(mdata);
    
    return 0;
}
