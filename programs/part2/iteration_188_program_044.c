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

/* OpenMP for SIMD directives */
#include <omp.h>

/* Alignment attributes for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Simple random generator to prevent compile-time computation */
static inline float rand_float(void) {
    return (float)rand() / (float)RAND_MAX * 10.0f - 5.0f;
}

/* Function with hidden visibility - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls in vectorized loop */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with vectorized built-ins */
static void static_vectorized_func(double *out, const double *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Using exp and log built-ins */
        out[i] = exp(in[i]) * log(fabs(in[i]) + 1.0);
    }
}

/* Function with __builtin_memcpy in loop */
__attribute__((used))
void builtin_memcpy_test(char *dest, const char *src, int n) {
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; i += 32) {
        /* Vectorized memory copy using builtin */
        __builtin_memcpy(dest + i, src + i, 32);
    }
}

/* Function using __builtin_strlen in vectorizable context */
__attribute__((always_inline))
static inline int vectorized_strlen_test(const char **strings, int count) {
    int total = 0;
    #pragma GCC ivdep
    for (int i = 0; i < count; i++) {
        /* strlen may get vectorized for multiple strings */
        total += __builtin_strlen(strings[i]);
    }
    return total;
}

/* SIMD function declaration with OpenMP declare simd */
#pragma omp declare simd
float simd_math_function(float x) {
    return sinf(x) * cosf(x) + sqrtf(x * x + 1.0f);
}

/* Function with architecture-specific intrinsics */
#ifdef __x86_64__
__attribute__((target("avx2")))
void avx2_vector_math(float *out, const float *a, const float *b, int n) {
    for (int i = 0; i < n; i += 8) {
        __m256 va = _mm256_load_ps(a + i);
        __m256 vb = _mm256_load_ps(b + i);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_store_ps(out + i, vc);
    }
}
#endif

/* Multiple small inline functions with different built-ins */
__attribute__((always_inline))
static inline void pow_loop(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.5f);
    }
}

__attribute__((always_inline))
static inline void exp_loop(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i] * 0.5f);
    }
}

__attribute__((always_inline))
static inline void fabs_loop(float *out, const float *in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = fabsf(in[i]) + 1.0f;
    }
}

/* Conditional function selection with dead code path */
void conditional_vectorization(int mode, float *out, const float *in, int n) {
    switch (mode) {
        case 0:
            pow_loop(out, in, n);
            break;
        case 1:
            exp_loop(out, in, n);
            break;
        case 2:
            fabs_loop(out, in, n);
            break;
        default:
            /* Dead code path but still processed by front-end */
            if (0) {
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    out[i] = sinf(in[i]) / cosf(in[i]);
                }
            }
            break;
    }
}

/* Type-punning with union for vector/scalar conversion */
typedef union {
    __m128 v;
    float f[4];
} VectorFloat;

/* Mixed data types and alignment */
void mixed_type_vectorization(float *fout, double *dout, 
                              const float *fin, const double *din, int n) {
    /* Float operations */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        fout[i] = sinf(fin[i]) + cosf(fin[i]);
    }
    
    /* Double operations */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dout[i] = sqrt(din[i]) * log(din[i] + 1.0);
    }
}

/* Complex OpenMP context with reduction */
float omp_reduction_test(const float *data, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; j++) {
            /* Nested vectorization with built-in */
            sum += sinf(data[i] + j * 0.1f);
        }
    }
    
    return sum;
}

/* CPU feature detection for architecture-specific paths */
void cpu_dependent_vectorization(float *out, const float *in, int n) {
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path */
        for (int i = 0; i < n; i += 8) {
            __m256 v = _mm256_load_ps(in + i);
            __m256 result = _mm256_sqrt_ps(v);
            _mm256_store_ps(out + i, result);
        }
    } else 
    #endif
    {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sqrtf(in[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    const int N = 1024;
    const int N_ALIGNED = 1024;
    
    /* Aligned arrays as per requirements */
    float ALIGN_32 fdata[N], fresult[N];
    double ALIGN_32 ddata[N], dresult[N];
    char ALIGN_32 src_buf[N * 4], dest_buf[N * 4];
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        fdata[i] = rand_float();
        ddata[i] = (double)rand_float();
        src_buf[i] = 'A' + (rand() % 26);
    }
    
    /* Test 1: Math-intensive function with OpenMP SIMD */
    hidden_visibility_math(fresult, fdata, N);
    
    /* Test 2: Static function with vectorized built-ins */
    static_vectorized_func(dresult, ddata, N);
    
    /* Test 3: Builtin memcpy in vectorized loop */
    builtin_memcpy_test(dest_buf, src_buf, N);
    
    /* Test 4: String operations */
    const char *strings[] = {"test1", "test22", "test333", "test4444"};
    int str_total = vectorized_strlen_test(strings, 4);
    
    /* Test 5: SIMD declared function */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        fresult[i] += simd_math_function(fdata[i]);
    }
    
    /* Test 6: Architecture-specific path */
    cpu_dependent_vectorization(fresult, fdata, N);
    
    /* Test 7: Conditional vectorization */
    for (int mode = 0; mode < 3; mode++) {
        conditional_vectorization(mode, fresult, fdata, N);
    }
    
    /* Test 8: Mixed data types */
    mixed_type_vectorization(fresult, dresult, fdata, ddata, N);
    
    /* Test 9: OpenMP reduction */
    float reduction_sum = omp_reduction_test(fdata, N/16);
    
    /* Test 10: Type-punning with vector types */
    VectorFloat vec;
    vec.v = _mm_set1_ps(1.0f);
    for (int i = 0; i < 4; i++) {
        fresult[i] += vec.f[i];
    }
    
    /* Prevent dead code elimination */
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += fresult[i] + (float)dresult[i];
    }
    
    printf("Results: checksum=%f, strlen_total=%d, reduction_sum=%f\n",
           checksum, str_total, reduction_sum);
    
    return 0;
}
