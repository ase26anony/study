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
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    g_result += _mm512_extract_epi64(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON intrinsic with multiple operands */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex table lookup that may expand to multi-operand RTL */
    uint8x16_t result = vqtbl4q_u8(
        vcombine_u8x16_t(a, b, c, d),
        vdupq_n_u8(0)
    );
    
    g_result += vgetq_lane_u8(result, 0);
#endif

    /* Generic multi-operand inline assembly as fallback */
    __asm__ volatile (
        "# 10-operand pattern\n"
        "mov %0, %1\n"
        "add %0, %2\n"
        "sub %0, %3\n"
        "and %0, %4\n"
        "or  %0, %5\n"
        "xor %0, %6\n"
        "shl %0, %7\n"
        "shr %0, %8\n"
        "ror %0, %9\n"
        : "=r"(g_result)
        : "r"(g_result), "r"(1), "r"(2), "r"(0xFF), 
          "r"(0xAA), "r"(0x55), "r"(2), "r"(1), "r"(4)
        : "cc"
    );
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    /* Explicit 11-operand inline assembly */
    __asm__ volatile (
        "# 11-operand pattern\n"
        "mov %0, %1\n"
        "add %0, %2\n"
        "sub %0, %3\n"
        "and %0, %4\n"
        "or  %0, %5\n"
        "xor %0, %6\n"
        "shl %0, %7\n"
        "shr %0, %8\n"
        "ror %0, %9\n"
        "rol %0, %10\n"
        : "=r"(g_result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
#ifdef __x86_64__
    #ifdef __AVX512VL__
    #include <immintrin.h>
    /* Another AVX-512 intrinsic that may use many operands */
    __m256i src1 = _mm256_set1_epi32(1);
    __m256i src2 = _mm256_set1_epi32(2);
    __m256i src3 = _mm256_set1_epi32(3);
    __m256i mask = _mm256_set1_epi32(-1);
    
    /* Complex blend with multiple sources */
    __m256i result = _mm256_mask_blend_epi32(
        0x0F, src1, src2
    );
    
    g_result += _mm256_extract_epi32(result, 0);
    #endif
#endif
}

/* Additional test with vector operations that might combine */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Chain of operations that might be combined into multi-operand RTL */
    __m512i v1 = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    /* Complex expression that might expand to many operands */
    __m512i result = _mm512_add_epi32(
        _mm512_mullo_epi32(v1, v2),
        _mm512_sub_epi32(v3, v4)
    );
    
    /* Masked store with multiple parameters */
    __mmask16 store_mask = 0xFFFF;
    int32_t array[16] __attribute__((aligned(64)));
    _mm512_mask_store_epi32(array, store_mask, result);
    
    g_result += array[0];
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", g_result);
    printf("Test completed.\n");
    
    return g_result == 0 ? 0 : 1;
}
