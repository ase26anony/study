/* Test program to cover 10 and 11-operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force aggressive optimization and inlining */
#define HOT __attribute__((hot, always_inline))
#define NOINLINE __attribute__((noinline))

/* Target-specific intrinsics */
#ifdef __AVX512F__
#include <immintrin.h>
#elif defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
#endif

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex reduction using GCC vector extensions */
HOT NOINLINE v8sf complex_vector_operation(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression that might expand to multi-operand instructions */
    v8sf t1 = a * b + c;
    v8sf t2 = d * a - b;
    v8sf t3 = t1 * t2 + a * c;
    v8sf t4 = b * d - t1;
    
    /* Nested FMA-like operations */
    v8sf result = t1 * t2 + t3 * t4;
    
    /* Force use of multiple operands through conditional operations */
    v8sf mask = a > b;
    result = (mask & t1) | (~mask & t2);
    
    return result;
}

#ifdef __AVX512F__
/* AVX-512 mask operations with many operands */
HOT NOINLINE __m512 avx512_complex_op(__m512 a, __m512 b, __m512 c, 
                                      __m512 d, __m512 e, __mmask16 k) {
    /* Chain of masked operations - each can expand to many operands */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, e);
    __m512 t3 = _mm512_mask_fmadd_ps(t2, k, a, b, c);
    __m512 t4 = _mm512_mask_sub_ps(t3, k, d, e);
    
    /* Complex blend with mask */
    __m512 result = _mm512_mask_blend_ps(k, t1, t2);
    result = _mm512_mask_fnmadd_ps(result, k, t3, t4, a);
    
    return result;
}

/* Test AVX-512 gather with multiple operands */
HOT NOINLINE __m512i test_gather(__m512i index, __m512i mask, int scale) {
    int base[1024];
    for (int i = 0; i < 1024; i++) base[i] = i;
    
    /* _mm512_i32gather_epi32 has index, base, scale, mask - during expansion
       this may create many operands */
    return _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask,
                                       index, base, scale);
}
#endif

#ifdef __ARM_NEON
/* ARM NEON complex operations */
HOT NOINLINE float32x4x4_t neon_complex_op(float32x4x4_t a, float32x4x4_t b) {
    /* vld4/vst4 operations involve many operands */
    float32x4x4_t result;
    
    /* Complex interleaved operations */
    result.val[0] = vmlaq_f32(a.val[0], b.val[0], a.val[1]);
    result.val[1] = vmlsq_f32(a.val[1], b.val[1], a.val[2]);
    result.val[2] = vfmaq_f32(a.val[2], b.val[2], a.val[3]);
    result.val[3] = vfmsq_f32(a.val[3], b.val[3], a.val[0]);
    
    /* Table lookup with multiple operands */
    uint8x16x4_t tbl;
    tbl.val[0] = vreinterpretq_u8_f32(result.val[0]);
    tbl.val[1] = vreinterpretq_u8_f32(result.val[1]);
    tbl.val[2] = vreinterpretq_u8_f32(result.val[2]);
    tbl.val[3] = vreinterpretq_u8_f32(result.val[3]);
    
    return result;
}
#endif

/* Inline assembly with many operands - directly tests operand handling */
HOT NOINLINE uint64_t many_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i,
                                       uint64_t j) {
    uint64_t out1, out2, out3, out4, out5;
    
    /* 11-operand asm statement */
    asm volatile (
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "sub %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "and %2, %2, %10\n\t"
        "mov %3, %11\n\t"
        "or %3, %3, %12\n\t"
        "mov %4, %13\n\t"
        "xor %4, %4, %14"
        : "=&r"(out1), "=&r"(out2), "=&r"(out3), "=&r"(out4), "=&r"(out5)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Complex built-in usage */
HOT NOINLINE double complex_builtins(double a, double b, double c, 
                                     double d, double e) {
    /* Chain of FMA operations - each expands to multiple operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, t1);
    double t3 = __builtin_fma(a, c, b);
    double t4 = __builtin_fma(d, t1, t2);
    
    /* Complex expression preventing early folding */
    double result = __builtin_fma(t1, t2, __builtin_fma(t3, t4, a));
    result = __builtin_fma(result, b, __builtin_fma(c, d, e));
    
    return result;
}

/* OpenMP SIMD reduction with vector types */
void omp_vector_reduction(v8sf *data, int n, v8sf *result) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex expression that might expand to multi-operand instructions */
        v8sf temp = data[i];
        sum = sum + temp * temp - data[i % n] + data[(i + 1) % n];
    }
    
    *result = sum;
}

int main() {
    /* Initialize test data */
    v8sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vec2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    v8sf vec3 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf vec4 = {9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f};
    
    /* Test GCC vector extensions */
    v8sf result1 = complex_vector_operation(vec1, vec2, vec3, vec4);
    
    /* Test complex builtins */
    double builtin_result = complex_builtins(1.1, 2.2, 3.3, 4.4, 5.5);
    
    /* Test inline assembly with many operands */
    uint64_t asm_result = many_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test OpenMP reduction */
    v8sf data[100];
    v8sf omp_result;
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 8; j++) {
            data[i][j] = (float)(i + j);
        }
    }
    omp_vector_reduction(data, 100, &omp_result);
    
    #ifdef __AVX512F__
    /* Test AVX-512 operations */
    __m512 avx_vec1 = _mm512_set1_ps(1.0f);
    __m512 avx_vec2 = _mm512_set1_ps(2.0f);
    __m512 avx_vec3 = _mm512_set1_ps(3.0f);
    __m512 avx_vec4 = _mm512_set1_ps(4.0f);
    __m512 avx_vec5 = _mm512_set1_ps(5.0f);
    __mmask16 mask = 0xAAAA;
    
    __m512 avx_result = avx512_complex_op(avx_vec1, avx_vec2, avx_vec3,
                                          avx_vec4, avx_vec5, mask);
    
    /* Test gather */
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __m512i gather_mask = _mm512_set1_epi32(-1);
    __m512i gather_result = test_gather(index, gather_mask, 4);
    #endif
    
    #ifdef __ARM_NEON
    /* Test ARM NEON operations */
    float32x4x4_t neon_a, neon_b, neon_result;
    for (int i = 0; i < 4; i++) {
        neon_a.val[i] = vdupq_n_f32((float)(i + 1));
        neon_b.val[i] = vdupq_n_f32((float)(i + 2));
    }
    neon_result = neon_complex_op(neon_a, neon_b);
    #endif
    
    /* Print results to prevent optimization */
    printf("Result1: %f\n", result1[0]);
    printf("Builtin result: %f\n", builtin_result);
    printf("ASM result: %lu\n", asm_result);
    printf("OMP result: %f\n", omp_result[0]);
    
    return 0;
}
