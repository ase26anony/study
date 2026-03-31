/* Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -ffast-math -fopt-info-vec-missed targhooks_test.c -lm -o targhooks_test */

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

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Random initialization to prevent compile-time computation */
static inline float random_float(float min, float max) {
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

/* Function with hidden visibility containing vectorizable built-ins */
__attribute__((visibility("hidden")))
__attribute__((used))
__attribute__((nothrow))
static void hidden_visibility_math(float* ALIGN_32 in, float* ALIGN_32 out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls that should be vectorized */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with always_inline attribute */
static inline __attribute__((always_inline)) 
void inline_math_operations(double* ALIGN_32 in, double* ALIGN_32 out, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* Built-in math functions that GCC should vectorize */
        out[i] = exp(in[i]) * log(fabs(in[i]) + 1.0);
    }
}

/* Function using __builtin_memcpy in vectorizable context */
__attribute__((used))
void vectorized_memcpy_ops(char* ALIGN_32 dst, char* ALIGN_32 src, int size, int iterations) {
    #pragma omp parallel for simd
    for (int i = 0; i < iterations; i++) {
        /* __builtin_memcpy in loop - GCC may vectorize this */
        __builtin_memcpy(dst + i * 64, src + i * 64, 64);
    }
}

/* OpenMP declare simd function with built-ins */
#pragma omp declare simd uniform(n) linear(i)
__attribute__((nothrow))
float simd_math_function(float x, int n) {
    float result = x;
    for (int i = 0; i < n; i++) {
        result = sinf(result) + cosf(result);
    }
    return result;
}

/* Architecture-specific path with fallback */
void architecture_specific_math(float* ALIGN_32 in, float* ALIGN_32 out, int n) {
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - compiler may still consider built-in vectorization */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(&in[i]);
            __m256 result = _mm256_add_ps(_mm256_sin_ps(vec), _mm256_cos_ps(vec));
            _mm256_store_ps(&out[i], result);
        }
    } else 
#endif
    {
        /* Fallback scalar path with built-in calls - should trigger vectorization */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(in[i]);
        }
    }
}

/* Multiple built-in types in switch statement */
__attribute__((flatten))
void multi_builtin_switch(float* ALIGN_32 in, float* ALIGN_32 out, int n, int mode) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sinf(in[i]) * cosf(in[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sqrtf(fabsf(in[i])) + logf(fabsf(in[i]) + 1.0f);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = expf(in[i] * 0.5f) - 1.0f;
            }
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = powf(in[i], 1.5f);
            }
    }
}

/* Dead code path with vectorizable built-ins (still processed by front-end) */
static void dead_code_path(float* ALIGN_32 in, float* ALIGN_32 out, int n) {
    if (0) {  /* Dead code, but still parsed */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            /* These built-in calls should still be considered for vectorization */
            out[i] = sinf(in[i]) / cosf(in[i]);  /* tanf alternative */
        }
    }
}

/* Type-punning with union and memcpy */
void type_punning_operations(float* ALIGN_32 in, int* ALIGN_32 out, int n) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Use __builtin_memcpy for type punning */
        __builtin_memcpy(&converter.f, &in[i], sizeof(float));
        out[i] = converter.i & 0x7FFFFFFF;  /* Clear sign bit */
    }
}

/* Complex reduction with built-ins */
float complex_reduction(float* ALIGN_32 data, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; j++) {
            /* Nested loops with built-in calls */
            sum += sinf(data[i] + j * 0.1f) * cosf(data[i] - j * 0.1f);
        }
    }
    
    return sum;
}

/* Main test function */
int main() {
    const int N = 1024;
    const int ITERATIONS = 100;
    
    /* Aligned arrays for vectorization */
    float* ALIGN_32 in_array = (float*)aligned_alloc(32, N * sizeof(float));
    float* ALIGN_32 out_array = (float*)aligned_alloc(32, N * sizeof(float));
    float* ALIGN_32 temp_array = (float*)aligned_alloc(32, N * sizeof(float));
    int* ALIGN_32 int_array = (int*)aligned_alloc(32, N * sizeof(int));
    
    char* ALIGN_32 src_buf = (char*)aligned_alloc(32, N * 64);
    char* ALIGN_32 dst_buf = (char*)aligned_alloc(32, N * 64);
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        in_array[i] = random_float(-3.14f, 3.14f);
        out_array[i] = 0.0f;
        temp_array[i] = random_float(0.1f, 10.0f);
    }
    
    memset(src_buf, 'A', N * 64);
    memset(dst_buf, 0, N * 64);
    
    float total_sum = 0.0f;
    
    /* Test 1: Hidden visibility function */
    hidden_visibility_math(in_array, out_array, N);
    for (int i = 0; i < N; i++) total_sum += out_array[i];
    
    /* Test 2: Inline math operations */
    inline_math_operations((double*)temp_array, (double*)out_array, N/2);
    for (int i = 0; i < N/2; i++) total_sum += out_array[i];
    
    /* Test 3: Vectorized memcpy operations */
    vectorized_memcpy_ops(dst_buf, src_buf, 64, N);
    total_sum += dst_buf[0] + dst_buf[N*64-1];
    
    /* Test 4: Architecture-specific path */
    architecture_specific_math(in_array, out_array, N);
    for (int i = 0; i < N; i++) total_sum += out_array[i];
    
    /* Test 5: Multiple switch cases */
    for (int mode = 0; mode < 4; mode++) {
        multi_builtin_switch(in_array, out_array, N, mode);
        for (int i = 0; i < N; i++) total_sum += out_array[i];
    }
    
    /* Test 6: Dead code path (still processed) */
    dead_code_path(in_array, out_array, N);
    
    /* Test 7: Type punning operations */
    type_punning_operations(in_array, int_array, N);
    for (int i = 0; i < N; i++) total_sum += int_array[i];
    
    /* Test 8: Complex reduction */
    total_sum += complex_reduction(in_array, N/4);
    
    /* Test 9: OpenMP declare simd function */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        out_array[i] = simd_math_function(in_array[i], 2);
        total_sum += out_array[i];
    }
    
    /* Test 10: Mixed built-ins with strlen-like operation */
    char test_str[] = "This is a test string for vectorization analysis.";
    int len_sum = 0;
    #pragma omp simd reduction(+:len_sum)
    for (int i = 0; i < ITERATIONS; i++) {
        /* strlen as built-in - may be vectorized */
        len_sum += __builtin_strlen(test_str) + i;
    }
    total_sum += len_sum;
    
    printf("Total sum: %f\n", total_sum);
    printf("(This value varies due to random initialization)\n");
    
    /* Cleanup */
    free(in_array);
    free(out_array);
    free(temp_array);
    free(int_array);
    free(src_buf);
    free(dst_buf);
    
    return 0;
}
