#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t prng() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(__m256i* arr_i, __m256d* arr_d, __m256* arr_f, size_t n) {
    for (size_t i = 0; i < n; i++) {
        uint32_t vals[8];
        for (int j = 0; j < 8; j++) vals[j] = prng();
        arr_i[i] = _mm256_set_epi32(vals[7], vals[6], vals[5], vals[4], 
                                   vals[3], vals[2], vals[1], vals[0]);
        
        double dvals[4];
        for (int j = 0; j < 4; j++) dvals[j] = (double)prng() / 1000.0;
        arr_d[i] = _mm256_set_pd(dvals[3], dvals[2], dvals[1], dvals[0]);
        
        float fvals[8];
        for (int j = 0; j < 8; j++) fvals[j] = (float)prng() / 1000.0f;
        arr_f[i] = _mm256_set_ps(fvals[7], fvals[6], fvals[5], fvals[4],
                                fvals[3], fvals[2], fvals[1], fvals[0]);
    }
}

/* Complex expression with many temporaries - forces expander to create many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* out, const __m256i* in1, const __m256i* in2,
                          const __m256d* in3, const __m256* in4, size_t n) {
    volatile size_t i = 0; /* Prevent loop unrolling */
    
    for (i = 0; i < n; i++) {
        /* Load multiple vectors - creates many SSA values */
        __m256i v1 = in1[i];
        __m256i v2 = in2[i];
        __m256d d1 = in3[i];
        __m256 f1 = in4[i];
        
        /* Complex chain of operations with many intermediate values */
        __m256i t1 = _mm256_add_epi32(v1, v2);
        __m256i t2 = _mm256_sub_epi32(v1, v2);
        __m256i t3 = _mm256_mullo_epi32(t1, t2);
        
        /* Convert vectors - might use __builtin_convertvector with many args */
        __m256d d2 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(t3));
        __m256 f2 = _mm256_cvtepi32_ps(t3);
        
        /* Create a complex shuffle mask with many immediate arguments */
        /* This is designed to potentially trigger the 10-11 argument case */
        __m256i shuffle_mask = _mm256_set_epi32(
            7, 6, 5, 4, 3, 2, 1, 0  /* 8 arguments already */
        );
        
        /* Extended inline asm with 11 operands */
        /* This directly targets the uncovered lines */
        __m256i result;
        asm volatile (
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %0, %3\n\t"
            "vpmulld %0, %0, %4\n\t"
            "vpshufd %0, %0, %5\n\t"
            "vpblendd %0, %0, %6, %7\n\t"
            : "=&x" (result)
            : "x" (v1), "x" (v2), "x" (t1), 
              "x" (t2), "i" (0x1B),  /* 6th arg: immediate */
              "x" (t3), "i" (0xF0),  /* 8th arg: immediate */
              "m" (in1[i]),          /* 9th arg: memory */
              "m" (in2[i]),          /* 10th arg: memory */
              "r" (i)                /* 11th arg: integer */
            : "memory"
        );
        
        /* Another complex operation using GCC vector builtins */
        /* This creates a complex expression tree */
        typedef int32_t v8si __attribute__((vector_size(32)));
        typedef float v8sf __attribute__((vector_size(32)));
        typedef double v4df __attribute__((vector_size(32)));
        
        v8si vi1 = (v8si)v1;
        v8si vi2 = (v8si)v2;
        v8sf vf1 = (v8sf)f1;
        v4df vd1 = (v4df)d1;
        
        /* Complex expression with many operands */
        v8si temp_result = vi1 + vi2;
        temp_result = temp_result * (vi1 - vi2);
        temp_result = temp_result >> 1;
        temp_result = temp_result | (vi1 & vi2);
        
        /* Convert with potential many-argument builtin */
        v8sf float_result = __builtin_convertvector(temp_result, v8sf);
        
        /* Blend operation with many arguments */
        __m256 final_blend;
        asm volatile (
            "vblendps %0, %1, %2, %3\n\t"
            "vaddps %0, %0, %4\n\t"
            "vmulps %0, %0, %5\n\t"
            "vfmadd132ps %0, %6, %7\n\t"
            : "=x" (final_blend)
            : "x" (f1), "x" (f2), "i" (0xAA),
              "x" (float_result), "x" (float_result),
              "x" (float_result), "x" (float_result),
              "m" (in4[i]), "r" (i), "i" (255)
            : "memory"
        );
        
        /* Store results */
        out[i] = result;
    }
}

/* Alternative function using ARM NEON style if compiled for ARM */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline, target("arch=armv8-a+simd")))
static void test_many_args_neon(int32x4_t* out, const int32x4_t* in1, 
                               const int32x4_t* in2, const float32x4_t* in3,
                               const float64x2_t* in4, size_t n) {
    volatile size_t i = 0;
    
    for (i = 0; i < n; i++) {
        int32x4_t v1 = in1[i];
        int32x4_t v2 = in2[i];
        float32x4_t f1 = in3[i];
        float64x2_t d1 = in4[i];
        
        /* Complex NEON operations with many lane selections */
        int32x4_t t1 = vaddq_s32(v1, v2);
        int32x4_t t2 = vsubq_s32(v1, v2);
        int32x4_t t3 = vmulq_s32(t1, t2);
        
        /* Extended asm with many operands for ARM */
        int32x4_t result;
        asm volatile (
            "add %0.4s, %1.4s, %2.4s\n\t"
            "sub %0.4s, %0.4s, %3.4s\n\t"
            "mul %0.4s, %0.4s, %4.4s\n\t"
            "rev64 %0.4s, %0.4s\n\t"
            "zip1 %0.4s, %0.4s, %5.4s\n\t"
            : "=w" (result)
            : "w" (v1), "w" (v2), "w" (t1),
              "w" (t2), "w" (t3),
              "m" (in1[i]), "m" (in2[i]),
              "r" (i), "i" (0), "i" (255)
            : "memory"
        );
        
        out[i] = result;
    }
}
#endif

/* Multi-statement expression with many temporaries */
__attribute__((noinline))
static int complex_expression(int a, int b, int c, int d, int e,
                             int f, int g, int h, int i, int j) {
    /* Force many intermediate values */
    int t1 = a + b;
    int t2 = c - d;
    int t3 = e * f;
    int t4 = g / (h + 1);
    int t5 = i ^ j;
    int t6 = t1 & t2;
    int t7 = t3 | t4;
    int t8 = t5 << 2;
    int t9 = t6 >> 1;
    int t10 = t7 + t8;
    
    /* Final complex expression */
    return (t10 * t9) + (t1 * t2) - (t3 / t4) | (t5 & t6) ^ (t7 | t8);
}

/* Main function with checksum calculation */
int main() {
    const size_t N = 1024;
    const size_t VEC_N = N / 8; /* 8 ints per AVX2 vector */
    
    /* Allocate aligned memory for vectors */
    __m256i* vec_in1 = (__m256i*)aligned_alloc(32, VEC_N * sizeof(__m256i));
    __m256i* vec_in2 = (__m256i*)aligned_alloc(32, VEC_N * sizeof(__m256i));
    __m256d* vec_in3 = (__m256d*)aligned_alloc(32, VEC_N * sizeof(__m256d));
    __m256* vec_in4 = (__m256*)aligned_alloc(32, VEC_N * sizeof(__m256));
    __m256i* vec_out = (__m256i*)aligned_alloc(32, VEC_N * sizeof(__m256i));
    
    if (!vec_in1 || !vec_in2 || !vec_in3 || !vec_in4 || !vec_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(vec_in1, vec_in3, vec_in4, VEC_N);
    
    /* Fill second input array */
    for (size_t i = 0; i < VEC_N; i++) {
        uint32_t vals[8];
        for (int j = 0; j < 8; j++) vals[j] = prng();
        vec_in2[i] = _mm256_set_epi32(vals[7], vals[6], vals[5], vals[4],
                                     vals[3], vals[2], vals[1], vals[0]);
    }
    
    /* Call the function with many arguments */
    test_many_args(vec_out, vec_in1, vec_in2, vec_in3, vec_in4, VEC_N);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    int32_t* out_data = (int32_t*)vec_out;
    for (size_t i = 0; i < N; i++) {
        checksum += (uint64_t)out_data[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Also test the scalar complex expression */
    int scalar_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    printf("Scalar result: %d\n", scalar_result);
    
    /* Cleanup */
    free(vec_in1);
    free(vec_in2);
    free(vec_in3);
    free(vec_in4);
    free(vec_out);
    
    return 0;
}
