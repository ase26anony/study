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
    global_counter += _mm512_reduce_add_epi64(result);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON intrinsic with multiple operands */
    #include <arm_neon.h>
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    
    /* Complex table lookup with multiple registers */
    uint8x16_t result = vqtbl4q_u8(
        (uint8x16x4_t){v1, v2, v3, v4},
        vdupq_n_u8(0)
    );
    
    /* Use result */
    global_counter += vgetq_lane_u8(result, 0);
#endif
    
    /* Generic fallback: inline assembly with 10 operands */
    asm volatile(
        "# 10-operand asm block\n"
        "mov %0, %1\n"
        "add %0, %2\n"
        "sub %0, %3\n"
        "and %0, %4\n"
        "or  %0, %5\n"
        "xor %0, %6\n"
        "shl %0, %7\n"
        "shr %0, %8\n"
        "ror %0, %9\n"
        : "=r"(global_counter)
        : "r"(global_counter), "r"(1), "r"(2), "r"(0xFF), 
          "r"(0xAA), "r"(0x55), "r"(1), "r"(2), "r"(3)
        : "cc"
    );
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* AVX-512 masked gather with multiple parameters */
    #ifdef __AVX512F__
    #include <immintrin.h>
    long long base[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    __m512i vindex = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 k = 0xFF;
    int scale = 1;
    
    __m512i result = _mm512_mask_i64gather_epi64(
        _mm512_setzero_si512(),  // src
        k,                       // mask
        vindex,                  // indices
        base,                    // base
        scale                    // scale
    );
    
    global_counter += _mm512_reduce_add_epi64(result);
    #endif
    
#elif defined(__aarch64__)
    /* ARM SVE-like multiple operand operation (if available) */
    #include <arm_neon.h>
    /* Complex multiple register operation */
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    uint8x16_t v5 = vdupq_n_u8(5);
    
    /* Multiple operations chained */
    uint8x16_t t1 = vaddq_u8(v1, v2);
    uint8x16_t t2 = vaddq_u8(v3, v4);
    uint8x16_t result = vaddq_u8(t1, t2);
    result = vaddq_u8(result, v5);
    
    global_counter += vgetq_lane_u8(result, 0);
#endif
    
    /* Generic fallback: inline assembly with exactly 11 operands */
    int temp1 = global_counter;
    int temp2 = 1;
    int temp3 = 2;
    int temp4 = 3;
    int temp5 = 4;
    int temp6 = 5;
    int temp7 = 6;
    int temp8 = 7;
    int temp9 = 8;
    int temp10 = 9;
    int temp11 = 10;
    
    asm volatile(
        "# 11-operand asm block\n"
        "mov %0, %1\n"
        "add %0, %2\n"
        "sub %0, %3\n"
        "mul %0, %4\n"
        "div %0, %5\n"
        "and %0, %6\n"
        "or  %0, %7\n"
        "xor %0, %8\n"
        "shl %0, %9\n"
        "shr %0, %10\n"
        "ror %0, %11\n"
        : "=r"(global_counter)
        : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4),
          "r"(temp5), "r"(temp6), "r"(temp7), "r"(temp8),
          "r"(temp9), "r"(temp10), "r"(temp11)
        : "cc"
    );
}

/* Additional test with complex vector operations */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    /* Complex chain that might generate multi-operand RTL */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __m512i e = _mm512_set1_epi32(5);
    
    /* Chain of operations that might be combined */
    __m512i r1 = _mm512_add_epi32(a, b);
    __m512i r2 = _mm512_sub_epi32(c, d);
    __m512i r3 = _mm512_mullo_epi32(r1, r2);
    __m512i r4 = _mm512_add_epi32(r3, e);
    
    /* Masked operation with many parameters */
    __mmask16 m = 0xAAAA;
    __m512i src = _mm512_setzero_si512();
    __m512i result = _mm512_mask_add_epi32(src, m, r4, a);
    
    global_counter += _mm512_reduce_add_epi32(result);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
