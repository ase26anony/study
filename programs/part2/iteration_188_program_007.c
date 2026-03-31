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

/* Alignment hints to engage vectorizer analysis */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Visibility attributes to intersect with DECL_VISIBILITY setting */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))

/* Simple random generator to prevent compile-time computation */
static inline float random_float() {
    static unsigned seed = 123456789;
    seed = seed * 1103515245 + 12345;
    return (float)(seed & 0xFFFF) / 65535.0f;
}

/* 
 * Function 1: Math-intensive with explicit SIMD pragma
 * Triggers vectorization of sinf/cosf builtins
 */
USED_FUNC NOTHROW_FUNC
static void math_intensive(float* ALIGN_32 in, float* ALIGN_32 out1, float* ALIGN_32 out2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        out1[i] = sinf(in[i]);
        out2[i] = cosf(in[i]);
    }
}

/* 
 * Function 2: Memory operations with __builtin_memcpy in vectorizable loop
 */
static inline void memory_ops(char* ALIGN_32 dst, const char* ALIGN_32 src, int size, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        /* This may trigger builtin vectorization for memcpy */
        __builtin_memcpy(dst + i*16, src + i*16, 16);
    }
}

/*
 * Function 3: Conditional path with architecture-specific intrinsics
 * Forces compiler to analyze both vector and scalar paths
 */
HIDDEN_VIS
void conditional_vector_path(float* ALIGN_32 data, int n, float* result) {
    float sum = 0.0f;
    
    if (__builtin_cpu_supports("avx2")) {
        /* Vector path - may trigger builtin alternatives */
        #ifdef __AVX2__
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(&data[i]);
            __m256 sqrt_vec = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&result[i], sqrt_vec);
        }
        #endif
    } else {
        /* Scalar fallback - should vectorize sqrtf */
        #pragma omp simd reduction(+:sum)
        for (int i = 0; i < n; ++i) {
            result[i] = sqrtf(data[i]);
            sum += result[i];
        }
    }
    
    /* Dead code path that still gets analyzed */
    if (0) {
        /* Contains more builtin calls for vectorizer to consider */
        double dead ALIGN_32[8];
        for (int i = 0; i < 8; ++i) {
            dead[i] = __builtin_sqrt(data[i]);
        }
    }
}

/*
 * Function 4: Hidden visibility helper with mixed math operations
 */
HIDDEN_VIS NOTHROW_FUNC
static void hidden_visibility_helper(double* ALIGN_32 in, double* ALIGN_32 out, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        out[i] = exp(in[i]) + log(fabs(in[i]) + 1.0);
    }
}

/*
 * Function 5: Always_inline function with pow operations
 */
static inline __attribute__((always_inline)) 
void power_loop(float* ALIGN_32 in, float* ALIGN_32 out, int n, float exponent) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        out[i] = powf(in[i], exponent);
    }
}

/*
 * Function 6: OpenMP declare simd function
 */
#pragma omp declare simd
float simd_math(float x, float y) {
    return sinf(x) * cosf(y) + sqrtf(x*y);
}

/*
 * Function 7: Type-punning with unions and builtins
 */
void type_punning_operations(int* ALIGN_32 int_data, float* ALIGN_32 float_data, int n) {
    union {
        int i;
        float f;
    } converter;
    
    for (int i = 0; i < n; ++i) {
        /* May trigger builtin vectorization for type conversion */
        converter.i = int_data[i];
        float_data[i] = __builtin_sqrtf(converter.f);
    }
}

/*
 * Function 8: Complex control flow with switch statement
 * Presents multiple vectorization candidates to the compiler
 */
static float dispatch_vector_ops(int mode, float* ALIGN_32 data, int n) {
    float result ALIGN_32[1024];
    
    switch (mode) {
        case 0: {
            /* sin/cos path */
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                result[i] = sinf(data[i]) + cosf(data[i]);
            }
            break;
        }
        case 1: {
            /* exp/log path */
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                result[i] = expf(data[i]) * logf(data[i] + 1.0f);
            }
            break;
        }
        case 2: {
            /* fabs/pow path */
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                result[i] = powf(fabs(data[i]), 1.5f);
            }
            break;
        }
        default: {
            /* mixed operations */
            #pragma omp simd
            for (int i = 0; i < n; ++i) {
                result[i] = sinf(data[i]) * cosf(data[i]) + sqrtf(data[i]);
            }
            break;
        }
    }
    
    /* Aggregate to prevent elimination */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        sum += result[i];
    }
    return sum;
}

/*
 * Main function: Orchestrates all test cases
 */
int main() {
    const int N = 1024;
    const int ITER = 10;
    
    /* Aligned arrays with different types */
    float float_data[N] ALIGN_32;
    float float_out1[N] ALIGN_32;
    float float_out2[N] ALIGN_32;
    double double_data[N] ALIGN_32;
    double double_out[N] ALIGN_32;
    char char_data[N*16] ALIGN_32;
    char char_copy[N*16] ALIGN_32;
    int int_data[N] ALIGN_32;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        float_data[i] = random_float() * 10.0f;
        double_data[i] = float_data[i];
        int_data[i] = rand();
    }
    for (int i = 0; i < N*16; ++i) {
        char_data[i] = rand() % 256;
    }
    
    float total_sum = 0.0f;
    
    /* Test 1: Math-intensive operations */
    for (int iter = 0; iter < ITER; ++iter) {
        math_intensive(float_data, float_out1, float_out2, N);
        for (int i = 0; i < N; ++i) {
            total_sum += float_out1[i] + float_out2[i];
        }
    }
    
    /* Test 2: Memory operations */
    memory_ops(char_copy, char_data, N*16, N);
    
    /* Test 3: Conditional vector path */
    conditional_vector_path(float_data, N, float_out1);
    for (int i = 0; i < N; ++i) {
        total_sum += float_out1[i];
    }
    
    /* Test 4: Hidden visibility helper */
    hidden_visibility_helper(double_data, double_out, N);
    for (int i = 0; i < N; ++i) {
        total_sum += (float)double_out[i];
    }
    
    /* Test 5: Power loop */
    power_loop(float_data, float_out2, N, 2.5f);
    for (int i = 0; i < N; ++i) {
        total_sum += float_out2[i];
    }
    
    /* Test 6: OpenMP declare simd */
    #pragma omp parallel for simd reduction(+:total_sum)
    for (int i = 0; i < N; ++i) {
        total_sum += simd_math(float_data[i], float_data[(i+1)%N]);
    }
    
    /* Test 7: Type punning */
    type_punning_operations(int_data, float_out1, N);
    for (int i = 0; i < N; ++i) {
        total_sum += float_out1[i];
    }
    
    /* Test 8: Complex control flow */
    for (int mode = 0; mode < 4; ++mode) {
        total_sum += dispatch_vector_ops(mode, float_data, N);
    }
    
    /* Additional test: strlen in vectorizable context */
    const char* test_strings[] = {"vector", "builtin", "function", "test"};
    int len_sum = 0;
    #pragma omp simd reduction(+:len_sum)
    for (int i = 0; i < 4; ++i) {
        len_sum += __builtin_strlen(test_strings[i]);
    }
    total_sum += (float)len_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    
    return 0;
}
