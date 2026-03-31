/* test_multi_operand_expansion.c
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine -o test test_multi_operand_expansion.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== Architecture-Specific Intrinsics ==================== */

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* AVX-512 operations with many operands */
void test_avx512_multi_operand(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __m512d d = _mm512_set1_pd(4.0);
    __m512d e = _mm512_set1_pd(5.0);
    __m512d f = _mm512_set1_pd(6.0);
    __m512d g = _mm512_set1_pd(7.0);
    __m512d h = _mm512_set1_pd(8.0);
    __m512d i = _mm512_set1_pd(9.0);
    __m512d j = _mm512_set1_pd(10.0);
    __mmask8 k = 0xFF;
    
    /* This could potentially expand to a multi-operand instruction */
    __m512d result = _mm512_fmadd_pd(a, b, _mm512_fmadd_pd(c, d, 
                          _mm512_fmadd_pd(e, f, _mm512_fmadd_pd(g, h, 
                          _mm512_fmadd_pd(i, j, a)))));
    
    /* Use result to prevent optimization */
    volatile __m512d sink = result;
    (void)sink;
}

/* AVX-512 masked operation with many parameters */
void test_avx512_masked(void) {
    __m512i src1 = _mm512_set1_epi32(1);
    __m512i src2 = _mm512_set1_epi32(2);
    __m512i src3 = _mm512_set1_epi32(3);
    __mmask16 mask = 0xFFFF;
    int imm8 = 1;
    
    /* Complex operation that might use many operands */
    __m512i result = _mm512_mask_add_epi32(src1, mask, src2, src3);
    
    volatile __m512i sink = result;
    (void)sink;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#include <arm_acle.h>

/* ARM SVE/SVE2 style multi-operand operations (using NEON as proxy) */
void test_arm_multi_operand(void) {
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
    
    volatile int32x4_t sink = result;
    (void)sink;
}
#endif

#ifdef __powerpc64__
#include <altivec.h>

/* PowerPC VSX/Altivec operations */
void test_powerpc_multi_operand(void) {
    vector int a = vec_splats(1);
    vector int b = vec_splats(2);
    vector int c = vec_splats(3);
    vector int d = vec_splats(4);
    vector int e = vec_splats(5);
    vector int f = vec_splats(6);
    vector int g = vec_splats(7);
    vector int h = vec_splats(8);
    vector int i = vec_splats(9);
    vector int j = vec_splats(10);
    
    /* Complex permute and compute */
    vector int t1 = vec_add(a, b);
    vector int t2 = vec_add(t1, c);
    vector int t3 = vec_add(t2, d);
    vector int t4 = vec_add(t3, e);
    vector int t5 = vec_add(t4, f);
    vector int t6 = vec_add(t5, g);
    vector int t7 = vec_add(t6, h);
    vector int t8 = vec_add(t7, i);
    vector int result = vec_add(t8, j);
    
    volatile vector int sink = result;
    (void)sink;
}
#endif

/* ==================== Inline Assembly with Many Operands ==================== */

/* Force 11-operand inline assembly expansion */
int test_inline_asm_11_operands(void) {
    int64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    int64_t f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int64_t result;
    
    /* 11 operands: 1 output + 10 inputs = 11 total */
    asm volatile (
        "/* 11-operand test %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 + %10 */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    return (int)result;
}

/* Force 10-operand inline assembly expansion */
int test_inline_asm_10_operands(void) {
    int32_t a = 1, b = 2, c = 3, d = 4, e = 5;
    int32_t f = 6, g = 7, h = 8, i = 9, j = 10;
    int32_t result;
    
    /* 10 operands: 1 output + 9 inputs = 10 total */
    asm volatile (
        "/* 10-operand test */\n\t"
        "imul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    return result;
}

/* ==================== Built-in Functions with Many Arguments ==================== */

/* Atomic built-in with many parameters */
int test_atomic_builtin(void) {
    int64_t atomic_var = 0;
    int64_t expected = 0;
    int64_t desired = 42;
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which when expanded
     * might need additional operands for memory addresses */
    int result = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           weak, success_memorder, failure_memorder);
    
    return result ? (int)atomic_var : -1;
}

/* Complex expression that might combine into multi-operand instruction */
double test_complex_expression_10_operands(void) {
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0, e = 5.0;
    double f = 6.0, g = 7.0, h = 8.0, i = 9.0, j = 10.0;
    
    /* Expression with 10 operands that might be combined by the compiler */
    double result = a * b + c * d + e * f + g * h + i * j;
    
    /* More complex chain */
    result = result + a * c + b * d + e * g + f * h + i * j;
    
    return result;
}

/* Bit-field operations that might need many operands */
uint64_t test_bitfield_operations(void) {
    uint64_t value = 0x123456789ABCDEF0ULL;
    uint64_t mask1 = 0xFF00FF00FF00FF00ULL;
    uint64_t mask2 = 0x00FF00FF00FF00FFULL;
    uint64_t shift1 = 8, shift2 = 16, shift3 = 24, shift4 = 32;
    
    /* Complex bit manipulation that might be combined */
    uint64_t result = ((value & mask1) >> shift1) |
                      ((value & mask2) << shift1) |
                      ((value >> shift2) & mask1) |
                      ((value << shift2) & mask2) |
                      ((value >> shift3) & 0xFFFF) |
                      ((value << shift3) & 0xFFFF0000);
    
    return result;
}

/* ==================== Main Driver ==================== */

int main(void) {
    int total = 0;
    
    printf("Testing multi-operand instruction expansion...\n");
    
    /* Test architecture-specific intrinsics */
#ifdef __x86_64__
    test_avx512_multi_operand();
    test_avx512_masked();
    printf("x86_64 intrinsics tested\n");
#endif
    
#ifdef __aarch64__
    test_arm_multi_operand();
    printf("AArch64 intrinsics tested\n");
#endif
    
#ifdef __powerpc64__
    test_powerpc_multi_operand();
    printf("PowerPC intrinsics tested\n");
#endif
    
    /* Test inline assembly with many operands */
    total += test_inline_asm_11_operands();
    total += test_inline_asm_10_operands();
    
    /* Test built-in functions */
    total += test_atomic_builtin();
    
    /* Test complex expressions */
    double fp_result = test_complex_expression_10_operands();
    total += (int)fp_result;
    
    /* Test bitfield operations */
    total += (int)test_bitfield_operations();
    
    printf("Total result: %d\n", total);
    printf("Check RTL dumps for multi-operand expansion:\n");
    printf("  - Look for 'expand' dump file with case 10: or case 11: patterns\n");
    printf("  - Check 'combine' dump for instruction combination attempts\n");
    
    return total == 0 ? 0 : 1;
}
