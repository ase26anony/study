/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 intrinsic that often expands to multi-operand RTL */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i a = _mm512_set1_epi64(1);
    __m512i b = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 k = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(a, k, idx, b);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON intrinsic with multiple operands */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t mask = vdupq_n_u8(0x80);
    
    /* Complex table lookup operation */
    uint8x16_t result = vqtbl1q_u8(a, b);
    result = vbslq_u8(mask, a, result);
    
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    asm volatile (
        "/* 10-operand assembly template */\n\t"
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
        : [res] "=r" (result)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Complex AVX-512 masked operation with immediate */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi32(1);
    __m512i src2 = _mm512_set1_epi32(2);
    __m512i src3 = _mm512_set1_epi32(3);
    __mmask16 k1 = 0xAAAA;
    __mmask16 k2 = 0x5555;
    int imm = 5;
    
    /* Multi-operation sequence that might combine */
    __m512i t1 = _mm512_mask_add_epi32(src1, k1, src2, src3);
    __m512i t2 = _mm512_mask_slli_epi32(src2, k2, t1, imm);
    __m512i result = _mm512_mask_blend_epi32(k1, t1, t2);
    
    global_counter += _mm512_extract_epi32(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM SVE-like pattern with multiple registers */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    uint8x16_t e = vdupq_n_u8(5);
    
    /* Complex sequence that might generate multi-operand RTL */
    uint8x16_t t1 = vaddq_u8(a, b);
    uint8x16_t t2 = vaddq_u8(c, d);
    uint8x16_t t3 = vaddq_u8(t1, t2);
    uint8x16_t result = vaddq_u8(t3, e);
    
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long result;
    
    asm volatile (
        "/* 11-operand assembly template */\n\t"
        "mov %[res], %[a1]\n\t"
        "add %[res], %[a2]\n\t"
        "add %[res], %[a3]\n\t"
        "add %[res], %[a4]\n\t"
        "add %[res], %[a5]\n\t"
        "add %[res], %[a6]\n\t"
        "add %[res], %[a7]\n\t"
        "add %[res], %[a8]\n\t"
        "add %[res], %[a9]\n\t"
        "add %[res], %[a10]\n\t"
        "add %[res], %[a11]"
        : [res] "=r" (result)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10), [a11] "r" (op11)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test with complex vector operations */
__attribute__((noinline, optimize("O3")))
void test_complex_vector_ops(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    /* Create a pattern that might generate 10+ operand RTL */
    __m512i v1 = _mm512_set_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i v2 = _mm512_set_epi64(8, 7, 6, 5, 4, 3, 2, 1);
    __m512i v3 = _mm512_set_epi64(2, 3, 4, 5, 6, 7, 8, 9);
    __m512i v4 = _mm512_set_epi64(9, 8, 7, 6, 5, 4, 3, 2);
    
    __mmask8 m1 = 0xAA;
    __mmask8 m2 = 0x55;
    
    /* Complex sequence that might be combined */
    __m512i r1 = _mm512_mask_add_epi64(v1, m1, v2, v3);
    __m512i r2 = _mm512_mask_sub_epi64(v4, m2, v1, v2);
    __m512i result = _mm512_mask_blend_epi64(m1, r1, r2);
    
    global_counter += _mm512_extract_epi64(result, 0);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_complex_vector_ops();
    }
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return global_counter > 0 ? 0 : 1;
}
