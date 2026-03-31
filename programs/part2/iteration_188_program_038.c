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
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Simple random generator to prevent compile-time computation */
static inline float random_float() {
    static unsigned seed = 12345;
    seed = seed * 1103515245 + 12345;
    return (float)(seed & 0xFFFF) / 65536.0f;
}

/* Function with hidden visibility - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float* ALIGN_32 out, const float* ALIGN_32 in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Multiple built-in calls to trigger vectorization */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with vectorized built-ins */
static void static_vectorized_memcpy(char* ALIGN_32 dst, const char* ALIGN_32 src, int size) {
    const int chunk = 64;
    #pragma omp simd
    for (int i = 0; i < size; i += chunk) {
        int len = (size - i) > chunk ? chunk : (size - i);
        /* Use __builtin_memcpy in vectorizable loop */
        __builtin_memcpy(dst + i, src + i, len);
    }
}

/* Function with __builtin_cpu_supports check for architecture-specific paths */
__attribute__((always_inline))
inline void architecture_specific_math(double* ALIGN_32 out, const double* ALIGN_32 in, int n) {
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - compiler may still analyze scalar fallback */
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(in + i);
            __m256d result = _mm256_sqrt_pd(vec);
            _mm256_store_pd(out + i, result);
        }
    } else
    #endif
    {
        /* Scalar fallback with built-in calls - vectorization candidate */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            out[i] = sqrt(in[i]) + log(in[i] + 1.0);
        }
    }
}

/* OpenMP declare simd to create SIMD variants */
#pragma omp declare simd
__attribute__((used))
float simd_math_function(float x) {
    return powf(x, 1.5f) + expf(x * 0.5f);
}

/* Function with mixed data types and type punning */
__attribute__((noinline))
void mixed_type_vectorization(float* ALIGN_32 farr, int* ALIGN_32 iarr, int n) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int j = 0; j < n; ++j) {
        /* Use __builtin_ilogb for integer result from float */
        iarr[j] = __builtin_ilogb(farr[j]);
        farr[j] = __builtin_sqrtf(farr[j]);
    }
}

/* Dead code path that still contains vectorizable built-in calls */
__attribute__((used))
static void dead_code_path(double* ALIGN_32 arr, int n) {
    if (0) {  /* Dead code, but front-end still processes declarations */
        #pragma GCC ivdep
        for (int i = 0; i < n; ++i) {
            arr[i] = sin(arr[i]) * cos(arr[i]) + exp(arr[i]);
        }
    }
}

/* Complex OpenMP nesting with reduction */
void openmp_nested_math(float* ALIGN_32 data, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; ++j) {
            float val = data[i * 16 + j];
            /* Multiple built-in calls in nested loop */
            sum += sinf(val) + cosf(val) + sqrtf(fabsf(val));
            data[i * 16 + j] = val * 0.5f;
        }
    }
    
    /* Prevent dead code elimination */
    data[0] = sum / (n * 16);
}

/* Switch statement with multiple vectorization candidates */
__attribute__((flatten))
void switch_vectorization(int mode, float* ALIGN_32 out, const float* ALIGN_32 in, int n) {
    switch (mode & 3) {
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
                out[i] = sqrtf(fabsf(in[i]));
            }
            break;
            
        case 3:
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                out[i] = logf(in[i] + 1.0f);
            }
            break;
    }
}

/* Main test driver */
int main() {
    const int N = 1024;
    const int M = 512;
    
    /* Aligned arrays with different types */
    float float_array1[N] ALIGN_32;
    float float_array2[N] ALIGN_32;
    double double_array[M] ALIGN_64;
    int int_array[N] ALIGN_32;
    char char_buffer1[N] ALIGN_32;
    char char_buffer2[N] ALIGN_32;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        float_array1[i] = random_float() * 10.0f;
        int_array[i] = rand() % 100;
        char_buffer1[i] = 'A' + (rand() % 26);
    }
    
    for (int i = 0; i < M; ++i) {
        double_array[i] = random_float() * 5.0 + 0.1;
    }
    
    /* Test 1: Math-intensive function with OpenMP SIMD */
    hidden_visibility_math(float_array2, float_array1, N);
    
    /* Test 2: Memory/copy function with __builtin_memcpy */
    static_vectorized_memcpy(char_buffer2, char_buffer1, N);
    
    /* Test 3: Architecture-specific paths */
    architecture_specific_math((double*)float_array2, (double*)float_array1, N/2);
    
    /* Test 4: Mixed data types */
    mixed_type_vectorization(float_array1, int_array, N);
    
    /* Test 5: OpenMP nested math */
    openmp_nested_math(float_array1, N/16);
    
    /* Test 6: Switch-based vectorization */
    for (int mode = 0; mode < 4; ++mode) {
        switch_vectorization(mode, float_array2, float_array1, N);
    }
    
    /* Test 7: SIMD function variants */
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        float_array2[i] = simd_math_function(float_array1[i]);
    }
    
    /* Test 8: strlen in vectorizable context */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < N/64; ++i) {
        total_len += __builtin_strlen(char_buffer1 + i * 64);
    }
    
    /* Aggregate results to prevent elimination */
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < N; ++i) {
        checksum += float_array1[i] + float_array2[i];
    }
    
    printf("Result checksum: %f\n", checksum);
    printf("Total string length: %d\n", total_len);
    
    return 0;
}
