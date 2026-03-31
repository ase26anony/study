/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* x86_64 AVX-512 intrinsics that often expand to multi-operand patterns */
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON intrinsics for complex operations */
    #include <arm_neon.h>
    
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex table lookup operations */
    uint8x16_t result = vqtbl4q_u8(
        (uint8x16x4_t){a, b, c, d},
        vdupq_n_u8(0)
    );
    
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    asm volatile (
        "/* 10-operand test */\n\t"
        "mov %[res], %[a1]\n\t"
        "add %[res], %[res], %[a2]\n\t"
        "add %[res], %[res], %[a3]\n\t"
        "add %[res], %[res], %[a4]\n\t"
        "add %[res], %[res], %[a5]\n\t"
        "add %[res], %[res], %[a6]\n\t"
        "add %[res], %[res], %[a7]\n\t"
        "add %[res], %[res], %[a8]\n\t"
        "add %[res], %[res], %[a9]\n\t"
        "add %[res], %[res], %[a10]"
        : [res] "=&r" (result)
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
    /* x86_64 complex inline assembly with 11 operands */
    long a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    long a6 = 6, a7 = 7, a8 = 8, a9 = 9, a10 = 10, a11 = 11;
    long result1, result2;
    
    asm volatile (
        "/* 11-operand test for x86 */\n\t"
        "mov %[r1], %[v1]\n\t"
        "mov %[r2], %[v2]\n\t"
        "add %[r1], %[r1], %[v3]\n\t"
        "add %[r2], %[r2], %[v4]\n\t"
        "imul %[r1], %[r1], %[v5]\n\t"
        "imul %[r2], %[r2], %[v6]\n\t"
        "add %[r1], %[r1], %[v7]\n\t"
        "add %[r2], %[r2], %[v8]\n\t"
        "sub %[r1], %[r1], %[v9]\n\t"
        "sub %[r2], %[r2], %[v10]\n\t"
        "add %[r1], %[r1], %[v11]\n\t"
        "add %[r1], %[r1], %[r2]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [v1] "r" (a1), [v2] "r" (a2), [v3] "r" (a3),
          [v4] "r" (a4), [v5] "r" (a5), [v6] "r" (a6),
          [v7] "r" (a7), [v8] "r" (a8), [v9] "r" (a9),
          [v10] "r" (a10), [v11] "r" (a11)
        : "cc"
    );
    
    global_counter += result1 + result2;
    
#elif defined(__aarch64__)
    /* ARM inline assembly with 11 operands */
    #include <arm_neon.h>
    
    uint64x2_t v1 = vdupq_n_u64(1);
    uint64x2_t v2 = vdupq_n_u64(2);
    uint64x2_t v3 = vdupq_n_u64(3);
    uint64x2_t v4 = vdupq_n_u64(4);
    uint64x2_t v5 = vdupq_n_u64(5);
    uint64x2_t result;
    
    /* Complex vector operation that may expand to many operands */
    asm volatile (
        "/* 11-operand NEON test */\n\t"
        "add %0.2d, %1.2d, %2.2d\n\t"
        "add %0.2d, %0.2d, %3.2d\n\t"
        "add %0.2d, %0.2d, %4.2d\n\t"
        "add %0.2d, %0.2d, %5.2d"
        : "=w" (result)
        : "w" (v1), "w" (v2), "w" (v3), "w" (v4), "w" (v5),
          "0" (result), "1" (v1), "2" (v2), "3" (v3), "4" (v4)
        : /* No clobbers */
    );
    
    global_counter += vgetq_lane_u64(result, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    long vals[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    long result = 0;
    
    asm volatile (
        "/* Generic 11-operand test */\n\t"
        "mov %[res], %[v1]\n\t"
        "add %[res], %[res], %[v2]\n\t"
        "add %[res], %[res], %[v3]\n\t"
        "add %[res], %[res], %[v4]\n\t"
        "add %[res], %[res], %[v5]\n\t"
        "add %[res], %[res], %[v6]\n\t"
        "add %[res], %[res], %[v7]\n\t"
        "add %[res], %[res], %[v8]\n\t"
        "add %[res], %[res], %[v9]\n\t"
        "add %[res], %[res], %[v10]\n\t"
        "add %[res], %[res], %[v11]"
        : [res] "=&r" (result)
        : [v1] "r" (vals[0]), [v2] "r" (vals[1]), [v3] "r" (vals[2]),
          [v4] "r" (vals[3]), [v5] "r" (vals[4]), [v6] "r" (vals[5]),
          [v7] "r" (vals[6]), [v8] "r" (vals[7]), [v9] "r" (vals[8]),
          [v10] "r" (vals[9]), [v11] "r" (vals[10])
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test using vector operations that might combine into multi-operand patterns */
__attribute__((noinline, optimize("O3")))
void test_vector_combine(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Complex sequence that might be combined into a single multi-operand pattern */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_mullo_epi32(r1, v3);
    __m512i r3 = _mm512_sub_epi32(r2, v4);
    __m512i r4 = _mm512_slli_epi32(r3, 2);
    
    /* Use masked store which has many operands */
    __mmask16 mask = 0xAAAA;
    _mm512_mask_storeu_epi32(&global_counter, mask, r4);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call both test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_vector_combine();
    }
    
    printf("Result: %d\n", global_counter);
    return global_counter > 0 ? 0 : 1;
}
