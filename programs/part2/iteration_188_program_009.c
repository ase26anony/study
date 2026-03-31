/* 
 * Test program designed to trigger GCC's default_builtin_vectorized_function
 * and specifically execute the flag-setting block in targhooks.cc lines 981-990.
 * Compile with: g++ -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fopt-info-vec -ffast-math
 */

#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints to engage vectorizer's alignment analysis */
#define ALIGNED_32 __attribute__((aligned(32)))
#define ALIGNED_64 __attribute__((aligned(64)))

/* Simple random generator to prevent compile-time computation */
static inline float random_float() {
    static unsigned seed = 12345;
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

/* Function with hidden visibility - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float* ALIGNED_32 out, const float* ALIGNED_32 in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Multiple built-in math functions to trigger vectorization */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with always_inline attribute */
static inline __attribute__((always_inline)) 
void inline_math_pow(double* ALIGNED_32 out, const double* ALIGNED_32 in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        out[i] = pow(in[i], 2.5) + exp(in[i] * 0.5);
    }
}

/* Function with OpenMP declare simd to create SIMD variants */
#pragma omp declare simd
__attribute__((used)) float simd_math_function(float x) {
    return logf(x + 1.0f) * expf(x);
}

/* Memory/copy function using __builtin_memcpy */
__attribute__((noinline))
void builtin_memcpy_loop(char* ALIGNED_64 dst, const char* ALIGNED_64 src, int size, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        /* Using __builtin_memcpy in a loop */
        __builtin_memcpy(dst + i * 64, src + i * 64, 64);
    }
}

/* Conditional function with architecture-specific paths */
__attribute__((noinline))
void conditional_vectorization(float* ALIGNED_32 out, const float* ALIGNED_32 in, int n) {
    /* Dead code path that still gets analyzed */
    if (0) {
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            out[i] = sinf(in[i]) * cosf(in[i]);  /* Vectorizable but dead code */
        }
    }
    
    /* Real path with CPU feature detection */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX intrinsic path - may trigger built-in vectorization */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(&in[i]);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&out[i], result);
        }
    } else 
    #endif
    {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            out[i] = sqrtf(in[i]);  /* Triggers builtin_vectorized_function */
        }
    }
}

/* Function with mixed data types and type punning */
__attribute__((noinline))
void mixed_type_vectorization(float* ALIGNED_32 fout, int* ALIGNED_32 iout, 
                              const float* ALIGNED_32 fin, int n) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Using __builtin_ilogb for integer result */
        fout[i] = sinf(fin[i]) * 2.0f;
        iout[i] = __builtin_ilogb(fin[i]);
    }
}

/* OpenMP parallel region with SIMD reduction */
__attribute__((noinline))
float omp_simd_reduction(const float* ALIGNED_32 data, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        sum += logf(fabsf(data[i]) + 1.0f);
    }
    
    return sum;
}

/* Switch statement with multiple vectorization candidates */
__attribute__((noinline))
void switch_vectorization(float* ALIGNED_32 out, const float* ALIGNED_32 in, 
                          int n, int mode) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = sinf(in[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = cosf(in[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = expf(in[i]);
            }
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = sqrtf(fabsf(in[i]));
            }
            break;
    }
}

/* Main test driver */
int main() {
    const int N = 1024;
    const int ITERATIONS = 100;
    
    /* Aligned arrays as required */
    float ALIGNED_32 data[N];
    float ALIGNED_32 result1[N];
    float ALIGNED_32 result2[N];
    double ALIGNED_32 ddata[N];
    double ALIGNED_32 dresult[N];
    int ALIGNED_32 iresult[N];
    char ALIGNED_64 src_buf[64 * ITERATIONS];
    char ALIGNED_64 dst_buf[64 * ITERATIONS];
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        data[i] = random_float() + 0.1f;  /* Avoid log(0) */
        ddata[i] = (double)data[i];
    }
    
    for (int i = 0; i < 64 * ITERATIONS; ++i) {
        src_buf[i] = (char)(rand() % 256);
    }
    
    float total_sum = 0.0f;
    
    /* Test 1: Hidden visibility math function */
    hidden_visibility_math(result1, data, N);
    for (int i = 0; i < N; i += 8) {
        total_sum += result1[i];
    }
    
    /* Test 2: Inline math with pow and exp */
    inline_math_pow(dresult, ddata, N);
    for (int i = 0; i < N; i += 8) {
        total_sum += (float)dresult[i];
    }
    
    /* Test 3: SIMD math function calls */
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        result2[i] = simd_math_function(data[i]);
    }
    
    /* Test 4: Builtin memcpy in loop */
    builtin_memcpy_loop(dst_buf, src_buf, 64, ITERATIONS);
    total_sum += dst_buf[0];  /* Use result */
    
    /* Test 5: Conditional vectorization */
    conditional_vectorization(result1, data, N);
    for (int i = 0; i < N; i += 8) {
        total_sum += result1[i];
    }
    
    /* Test 6: Mixed type vectorization */
    mixed_type_vectorization(result2, iresult, data, N);
    for (int i = 0; i < N; i += 8) {
        total_sum += result2[i] + iresult[i];
    }
    
    /* Test 7: OpenMP SIMD reduction */
    total_sum += omp_simd_reduction(data, N);
    
    /* Test 8: Switch-based vectorization (test all paths) */
    for (int mode = 0; mode < 4; ++mode) {
        switch_vectorization(result1, data, N, mode);
        total_sum += result1[mode];
    }
    
    /* Test 9: strlen in vectorizable context */
    const char* test_strings[] = {"hello", "world", "test", "vector"};
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < 4; ++i) {
        total_len += __builtin_strlen(test_strings[i]);
    }
    total_sum += total_len;
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    printf("String length sum: %d\n", total_len);
    
    return 0;
}
