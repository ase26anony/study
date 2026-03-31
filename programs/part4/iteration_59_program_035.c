/* Test program to cover 10-11 operand instruction patterns in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force inlining and optimization */
#define HOT __attribute__((hot, always_inline))
#define NOINLINE __attribute__((noinline))

/* x86 AVX-512 intrinsics */
#ifdef __AVX512F__
#include <immintrin.h>

/* Complex AVX-512 masked operation with many operands */
HOT static NOINLINE __m512 test_avx512_masked_ops(__m512 a, __m512 b, __m512 c, 
                                                  __m512 d, __m512 e, __mmask16 k) {
    /* Chain multiple masked operations to potentially create multi-operand patterns */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);  /* mask, 3 vectors = 4 operands in RTL */
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, e); /* adds more operands */
    
    /* FMA with mask - up to 5 explicit operands in intrinsic */
    __m512 t3 = _mm512_mask_fmadd_ps(t2, k, b, c, d);
    
    /* Another masked operation with blend */
    __m512 t4 = _mm512_mask_blend_ps(k, t1, t3);
    
    /* Complex expression that might expand to many RTL operands */
    return _mm512_mask_sub_ps(t4, k, 
           _mm512_mask_add_ps(a, k, b, c),
           _mm512_mask_mul_ps(d, k, e, t1));
}

/* Test with immediate constants which add more operands */
HOT static NOINLINE __m512i test_avx512_immediate(__m512i a, __m512i b, __m512i c) {
    /* Shifts with immediate values add extra constant operands */
    __m512i t1 = _mm512_slli_epi32(a, 3);      /* immediate 3 is an operand */
    __m512i t2 = _mm512_srli_epi32(b, 5);      /* immediate 5 is an operand */
    
    /* Blend with immediate control */
    __m512i t3 = _mm512_mask_blend_epi32(0xAAAA, t1, t2); /* immediate mask */
    
    /* Permute with immediate */
    __m512i t4 = _mm512_permutexvar_epi32(c, t3);
    
    /* Complex permute with multiple immediates */
    return _mm512_alignr_epi32(t4, t1, 8);     /* immediate 8 */
}
#endif

/* ARM NEON/AArch64 Advanced SIMD */
#ifdef __ARM_NEON
#include <arm_neon.h>

/* Complex multi-vector load/store operations */
HOT static NOINLINE void test_neon_multi_vector(int8_t* data, int8_t* out) {
    /* vld4 loads 4 vectors simultaneously - expands to many operands */
    int8x16x4_t vecs = vld4q_s8(data);
    
    /* Perform operations on all 4 vectors */
    int8x16_t r1 = vaddq_s8(vecs.val[0], vecs.val[1]);
    int8x16_t r2 = vaddq_s8(vecs.val[2], vecs.val[3]);
    int8x16_t r3 = vmulq_s8(r1, r2);
    
    /* Store results back with multi-vector store */
    int8x16x3_t results = {r1, r2, r3};
    vst3q_s8(out, results);
    
    /* Table lookup with multiple table vectors */
    uint8x16_t indices = vld1q_u8((uint8_t*)data);
    uint8x16x2_t tables = {vld1q_u8((uint8_t*)data + 16), vld1q_u8((uint8_t*)data + 32)};
    uint8x16_t tbl_result = vqtbl2q_u8(tables, indices);
    
    /* Complex by-lane operations */
    float32x4_t f1 = vld1q_f32((float*)data);
    float32x4_t f2 = vld1q_f32((float*)data + 4);
    float32x4_t f3 = vfmaq_laneq_f32(f1, f2, f1, 1); /* Lane selection adds operand */
}
#endif

/* GCC Vector Extensions - portable approach */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex vector expression that might generate multi-operand patterns */
HOT static NOINLINE v8sf test_gcc_vector_ops(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression with multiple operations */
    v8sf t1 = a + b * c;          /* Potential FMA pattern */
    v8sf t2 = d - a / b;          /* Multiple operations */
    v8sf t3 = t1 * t2 + c;        /* Another potential FMA */
    v8sf t4 = __builtin_fmaf(a, b, c);  /* Explicit FMA builtin */
    
    /* Blend-like operation using conditional operator */
    v8sf mask = a > b;
    v8sf result = mask ? t3 : t4;
    
    /* Complex reduction across lanes */
    v8sf t5 = {result[7], result[6], result[5], result[4], 
               result[3], result[2], result[1], result[0]};
    
    return result + t5;
}

/* Inline assembly with many operands - direct test of operand handling */
HOT static NOINLINE uint64_t test_many_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                                   uint64_t d, uint64_t e, uint64_t f) {
    uint64_t r1, r2, r3, r4, r5, r6;
    
    /* 11-operand asm statement: 6 outputs + 6 inputs (with duplicates) = 12 total
       but some are tied, resulting in ~10-11 distinct operands */
    asm volatile (
        "mov %[r1], %[a]            \n\t"
        "add %[r1], %[b]            \n\t"
        "mov %[r2], %[c]            \n\t"
        "sub %[r2], %[d]            \n\t"
        "mov %[r3], %[e]            \n\t"
        "imul %[r3], %[f]           \n\t"
        "mov %[r4], %[r1]           \n\t"
        "and %[r4], %[r2]           \n\t"
        "mov %[r5], %[r3]           \n\t"
        "or %[r5], %[r4]            \n\t"
        "mov %[r6], %[r5]           \n\t"
        "xor %[r6], %[a]            \n\t"
        : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3),
          [r4] "=&r" (r4), [r5] "=&r" (r5), [r6] "=&r" (r6)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}

/* OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
HOT static NOINLINE float test_omp_simd_reduction(float* arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * arr[i] + 1.0f;
    }
    
    return sum;
}
#endif

/* Main test driver */
int main() {
    volatile uint64_t result = 0;
    
    /* Test inline assembly with many operands */
    result += test_many_operand_asm(1, 2, 3, 4, 5, 6);
    
#ifdef __AVX512F__
    /* Initialize test vectors */
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __m512 avx_e = _mm512_set1_ps(5.0f);
    
    /* Test masked operations */
    __m512 avx_result = test_avx512_masked_ops(avx_a, avx_b, avx_c, avx_d, avx_e, 0xAAAA);
    
    /* Extract a single value to use the result */
    float avx_sum = _mm512_reduce_add_ps(avx_result);
    result += (uint64_t)avx_sum;
#endif
    
#ifdef __ARM_NEON
    /* Test NEON operations */
    int8_t neon_data[64] = {0};
    int8_t neon_out[48] = {0};
    
    for (int i = 0; i < 64; i++) {
        neon_data[i] = i;
    }
    
    test_neon_multi_vector(neon_data, neon_out);
    
    /* Use the output */
    for (int i = 0; i < 48; i++) {
        result += neon_out[i];
    }
#endif
    
    /* Test GCC vector extensions */
    v8sf gcc_vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf gcc_vec_b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf gcc_vec_c = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    v8sf gcc_vec_d = {1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f};
    
    v8sf gcc_result = test_gcc_vector_ops(gcc_vec_a, gcc_vec_b, gcc_vec_c, gcc_vec_d);
    
    /* Use the result */
    for (int i = 0; i < 8; i++) {
        result += (uint64_t)gcc_result[i];
    }
    
#ifdef _OPENMP
    /* Test OpenMP SIMD reduction */
    float omp_arr[100];
    for (int i = 0; i < 100; i++) {
        omp_arr[i] = i * 0.1f;
    }
    
    float omp_result = test_omp_simd_reduction(omp_arr, 100);
    result += (uint64_t)omp_result;
#endif
    
    printf("Result: %lu\n", (unsigned long)result);
    return (int)(result % 256);
}
