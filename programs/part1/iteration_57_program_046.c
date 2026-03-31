/* test_multi_operand.c - Test program for 10/11-operand instruction expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
  #define TARGET_X86 1
  #include <immintrin.h>
  #include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
  #define TARGET_ARM 1
  #ifdef __ARM_NEON
    #include <arm_neon.h>
  #endif
  #ifdef __ARM_FEATURE_SVE
    #include <arm_sve.h>
  #endif
#elif defined(__powerpc__) || defined(__PPC__)
  #define TARGET_PPC 1
  #include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single instruction
       with many operands during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline uint64_t inline_asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with exactly 11 operands to force expansion */
    __asm__ volatile (
        "/* 11-operand test */\n\t"
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Function using inline assembly with 10 operands */
static inline uint64_t inline_asm_10_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j) {
    uint64_t result;
    
    /* Inline assembly with exactly 10 operands */
    __asm__ volatile (
        "/* 10-operand test */\n\t"
        "imul %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

/* Test atomic built-in with many parameters */
static int test_atomic_builtin(int *ptr, int expected, int desired) {
    int weak = 0;
    int success = __atomic_compare_exchange(ptr, &expected, &desired,
                                            weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return success;
}

#if TARGET_X86
/* AVX-512 intrinsic with many operands */
static __m512d test_avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                         __m512d d, __m512d e, __m512d f,
                                         __mmask8 k1, __mmask8 k2) {
    /* Complex expression that might use multi-operand FMA */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* Masked operation with many operands */
    __m512d result = _mm512_mask_add_pd(t1, k1, t2, _mm512_set1_pd(1.0));
    result = _mm512_mask_mul_pd(result, k2, result, _mm512_set1_pd(2.0));
    
    return result;
}

/* AVX-512 masked FMA with rounding control - potentially 11 operands */
static __m512d test_avx512_masked_fma_round(__m512d a, __m512d b, __m512d c,
                                            __mmask8 k, int rounding) {
    /* This intrinsic expands to instruction with many operands:
       dest, src1, src2, src3, mask, rounding control */
    return _mm512_mask_fmadd_round_pd(a, k, b, c, rounding);
}
#endif

#if TARGET_ARM && defined(__ARM_FEATURE_SVE)
/* SVE2 intrinsic with lane selection - potentially many operands */
static svint32_t test_sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                        svint32_t d, svint32_t e, svint32_t f,
                                        svint32_t g, svint32_t h) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_s32(a, b, c);
    svint32_t t2 = svmla_s32(d, e, f);
    svint32_t result = svadd_s32(t1, t2);
    result = svmla_s32(result, g, h);
    
    return result;
}
#endif

#if TARGET_PPC
/* PowerPC VSX/Altivec operations */
static vector signed int test_ppc_multi_operand(vector signed int a,
                                                vector signed int b,
                                                vector signed int c,
                                                vector signed int d,
                                                vector signed int e,
                                                vector signed int f) {
    /* Complex permute and compute */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, e, f);
    vector signed int result = vec_add(t1, t2);
    
    /* Permute with many operands */
    result = vec_perm(result, t1, (vector unsigned char){0,1,2,3,4,5,6,7,
                                                         8,9,10,11,12,13,14,15});
    
    return result;
}
#endif

/* Main test driver */
int main() {
    int result = 0;
    
    printf("Testing multi-operand instruction expansion...\n");
    
    /* Test 1: Complex arithmetic expression (10 operands) */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    printf("Complex expression result: %d\n", expr_result);
    
    /* Test 2: Inline assembly with 11 operands */
    uint64_t asm11_result = inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm11_result;
    printf("11-operand asm result: %lu\n", asm11_result);
    
    /* Test 3: Inline assembly with 10 operands */
    uint64_t asm10_result = inline_asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += (int)asm10_result;
    printf("10-operand asm result: %lu\n", asm10_result);
    
    /* Test 4: Atomic built-in with many parameters */
    int atomic_var = 42;
    int atomic_result = test_atomic_builtin(&atomic_var, 42, 100);
    result += atomic_result;
    printf("Atomic compare-exchange: %s\n", atomic_result ? "success" : "failed");
    
    /* Test 5: Architecture-specific intrinsics */
    #if TARGET_X86
    {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __m512d avx_f = _mm512_set1_pd(6.0);
        
        __m512d avx_result = test_avx512_multi_operand(avx_a, avx_b, avx_c,
                                                       avx_d, avx_e, avx_f,
                                                       0xFF, 0x0F);
        double avx_sum = _mm512_reduce_add_pd(avx_result);
        result += (int)avx_sum;
        printf("AVX-512 result sum: %f\n", avx_sum);
    }
    #endif
    
    #if TARGET_ARM && defined(__ARM_NEON)
    {
        /* ARM NEON vector operations */
        int32x4_t neon_a = vdupq_n_s32(1);
        int32x4_t neon_b = vdupq_n_s32(2);
        int32x4_t neon_c = vdupq_n_s32(3);
        int32x4_t neon_d = vdupq_n_s32(4);
        
        /* Complex vector expression */
        int32x4_t neon_t1 = vmlaq_s32(neon_a, neon_b, neon_c);
        int32x4_t neon_t2 = vmlaq_s32(neon_d, neon_b, neon_c);
        int32x4_t neon_result = vaddq_s32(neon_t1, neon_t2);
        
        /* Extract and sum */
        int32_t neon_sum = vaddvq_s32(neon_result);
        result += neon_sum;
        printf("NEON result sum: %d\n", neon_sum);
    }
    #endif
    
    /* Test 6: Fused multiply-add chain (potential for instruction combining) */
    {
        double fmadd_chain = 0.0;
        double v1 = 1.1, v2 = 2.2, v3 = 3.3, v4 = 4.4, v5 = 5.5;
        double v6 = 6.6, v7 = 7.7, v8 = 8.8, v9 = 9.9, v10 = 10.1;
        
        /* This might be combined into multi-operand instruction */
        fmadd_chain = v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10;
        result += (int)fmadd_chain;
        printf("FMA chain result: %f\n", fmadd_chain);
    }
    
    /* Test 7: Bit-field operations across multiple words */
    {
        uint64_t bf1 = 0x123456789ABCDEF0ULL;
        uint64_t bf2 = 0xFEDCBA9876543210ULL;
        uint64_t bf3 = 0xAAAAAAAA55555555ULL;
        uint64_t bf4 = 0x55555555AAAAAAAAULL;
        
        /* Complex bit manipulation that might use multi-operand instructions */
        uint64_t bf_result = ((bf1 & 0xFFFF0000FFFF0000ULL) >> 16) |
                             ((bf2 & 0x0000FFFF0000FFFFULL) << 16) |
                             ((bf3 & 0xFF00FF00FF00FF00ULL) >> 8) |
                             ((bf4 & 0x00FF00FF00FF00FFULL) << 8);
        
        result += (int)bf_result;
        printf("Bit-field result: %lx\n", bf_result);
    }
    
    printf("Final accumulated result: %d\n", result);
    return result != 0 ? 0 : 1;
}
