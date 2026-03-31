/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force inlining and optimization */
#define HOT __attribute__((hot, always_inline))
#define NOINLINE __attribute__((noinline))

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* AVX-512 types */
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

/* Complex operation using multiple AVX-512 masked intrinsics */
HOT NOINLINE static v16sf avx512_complex_op(v16sf a, v16sf b, v16sf c, 
                                           v16sf d, __mmask16 mask) {
    /* Chain multiple masked operations to increase operand count */
    v16sf t1 = _mm512_mask_add_ps(a, mask, b, c);
    v16sf t2 = _mm512_mask_mul_ps(t1, mask, d, _mm512_set1_ps(2.0f));
    v16sf t3 = _mm512_mask_sub_ps(t2, mask, t1, _mm512_set1_ps(1.0f));
    
    /* FMA with mask - can generate many operands */
    v16sf result = _mm512_mask_fmadd_ps(t3, mask, a, b, c);
    
    /* Additional operation with immediate */
    result = _mm512_maskz_mov_ps(mask, result);
    
    return result;
}

/* Test AVX-512 11-operand pattern */
HOT NOINLINE static v8df avx512_11_operand(v8df a, v8df b, v8df c, v8df d,
                                          v8df e, v8df f, __mmask8 mask) {
    /* Complex sequence that might expand to 11 operands */
    v8df t1 = _mm512_mask_add_pd(a, mask, b, c);
    v8df t2 = _mm512_mask_mul_pd(t1, mask, d, e);
    v8df t3 = _mm512_mask_fmadd_pd(t2, mask, f, a, _mm512_set1_pd(3.14));
    
    /* Multiple operations chained */
    v8df result = _mm512_mask_sub_pd(t3, mask, 
                                    _mm512_mask_add_pd(b, mask, c, d),
                                    _mm512_set1_pd(1.0));
    
    return result;
}

#endif

#ifdef __aarch64__
#include <arm_neon.h>
#include <arm_acle.h>

/* ARM SVE/NEON complex operations */
typedef float32x4_t v4sf;
typedef float32x4x4_t v4sfx4;

/* Complex load/store with lane operations */
HOT NOINLINE static v4sfx4 neon_complex_load(const float* data) {
    /* vld4q loads 4 vectors with deinterleaving - can generate many operands */
    v4sfx4 result = vld4q_f32(data);
    
    /* Additional operations on each lane */
    result.val[0] = vaddq_f32(result.val[0], vdupq_n_f32(1.0f));
    result.val[1] = vmulq_f32(result.val[1], vdupq_n_f32(2.0f));
    result.val[2] = vfmaq_f32(result.val[2], result.val[3], vdupq_n_f32(3.0f));
    result.val[3] = vsubq_f32(result.val[3], result.val[0]);
    
    return result;
}

/* Table lookup with multiple operands */
HOT NOINLINE static uint8x16_t neon_table_lookup(uint8x16_t a, uint8x16_t b,
                                                uint8x16_t c, uint8x16_t d) {
    /* Complex permute/table lookup */
    uint8x16_t t1 = vqtbl1q_u8(a, b);
    uint8x16_t t2 = vqtbl1q_u8(c, d);
    uint8x16_t result = vaddq_u8(t1, t2);
    
    /* Additional operations */
    result = vshlq_n_u8(result, 2);
    result = vandq_u8(result, vdupq_n_u8(0xF));
    
    return result;
}

#endif

/* GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Complex expression with GCC vector extensions */
HOT NOINLINE static v8sf gcc_vector_complex(v8sf a, v8sf b, v8sf c, v8sf d,
                                           v8sf e, v8sf f, v8sf g) {
    /* Complex expression that might require many temporaries */
    v8sf t1 = a + b * c;
    v8sf t2 = d - e / f;
    v8sf t3 = t1 * t2 + g;
    v8sf t4 = __builtin_fmaf(a, b, c);
    v8sf result = t3 * t4 + a * b - c * d + e * f - g;
    
    /* Additional operations to increase complexity */
    result = result + __builtin_fmaf(d, e, f);
    result = result * __builtin_fmaf(g, a, b);
    
    return result;
}

/* Inline assembly with many operands */
HOT NOINLINE static uint64_t multi_operand_asm(uint64_t a, uint64_t b,
                                              uint64_t c, uint64_t d,
                                              uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h) {
    uint64_t r1, r2, r3, r4;
    
    /* 11-operand asm statement */
    __asm__ volatile (
        "add %[r1], %[a], %[b]\n\t"
        "mul %[r2], %[c], %[d]\n\t"
        "sub %[r3], %[e], %[f]\n\t"
        "and %[r4], %[g], %[h]\n\t"
        "or %[r1], %[r1], %[r2]\n\t"
        "xor %[r2], %[r3], %[r4]\n\t"
        "add %[r3], %[r1], %[r2]"
        : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3), [r4] "=&r" (r4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4;
}

/* OpenMP SIMD reduction with vector types */
HOT NOINLINE static v4df omp_vector_reduction(const v4df* arr, int n) {
    v4df sum = {0, 0, 0, 0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex expression in loop */
        v4df t1 = arr[i] * arr[i];
        v4df t2 = __builtin_fma(arr[i], arr[i], arr[i]);
        sum = sum + t1 + t2 + __builtin_fma(t1, t2, arr[i]);
    }
    
    return sum;
}

/* Main test function */
int main() {
    volatile int result = 0;
    
    /* Test inline assembly path */
    result += multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Test GCC vector extensions */
    v8sf v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8sf v2 = {2, 3, 4, 5, 6, 7, 8, 9};
    v8sf v3 = {3, 4, 5, 6, 7, 8, 9, 10};
    v8sf v4 = {4, 5, 6, 7, 8, 9, 10, 11};
    v8sf v5 = {5, 6, 7, 8, 9, 10, 11, 12};
    v8sf v6 = {6, 7, 8, 9, 10, 11, 12, 13};
    v8sf v7 = {7, 8, 9, 10, 11, 12, 13, 14};
    
    v8sf vec_result = gcc_vector_complex(v1, v2, v3, v4, v5, v6, v7);
    result += (int)vec_result[0];
    
    /* Test OpenMP reduction */
    v4df arr[4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    
    v4df omp_result = omp_vector_reduction(arr, 4);
    result += (int)omp_result[0];
    
    /* Target-specific tests */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        __m512 avx_a = _mm512_set1_ps(1.0f);
        __m512 avx_b = _mm512_set1_ps(2.0f);
        __m512 avx_c = _mm512_set1_ps(3.0f);
        __m512 avx_d = _mm512_set1_ps(4.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 avx_result = avx512_complex_op(avx_a, avx_b, avx_c, avx_d, mask);
        result += (int)_mm512_cvtss_f32(avx_result);
        
        /* Test 11-operand pattern */
        __m512d davx_a = _mm512_set1_pd(1.0);
        __m512d davx_b = _mm512_set1_pd(2.0);
        __m512d davx_c = _mm512_set1_pd(3.0);
        __m512d davx_d = _mm512_set1_pd(4.0);
        __m512d davx_e = _mm512_set1_pd(5.0);
        __m512d davx_f = _mm512_set1_pd(6.0);
        __mmask8 dmask = 0xAA;
        
        __m512d davx_result = avx512_11_operand(davx_a, davx_b, davx_c, davx_d,
                                               davx_e, davx_f, dmask);
        result += (int)_mm512_cvtsd_f64(davx_result);
    }
    #endif
    
    #ifdef __aarch64__
    float neon_data[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v4sfx4 neon_result = neon_complex_load(neon_data);
    result += (int)vgetq_lane_f32(neon_result.val[0], 0);
    
    uint8x16_t neon_a = vdupq_n_u8(1);
    uint8x16_t neon_b = vdupq_n_u8(2);
    uint8x16_t neon_c = vdupq_n_u8(3);
    uint8x16_t neon_d = vdupq_n_u8(4);
    
    uint8x16_t neon_tbl_result = neon_table_lookup(neon_a, neon_b, neon_c, neon_d);
    result += vgetq_lane_u8(neon_tbl_result, 0);
    #endif
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
