/* test_multi_operand_expansion.c
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine -o test test_multi_operand_expansion.c
 */

#include <stdint.h>
#include <string.h>

/* Prevent dead code elimination */
volatile int sink;

/* Generic vector types for portability */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* ==================== ARCHITECTURE-SPECIFIC INTRINSICS ==================== */

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

void test_avx512_multi_operand(void) {
    /* AVX-512 masked operations with multiple operands */
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __m512d d = _mm512_set1_pd(4.0);
    __m512d e = _mm512_set1_pd(5.0);
    __m512d f = _mm512_set1_pd(6.0);
    __m512d g = _mm512_set1_pd(7.0);
    __m512d h = _mm512_set1_pd(8.0);
    __mmask8 m = 0xFF;
    
    /* Complex expression that might combine into multi-operand instruction */
    __m512d result = _mm512_fmadd_pd(a, b, _mm512_fmadd_pd(c, d, 
                          _mm512_fmadd_pd(e, f, _mm512_fmadd_pd(g, h, a))));
    
    sink = _mm512_movemask_pd(result);
}

void test_avx512_masked_store(void) {
    /* AVX-512 masked store with many parameters */
    __m512i src = _mm512_set1_epi32(42);
    __mmask16 mask = 0xAAAA;
    int32_t mem[16] __attribute__((aligned(64)));
    
    /* This intrinsic expands to instruction with mask, src, memory operand */
    _mm512_mask_store_epi32(mem, mask, src);
    
    sink = mem[0];
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>
#include <arm_acle.h>

void test_sve2_multi_lane(void) {
    /* Simulate SVE2-style multi-operand intrinsics using NEON */
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    int32x4_t e = vdupq_n_s32(5);
    int32x4_t f = vdupq_n_s32(6);
    int32x4_t g = vdupq_n_s32(7);
    int32x4_t h = vdupq_n_s32(8);
    int32x4_t i = vdupq_n_s32(9);
    int32x4_t j = vdupq_n_s32(10);
    
    /* Complex chain that might be combined */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(t1, d, e);
    int32x4_t t3 = vmlaq_s32(t2, f, g);
    int32x4_t t4 = vmlaq_s32(t3, h, i);
    int32x4_t result = vaddq_s32(t4, j);
    
    sink = vgetq_lane_s32(result, 0);
}

#endif /* __aarch64__ */

#ifdef __powerpc64__
#include <altivec.h>

void test_vsx_multi_operand(void) {
    /* PowerPC VSX complex operations */
    vector double a = vec_splats(1.0);
    vector double b = vec_splats(2.0);
    vector double c = vec_splats(3.0);
    vector double d = vec_splats(4.0);
    vector double e = vec_splats(5.0);
    vector double f = vec_splats(6.0);
    
    /* VSX multiply-add with many operands */
    vector double t1 = vec_madd(a, b, c);
    vector double t2 = vec_madd(d, e, f);
    vector double result = vec_add(t1, t2);
    
    sink = vec_extract(result, 0);
}

#endif /* __powerpc64__ */

/* ==================== INLINE ASSEMBLY WITH MANY OPERANDS ==================== */

void test_inline_asm_11_operands(void) {
    /* Force 11-operand expansion via inline asm */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t f = 6, g = 7, h = 8, i = 9, j = 10;
    uint64_t k = 11;
    uint64_t result;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "/* 11-operand asm block */\n\t"
        "mov %[out], %[in1]\n\t"
        "add %[out], %[in2]\n\t"
        "add %[out], %[in3]\n\t"
        "add %[out], %[in4]\n\t"
        "add %[out], %[in5]\n\t"
        "add %[out], %[in6]\n\t"
        "add %[out], %[in7]\n\t"
        "add %[out], %[in8]\n\t"
        "add %[out], %[in9]\n\t"
        "add %[out], %[in10]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i),
          [in10] "r" (j)
        : "cc"
    );
    
    sink = result + k; /* Use all 11 variables */
}

void test_inline_asm_10_operands_mixed(void) {
    /* 10 operands with mixed constraints */
    uint32_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint32_t f = 6, g = 7, h = 8, i = 9;
    uint32_t mem[4] = {10, 11, 12, 13};
    uint32_t result;
    
    /* 10 operands: 1 output + 8 registers + 1 memory */
    asm volatile (
        "/* 10-operand mixed asm */\n\t"
        "mov %[out], %[in1]\n\t"
        "imul %[out], %[in2]\n\t"
        "add %[out], %[in3]\n\t"
        "add %[out], %[in4]\n\t"
        "add %[out], %[in5]\n\t"
        "add %[out], %[in6]\n\t"
        "add %[out], %[in7]\n\t"
        "add %[out], %[in8]\n\t"
        "add %[out], (%[mem])"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h),
          [mem] "r" (mem)
        : "memory", "cc"
    );
    
    sink = result + i;
}

/* ==================== COMPLEX EXPRESSIONS FOR COMBINE PASS ==================== */

double test_complex_expression_10_operands(void) {
    /* Create a complex expression that might be combined into a single
     * multi-operand instruction during the combine pass */
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
    double f = 6.6, g = 7.7, h = 8.8, i = 9.9, j = 10.10;
    
    /* Fused multiply-add chain with 10 operands */
    double result = a * b + c * d + e * f + g * h + i * j;
    
    /* Additional complexity to prevent simple decomposition */
    result = result * a + b * c + d * e + f * g + h * i + j;
    
    return result;
}

v4sf test_vector_expression_11_operands(v4sf a, v4sf b, v4sf c, v4sf d, v4sf e,
                                        v4sf f, v4sf g, v4sf h, v4sf i, v4sf j) {
    /* Vector expression with 11 vector operands */
    v4sf t1 = a * b + c;
    v4sf t2 = d * e + f;
    v4sf t3 = g * h + i;
    v4sf result = t1 + t2 + t3 + j;
    
    /* Additional operations to encourage combining */
    result = result * a + b * c + d * e + f * g + h * i + j;
    
    return result;
}

/* ==================== ATOMIC BUILTINS WITH MANY PARAMETERS ==================== */

void test_atomic_compare_exchange(void) {
    /* __atomic_compare_exchange has 6 parameters, which might expand
     * to a multi-operand instruction on some architectures */
    int64_t atomic_var = 42;
    int64_t expected = 42;
    int64_t desired = 100;
    int success;
    
    /* Full 6-parameter version */
    success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                        0, /* weak */
                                        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    sink = success + atomic_var + expected + desired;
}

/* ==================== BIT-FIELD MANIPULATION ==================== */

uint64_t test_bitfield_operations(void) {
    /* Complex bit-field insertion/extraction across multiple words */
    uint64_t a = 0xAAAAAAAAAAAAAAAA;
    uint64_t b = 0xBBBBBBBBBBBBBBBB;
    uint64_t c = 0xCCCCCCCCCCCCCCCC;
    uint64_t d = 0xDDDDDDDDDDDDDDDD;
    uint64_t e = 0xEEEEEEEEEEEEEEEE;
    uint64_t f = 0xFFFFFFFFFFFFFFFF;
    
    /* Extract bits from multiple sources and combine */
    uint64_t result = ((a & 0xFF) << 56) |
                      ((b & 0xFF00) << 40) |
                      ((c & 0xFF0000) << 24) |
                      ((d & 0xFF000000) << 8) |
                      ((e & 0xFF00000000) >> 8) |
                      ((f & 0xFF0000000000) >> 24);
    
    /* Additional bit manipulation */
    result = (result >> 32) | (result << 32);
    result = result ^ a ^ b ^ c ^ d ^ e ^ f;
    
    return result;
}

/* ==================== MAIN DRIVER ==================== */

int main(void) {
    int total = 0;
    
    /* Test architecture-specific intrinsics */
#if defined(__x86_64__)
    test_avx512_multi_operand();
    test_avx512_masked_store();
    total += 1;
#elif defined(__aarch64__)
    test_sve2_multi_lane();
    total += 2;
#elif defined(__powerpc64__)
    test_vsx_multi_operand();
    total += 3;
#endif
    
    /* Test inline assembly with many operands */
    test_inline_asm_11_operands();
    test_inline_asm_10_operands_mixed();
    total += 4;
    
    /* Test complex expressions */
    double expr_result = test_complex_expression_10_operands();
    total += (int)expr_result;
    
    /* Test vector expressions */
    v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf ve = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vf = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vg = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vh = {29.0f, 30.0f, 31.0f, 32.0f};
    v4sf vi = {33.0f, 34.0f, 35.0f, 36.0f};
    v4sf vj = {37.0f, 38.0f, 39.0f, 40.0f};
    
    v4sf vresult = test_vector_expression_11_operands(va, vb, vc, vd, ve,
                                                      vf, vg, vh, vi, vj);
    total += (int)vresult[0];
    
    /* Test atomic builtins */
    test_atomic_compare_exchange();
    total += 5;
    
    /* Test bit-field operations */
    uint64_t bit_result = test_bitfield_operations();
    total += (int)bit_result;
    
    /* Return sum to prevent optimization */
    return total % 256;
}
