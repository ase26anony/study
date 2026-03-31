/* 
 * Test program designed to trigger GCC's default_builtin_vectorized_function
 * and specifically execute the flag-setting block in targhooks.cc lines 981-990.
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -ffast-math -fopt-info-vec-all
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

#define SIZE 1024
#define ALIGN 32

/* Alignment hints to engage vectorizer */
typedef float f32_array[SIZE] __attribute__((aligned(ALIGN)));
typedef double f64_array[SIZE] __attribute__((aligned(ALIGN)));
typedef int i32_array[SIZE] __attribute__((aligned(ALIGN)));

/* Helper to prevent compile-time computation */
static float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7FFFFFFF) / 2147483647.0f;
}

/* Function with hidden visibility - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(f64_array out, const f64_array in1, const f64_array in2) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Multiple built-in calls to increase vectorization candidates */
        out[i] = exp(in1[i]) + log(fabs(in2[i]) + 1.0);
    }
}

/* Static function with vectorized built-ins */
__attribute__((always_inline))
static inline void vectorized_sincos(f32_array sin_out, f32_array cos_out, const f32_array angles) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        sin_out[i] = sinf(angles[i]);
        cos_out[i] = cosf(angles[i]);
    }
}

/* Function with OpenMP SIMD reduction */
__attribute__((used))
static float omp_simd_reduction(const f32_array data) {
    float sum = 0.0f;
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        sum += sqrtf(fabs(data[i])) + powf(data[i], 1.5f);
    }
    return sum;
}

/* Memory operations with builtins in vectorizable context */
__attribute__((nothrow))
static void builtin_memcpy_loop(f32_array dest, const f32_array src1, const f32_array src2) {
    for (int i = 0; i < SIZE; i += 8) {
        /* Use __builtin_memcpy with small size - may trigger vectorization */
        __builtin_memcpy(&dest[i], &src1[i], 8 * sizeof(float));
        
        /* Mixed with math builtins */
        #pragma GCC ivdep
        for (int j = i; j < i + 8 && j < SIZE; j++) {
            dest[j] += __builtin_sqrtf(src2[j]);
        }
    }
}

/* Conditional path with CPU feature detection */
__attribute__((used))
static void conditional_vectorization(f32_array out, const f32_array in) {
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX intrinsic path - compiler may still consider builtin vectorization */
        for (int i = 0; i < SIZE; i += 8) {
            __m256 vec = _mm256_load_ps(&in[i]);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&out[i], result);
        }
    } else 
#endif
    {
        /* Scalar fallback with builtin calls - vectorization candidate */
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            out[i] = __builtin_sqrtf(in[i]);
        }
    }
}

/* Dead code path that still gets analyzed */
static void dead_code_path(f32_array out, const f32_array in) {
    if (0) {  /* Dead code, but still parsed */
        #pragma GCC unroll 4
        for (int i = 0; i < SIZE; i++) {
            /* Multiple builtins in dead code */
            out[i] = __builtin_expf(in[i]) + __builtin_logf(in[i] + 1.0f);
        }
    }
}

/* Function with switch statement containing different vectorization candidates */
__attribute__((always_inline))
static inline float builtin_selector(int op, float x, float y) {
    switch (op & 3) {
        case 0: return __builtin_powf(x, y);
        case 1: return __builtin_expf(x);
        case 2: return __builtin_logf(fabsf(x) + 1.0f);
        case 3: return __builtin_sinpif(x);  /* sin(pi * x) */
        default: return 0.0f;
    }
}

/* Mixed data types and type punning */
__attribute__((used))
static void mixed_type_vectorization(f32_array fout, i32_array iout, 
                                    const f32_array fin, const i32_array iin) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Type punning via union */
        converter.f = fin[i];
        iout[i] = converter.i + iin[i];
        
        /* Builtin with integer argument */
        fout[i] = __builtin_sqrtf(fin[i]) + __builtin_ilogbf(fin[i]);
    }
}

/* Main test driver */
int main() {
    /* Initialize aligned arrays with pattern data */
    f32_array angles, sin_result, cos_result, data1, data2, copy_buf;
    f64_array dbl_in1, dbl_in2, dbl_out;
    i32_array int_in, int_out;
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        angles[i] = simple_rand(i) * 3.14159265f;
        data1[i] = simple_rand(i + SIZE) * 10.0f;
        data2[i] = simple_rand(i + 2*SIZE) * 5.0f;
        dbl_in1[i] = (double)simple_rand(i + 3*SIZE) * 10.0;
        dbl_in2[i] = (double)simple_rand(i + 4*SIZE) * 5.0;
        int_in[i] = (int)(simple_rand(i + 5*SIZE) * 1000.0f);
    }
    
    printf("Starting vectorization tests...\n");
    
    /* Test 1: Basic math function vectorization */
    vectorized_sincos(sin_result, cos_result, angles);
    
    /* Test 2: OpenMP SIMD reduction */
    float sum = omp_simd_reduction(data1);
    printf("Reduction sum: %f\n", sum);
    
    /* Test 3: Memory builtins in loop */
    builtin_memcpy_loop(copy_buf, sin_result, cos_result);
    
    /* Test 4: Conditional vectorization */
    conditional_vectorization(data2, data1);
    
    /* Test 5: Hidden visibility function */
    hidden_visibility_math(dbl_out, dbl_in1, dbl_in2);
    
    /* Test 6: Mixed type vectorization */
    mixed_type_vectorization(copy_buf, int_out, data1, int_in);
    
    /* Test 7: Builtin selector in loop (complex control flow) */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        sin_result[i] += builtin_selector(i, angles[i], data1[i]);
    }
    
    /* Ensure dead code path is at least referenced */
    dead_code_path(data2, data1);
    
    /* Aggregate results to prevent elimination */
    float total = 0.0f;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < SIZE; i++) {
        total += sin_result[i] + cos_result[i] + copy_buf[i] + data2[i] + (float)dbl_out[i];
    }
    
    printf("Final aggregated result: %f\n", total);
    printf("Tests completed.\n");
    
    return 0;
}
