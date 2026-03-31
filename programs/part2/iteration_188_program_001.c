/* 
 * Test program designed to trigger GCC's default_builtin_vectorized_function
 * and specifically execute the flag-setting block in targhooks.cc lines 981-990.
 * Compile with: g++ -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fno-builtin -o test_vectorization test_vectorization.cc
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
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
static float simple_rand(int seed) {
    static unsigned int state = 123456789;
    state = (state * 1103515245 + 12345) & 0x7fffffff;
    return (float)(state % 1000) / 1000.0f;
}

/* Function with hidden visibility to align with DECL_VISIBILITY setting */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float* ALIGN_32 out, const float* ALIGN_32 in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Multiple built-in calls to trigger vectorization */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with used attribute - may interact with declaration handling */
__attribute__((used, nothrow))
static void static_used_function(double* ALIGN_32 out, const double* ALIGN_32 in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* exp and log calls - GCC may vectorize these */
        out[i] = exp(in[i]) * log(fabs(in[i]) + 1.0);
    }
}

/* Function with OpenMP declare simd to create SIMD variants */
#pragma omp declare simd
__attribute__((always_inline))
static inline float simd_math_function(float x) {
    return powf(x, 1.5f) + expf(x * 0.5f);
}

/* Multiple small inline functions with different built-ins */
__attribute__((always_inline))
static inline float builtin_sqrt_wrapper(float x) {
    return __builtin_sqrtf(x);
}

__attribute__((always_inline))
static inline float builtin_sin_wrapper(float x) {
    return __builtin_sinf(x);
}

__attribute__((always_inline))
static inline float builtin_cos_wrapper(float x) {
    return __builtin_cosf(x);
}

/* Complex control flow with multiple vectorization candidates */
__attribute__((noinline))
static float conditional_vectorization(int func_id, float* ALIGN_32 data, int n) {
    float sum = 0.0f;
    
    switch(func_id) {
        case 0:
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; ++i) {
                sum += builtin_sqrt_wrapper(data[i]);
            }
            break;
            
        case 1:
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; ++i) {
                sum += builtin_sin_wrapper(data[i]) + builtin_cos_wrapper(data[i]);
            }
            break;
            
        case 2:
            #pragma omp simd reduction(+:sum)
            for (int i = 0; i < n; ++i) {
                sum += simd_math_function(data[i]);
            }
            break;
            
        default:
            /* Dead code path that still contains vectorizable built-ins */
            if (0) {  /* Always false, but compiler still parses */
                #pragma omp simd
                for (int i = 0; i < n; ++i) {
                    sum += __builtin_expf(data[i]);
                }
            }
            break;
    }
    
    return sum;
}

/* Memory operations using __builtin_memcpy in vectorizable context */
__attribute__((noinline))
static void vectorized_memcpy_operations(float* ALIGN_32 dst, 
                                         const float* ALIGN_32 src1, 
                                         const float* ALIGN_32 src2, 
                                         int n) {
    float temp[256] ALIGN_32;
    
    #pragma omp simd
    for (int i = 0; i < n; i += 8) {
        /* Use __builtin_memcpy with small fixed size - may trigger vectorization */
        __builtin_memcpy(&temp[i], &src1[i], 8 * sizeof(float));
        
        /* Additional operation to prevent elimination */
        for (int j = 0; j < 8 && (i + j) < n; ++j) {
            dst[i + j] = temp[i + j] + src2[i + j];
        }
    }
}

/* Architecture-specific intrinsics with fallback */
__attribute__((target_clones("avx2", "default")))
static void architecture_specific_vectorization(float* ALIGN_32 out, 
                                                const float* ALIGN_32 in, 
                                                int n) {
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(&in[i]);
            __m256 result = _mm256_add_ps(vec, _mm256_set1_ps(1.0f));
            _mm256_store_ps(&out[i], result);
        }
    } else 
#endif
    {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            /* sqrtf call that should be vectorized */
            out[i] = sqrtf(fabsf(in[i])) + 1.0f;
        }
    }
}

/* Mixed data types and type-punning */
static void mixed_type_vectorization(float* ALIGN_32 fout, 
                                     double* ALIGN_32 dout, 
                                     const int* ALIGN_32 iin, 
                                     int n) {
    /* Type-punning union for potential built-in vectorization */
    union {
        float f[4];
        int i[4];
    } converter ALIGN_32;
    
    #pragma omp parallel for simd
    for (int i = 0; i < n; ++i) {
        /* Mixed type operations */
        fout[i] = __builtin_sqrtf((float)iin[i]);
        dout[i] = __builtin_log((double)iin[i] + 1.0);
        
        /* Use __builtin_ilogb */
        int exp = __builtin_ilogb(fout[i] + 1.0f);
        converter.i[0] = exp;
        
        /* Ensure converter is used */
        fout[i] += converter.f[0] * 0.001f;
    }
}

/* OpenMP parallel region with nested SIMD */
static float openmp_nested_simd(const float* ALIGN_32 data, int n) {
    float total_sum = 0.0f;
    
    #pragma omp parallel for reduction(+:total_sum)
    for (int i = 0; i < n; i += 64) {
        float local_sum = 0.0f;
        int end = (i + 64 < n) ? i + 64 : n;
        
        #pragma omp simd reduction(+:local_sum)
        for (int j = i; j < end; ++j) {
            /* Multiple built-in calls in nested SIMD context */
            local_sum += sinf(data[j]) * cosf(data[j]) + sqrtf(data[j] + 1.0f);
        }
        
        total_sum += local_sum;
    }
    
    return total_sum;
}

/* Main test driver */
int main() {
    const int N = 1024;
    const int ITERATIONS = 100;
    
    /* Aligned arrays as required */
    float float_data[N] ALIGN_32;
    float float_out[N] ALIGN_32;
    float float_temp[N] ALIGN_32;
    double double_data[N] ALIGN_32;
    double double_out[N] ALIGN_32;
    int int_data[N] ALIGN_32;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        float_data[i] = simple_rand(i) * 10.0f;
        double_data[i] = (double)float_data[i];
        int_data[i] = (int)(float_data[i] * 100);
    }
    
    float total_result = 0.0f;
    
    /* Test 1: Hidden visibility math function */
    hidden_visibility_math(float_out, float_data, N);
    for (int i = 0; i < N; ++i) {
        total_result += float_out[i];
    }
    
    /* Test 2: Static used function with double precision */
    static_used_function(double_out, double_data, N);
    for (int i = 0; i < N; ++i) {
        total_result += (float)double_out[i];
    }
    
    /* Test 3: Conditional vectorization with multiple paths */
    for (int iter = 0; iter < 3; ++iter) {
        total_result += conditional_vectorization(iter, float_data, N);
    }
    
    /* Test 4: Memory operations */
    vectorized_memcpy_operations(float_temp, float_data, float_out, N);
    for (int i = 0; i < N; ++i) {
        total_result += float_temp[i];
    }
    
    /* Test 5: Architecture-specific with fallback */
    architecture_specific_vectorization(float_out, float_data, N);
    for (int i = 0; i < N; ++i) {
        total_result += float_out[i];
    }
    
    /* Test 6: Mixed data types */
    mixed_type_vectorization(float_out, double_out, int_data, N);
    for (int i = 0; i < N; ++i) {
        total_result += float_out[i] + (float)double_out[i];
    }
    
    /* Test 7: OpenMP nested SIMD */
    total_result += openmp_nested_simd(float_data, N);
    
    /* Additional loop with strlen-like operation */
    char test_strings[4][64] = {
        "test_string_1_for_vectorization_consideration",
        "test_string_2_for_vectorization_consideration", 
        "test_string_3_for_vectorization_consideration",
        "test_string_4_for_vectorization_consideration"
    };
    
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < 4; ++i) {
        /* __builtin_strlen in SIMD context */
        total_len += __builtin_strlen(test_strings[i]);
    }
    total_result += (float)total_len;
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total_result);
    
    return 0;
}
