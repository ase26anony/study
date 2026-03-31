/* 
 * This program is designed to trigger GCC's default_builtin_vectorized_function
 * hook, specifically the flag-setting block for compiler-generated built-in
 * function declarations during vectorization analysis.
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fno-builtin -o vectorized_builtins vectorized_builtins.c -lm
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

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Visibility attributes to interact with DECL_VISIBILITY setting */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))

/* Simple random generator to prevent compile-time computation */
static inline float random_float(float min, float max) {
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

/* ========== Function 1: Math-intensive with OpenMP SIMD ========== */
/* This should trigger vectorization of sinf/cosf builtins */
static void math_intensive_vectorized(float* ALIGN_32 in, float* ALIGN_32 out_sin, float* ALIGN_32 out_cos, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out_sin[i] = sinf(in[i]);  /* Vectorized sinf */
        out_cos[i] = cosf(in[i]);  /* Vectorized cosf */
    }
}

/* ========== Function 2: Memory operations with builtins ========== */
/* Uses __builtin_memcpy in vectorizable context */
static void memory_operations_vectorized(char* ALIGN_32 src, char* ALIGN_32 dst, int n, int chunk) {
    #pragma omp simd
    for (int i = 0; i < n; i += chunk) {
        __builtin_memcpy(dst + i, src + i, chunk);  /* Vectorized memcpy */
    }
}

/* ========== Function 3: Conditional architecture-specific paths ========== */
/* Mix of intrinsics and scalar builtins to trigger both paths */
USED_FUNC NOTHROW_FUNC static void conditional_vectorization(float* ALIGN_32 in, float* ALIGN_32 out, int n) {
    /* Dead code path that still gets analyzed */
    if (0) {
        #pragma GCC ivdep
        for (int i = 0; i < n; i++) {
            out[i] = sqrtf(in[i]) + powf(in[i], 2.0f);  /* Vectorization candidates */
        }
    }
    
    /* Real path with CPU dispatch */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX intrinsic path - compiler may still consider builtin vectorization */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(&in[i]);
            __m256 result = _mm256_sqrt_ps(vec);  /* Intrinsic, but may trigger builtin analysis */
            _mm256_store_ps(&out[i], result);
        }
    } else 
    #endif
    {
        /* Scalar fallback with builtins - explicit vectorization requested */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sqrtf(in[i]);  /* Vectorized sqrtf */
        }
    }
}

/* ========== Function 4: Hidden visibility helper ========== */
/* Marked hidden to align with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
HIDDEN_VIS static void hidden_visibility_helper(double* ALIGN_32 in, double* ALIGN_32 out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]) + log(fabs(in[i]) + 1.0);  /* Vectorized exp/log/fabs */
    }
}

/* ========== Function 5: Multiple small vectorizable functions ========== */
/* Several inline functions with different builtins */
static inline ALWAYS_INLINE void vector_pow(float* a, float* b, float p, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        a[i] = powf(b[i], p);  /* Vectorized powf */
    }
}

static inline ALWAYS_INLINE void vector_exp(float* a, float* b, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        a[i] = expf(b[i]);  /* Vectorized expf */
    }
}

static inline ALWAYS_INLINE void vector_fabs(float* a, float* b, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        a[i] = fabsf(b[i]);  /* Vectorized fabsf */
    }
}

/* Function that selects between vectorizable helpers */
static void multi_builtin_selector(int mode, float* ALIGN_32 a, float* ALIGN_32 b, int n) {
    switch (mode) {
        case 1:
            vector_pow(a, b, 2.0f, n);
            break;
        case 2:
            vector_exp(a, b, n);
            break;
        case 3:
            vector_fabs(a, b, n);
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                a[i] = sinf(b[i]) * cosf(b[i]);  /* Mixed builtins */
            }
    }
}

/* ========== Function 6: OpenMP SIMD with reduction ========== */
/* Complex vectorization context */
#pragma omp declare simd
static float reduction_vectorized(float* ALIGN_32 data, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += sqrtf(data[i]) * logf(data[i] + 1.0f);  /* Multiple vectorized builtins */
    }
    return sum;
}

/* ========== Function 7: Type-punning and mixed types ========== */
/* May trigger builtin vectorization for copy operations */
static void type_punning_vectorized(float* ALIGN_32 src, int* ALIGN_32 dst, int n) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Type punning through builtin bit operations */
        converter.f = src[i];
        dst[i] = converter.i;
        
        /* Also use __builtin_ilogb for integer result */
        dst[i] += __builtin_ilogb(src[i] + 1.0f);
    }
}

/* ========== Main function ========== */
int main() {
    const int N = 1024;
    const int ITERATIONS = 100;
    
    /* Aligned arrays for vectorization */
    float* ALIGN_32 array_f1 = (float*)aligned_alloc(32, N * sizeof(float));
    float* ALIGN_32 array_f2 = (float*)aligned_alloc(32, N * sizeof(float));
    float* ALIGN_32 array_f3 = (float*)aligned_alloc(32, N * sizeof(float));
    double* ALIGN_32 array_d1 = (double*)aligned_alloc(32, N * sizeof(double));
    double* ALIGN_32 array_d2 = (double*)aligned_alloc(32, N * sizeof(double));
    char* ALIGN_32 array_c1 = (char*)aligned_alloc(32, N * sizeof(char));
    char* ALIGN_32 array_c2 = (char*)aligned_alloc(32, N * sizeof(char));
    int* ALIGN_32 array_i1 = (int*)aligned_alloc(32, N * sizeof(int));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        array_f1[i] = random_float(0.1f, 3.14f);
        array_d1[i] = (double)array_f1[i];
        array_c1[i] = (char)(i % 256);
    }
    
    float total_sum = 0.0f;
    
    /* Execute all vectorization patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* 1. Math-intensive vectorization */
        math_intensive_vectorized(array_f1, array_f2, array_f3, N);
        
        /* 2. Memory operations */
        memory_operations_vectorized(array_c1, array_c2, N, 16);
        
        /* 3. Conditional vectorization */
        conditional_vectorization(array_f1, array_f2, N);
        
        /* 4. Hidden visibility helper */
        hidden_visibility_helper(array_d1, array_d2, N);
        
        /* 5. Multiple builtin selector */
        for (int mode = 0; mode < 4; mode++) {
            multi_builtin_selector(mode, array_f2, array_f1, N);
        }
        
        /* 6. OpenMP reduction */
        total_sum += reduction_vectorized(array_f1, N);
        
        /* 7. Type punning */
        type_punning_vectorized(array_f1, array_i1, N);
        
        /* Modify input slightly for next iteration */
        for (int i = 0; i < N; i++) {
            array_f1[i] += 0.01f;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Vectorization test complete.\n");
    printf("Total sum: %f\n", total_sum);
    printf("Sample values: %f, %f, %d\n", array_f2[0], array_d2[0], array_i1[0]);
    
    /* Cleanup */
    free(array_f1);
    free(array_f2);
    free(array_f3);
    free(array_d1);
    free(array_d2);
    free(array_c1);
    free(array_c2);
    free(array_i1);
    
    return 0;
}
