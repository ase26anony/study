/* test_optabs_10_11_operands.c
 * This program aims to trigger the 10 and 11 operand cases in optabs.cc
 * during RTL expansion phase.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* ============================================
 * 10-OPERAND TEST CASE
 * ============================================ */

#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noinline, optimize("O3")))
void test_10_operand_avx512(void) {
    /* Complex AVX-512 operation with many operands:
     * _mm512_mask_permutex2var_epi64 has 4 register operands + 1 immediate
     * During RTL expansion, this may become 10 operands when including
     * memory addressing modes and temporaries.
     */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Chain multiple operations to encourage pattern merging */
    result = _mm512_add_epi64(result, _mm512_set1_epi64(1));
    result = _mm512_maskz_compress_epi64(mask, result);
    
    /* Use result to prevent elimination */
    global_counter += _mm512_reduce_add_epi64(result);
}
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noinline, optimize("O3")))
void test_10_operand_neon(void) {
    /* Complex NEON operations with multiple registers and immediates */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    
    /* Multiple table lookups with many operands */
    uint8x16x2_t tbl2 = {a, b};
    uint8x16_t result = vqtbl2q_u8(tbl2, c);
    
    /* Additional operations to increase operand count in combined RTL */
    result = vaddq_u8(result, vdupq_n_u8(1));
    result = vshlq_u8(result, vdupq_n_u8(2));
    
    /* Use result */
    uint8_t temp[16];
    vst1q_u8(temp, result);
    global_counter += temp[0];
}
#endif

/* Generic fallback using inline assembly with exactly 10 operands */
__attribute__((noinline, optimize("O3")))
void test_10_operand_generic(void) {
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10;
    long result;
    
    /* Initialize values */
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10;
    
    /* Inline asm with 10 explicit operands + clobbers
     * This should generate RTL with 10 operands during expansion
     */
    asm volatile (
        "/* 10-operand test */\n\t"
        "mov %[res], %[a1]\n\t"
        "add %[res], %[a2]\n\t"
        "add %[res], %[a3]\n\t"
        "add %[res], %[a4]\n\t"
        "add %[res], %[a5]\n\t"
        "add %[res], %[a6]\n\t"
        "add %[res], %[a7]\n\t"
        "add %[res], %[a8]\n\t"
        "add %[res], %[a9]\n\t"
        "add %[res], %[a10]"
        : [res] "=&r" (result)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    global_counter += result;
}

/* ============================================
 * 11-OPERAND TEST CASE  
 * ============================================ */

#ifdef __AVX512F__
__attribute__((noinline, optimize("O3")))
void test_11_operand_avx512(void) {
    /* AVX-512 masked gather with multiple operands:
     * base, scale, index, mask, src + various addressing components
     * can expand to 11 operands in RTL
     */
    long base[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    __m512i vindex = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i src = _mm512_set1_epi64(100);
    __mmask8 mask = 0xFF;
    int scale = 8;
    
    __m512i result = _mm512_mask_i64gather_epi64(src, mask, vindex, 
                                                 base, scale);
    
    /* Additional operation to ensure pattern complexity */
    result = _mm512_mask_expand_epi64(result, mask, result);
    
    global_counter += _mm512_reduce_add_epi64(result);
}
#endif

/* Generic inline assembly with exactly 11 operands */
__attribute__((noinline, optimize("O3")))
void test_11_operand_generic(void) {
    long ops[11];
    long result;
    
    /* Initialize all 11 operands */
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* Inline asm with 11 explicit input operands
     * This should directly trigger the 11-operand case in optabs.cc
     */
    asm volatile (
        "/* 11-operand test */\n\t"
        "mov %[res], %[a1]\n\t"
        "imul %[res], %[a2]\n\t"
        "add %[res], %[a3]\n\t"
        "sub %[res], %[a4]\n\t"
        "add %[res], %[a5]\n\t"
        "sub %[res], %[a6]\n\t"
        "add %[res], %[a7]\n\t"
        "sub %[res], %[a8]\n\t"
        "add %[res], %[a9]\n\t"
        "sub %[res], %[a10]\n\t"
        "add %[res], %[a11]"
        : [res] "=&r" (result)
        : [a1] "r" (ops[0]), [a2] "r" (ops[1]), [a3] "r" (ops[2]),
          [a4] "r" (ops[3]), [a5] "r" (ops[4]), [a6] "r" (ops[5]),
          [a7] "r" (ops[6]), [a8] "r" (ops[7]), [a9] "r" (ops[8]),
          [a10] "r" (ops[9]), [a11] "r" (ops[10])
        : "cc"
    );
    
    global_counter += result;
}

/* ============================================
 * MAIN TEST DRIVER
 * ============================================ */

void test_10_operand(void) {
#if defined(__AVX512F__)
    test_10_operand_avx512();
#elif defined(__ARM_NEON)
    test_10_operand_neon();
#else
    test_10_operand_generic();
#endif
}

void test_11_operand(void) {
#if defined(__AVX512F__)
    test_11_operand_avx512();
#else
    test_11_operand_generic();
#endif
}

int main(void) {
    printf("Testing 10 and 11 operand RTL patterns...\n");
    
    /* Call both test functions multiple times to ensure
     * they're not optimized away and to hit different
     * code paths in the compiler */
    for (int i = 0; i < 3; i++) {
        test_10_operand();
        test_11_operand();
    }
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return global_counter == 0 ? 0 : 1;
}
