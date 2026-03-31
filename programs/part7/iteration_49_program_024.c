/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_result = 0;

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
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(a, mask, idx, b);
    
    /* Use result to prevent optimization */
    g_result += _mm512_extract_epi64(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE intrinsic with multiple operands */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t mask = vdupq_n_u8(0x80);
    
    /* Complex table lookup/permutation */
    uint8x16x2_t tbl = {a, b};
    uint8x16_t indices = vdupq_n_u8(0);
    uint8x16_t result = vqtbl2q_u8(tbl, indices);
    
    g_result += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    int64_t op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    int64_t op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    int64_t result = 0;
    
    asm volatile (
        "/* 10-operand test */\n\t"
        "mov %[res], %[op1]\n\t"
        "add %[res], %[res], %[op2]\n\t"
        "add %[res], %[res], %[op3]\n\t"
        "add %[res], %[res], %[op4]\n\t"
        "add %[res], %[res], %[op5]\n\t"
        "add %[res], %[res], %[op6]\n\t"
        "add %[res], %[res], %[op7]\n\t"
        "add %[res], %[res], %[op8]\n\t"
        "add %[res], %[res], %[op9]\n\t"
        "add %[res], %[res], %[op10]"
        : [res] "=r" (result)
        : [op1] "r" (op1), [op2] "r" (op2), [op3] "r" (op3),
          [op4] "r" (op4), [op5] "r" (op5), [op6] "r" (op6),
          [op7] "r" (op7), [op8] "r" (op8), [op9] "r" (op9),
          [op10] "r" (op10)
        : "cc"
    );
    
    g_result += result;
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
    __mmask16 mask = 0xAAAA;
    int imm = 0x1F;
    
    /* This may expand to complex RTL with many operands */
    __m512i result = _mm512_mask3_fmadd_epi32(src1, src2, src3, mask);
    
    g_result += _mm512_extract_epi32(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM SVE intrinsic - often requires many operands */
    #ifdef __ARM_FEATURE_SVE
    #include <arm_sve.h>
    svint32_t a = svdup_s32(1);
    svint32_t b = svdup_s32(2);
    svint32_t c = svdup_s32(3);
    svbool_t pg = svptrue_b32();
    
    /* Complex SVE operation */
    svint32_t result = svmla_s32_z(pg, a, b, c);
    
    int32_t res_arr[svcntw()];
    svst1_s32(pg, res_arr, result);
    g_result += res_arr[0];
    #endif
    
#else
    /* Generic inline assembly with exactly 11 operands */
    int64_t op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    int64_t op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    int64_t result = 0;
    
    asm volatile (
        "/* 11-operand test */\n\t"
        "mov %[res], %[op1]\n\t"
        "imul %[res], %[res], %[op2]\n\t"
        "add %[res], %[res], %[op3]\n\t"
        "sub %[res], %[res], %[op4]\n\t"
        "add %[res], %[res], %[op5]\n\t"
        "sub %[res], %[res], %[op6]\n\t"
        "add %[res], %[res], %[op7]\n\t"
        "sub %[res], %[res], %[op8]\n\t"
        "add %[res], %[res], %[op9]\n\t"
        "sub %[res], %[res], %[op10]\n\t"
        "add %[res], %[res], %[op11]"
        : [res] "=r" (result)
        : [op1] "r" (op1), [op2] "r" (op2), [op3] "r" (op3),
          [op4] "r" (op4), [op5] "r" (op5), [op6] "r" (op6),
          [op7] "r" (op7), [op8] "r" (op8), [op9] "r" (op9),
          [op10] "r" (op10), [op11] "r" (op11)
        : "cc"
    );
    
    g_result += result;
#endif
}

/* Additional test using complex vector operations that may merge into multi-operand RTL */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Create a complex chain that might be optimized into a single multi-operand pattern */
    __m512i v1 = _mm512_set_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i v2 = _mm512_set_epi64(8, 7, 6, 5, 4, 3, 2, 1);
    __m512i v3 = _mm512_set_epi64(2, 3, 4, 5, 6, 7, 8, 9);
    __m512i v4 = _mm512_set_epi64(9, 8, 7, 6, 5, 4, 3, 2);
    
    /* Complex sequence that might be pattern-matched */
    __m512i t1 = _mm512_add_epi64(v1, v2);
    __m512i t2 = _mm512_sub_epi64(v3, v4);
    __m512i t3 = _mm512_mullo_epi64(t1, t2);
    __m512i mask = _mm512_set1_epi64(0xFFFFFFFF);
    __m512i result = _mm512_and_epi64(t3, mask);
    
    g_result += _mm512_extract_epi64(result, 0);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", g_result);
    printf("Test completed successfully.\n");
    
    return 0;
}
