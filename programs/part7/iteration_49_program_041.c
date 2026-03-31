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
    
    /* Complex permute operation with many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    g_result += _mm512_extract_epi64(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE intrinsic with many operands */
    #include <arm_neon.h>
    
    /* Complex vector operation chain */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Multiple operations that might combine */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    uint8x16_t r4 = vmulq_u8(r3, a);
    
    /* Extract to prevent optimization */
    g_result += vgetq_lane_u8(r4, 0);
#endif
    
    /* Generic multi-operand inline assembly as fallback */
    __asm__ volatile (
        "# 10-operand asm block\n"
        "mov %0, %1\n"
        "add %0, %2\n"
        "sub %0, %3\n"
        "and %0, %4\n"
        "or  %0, %5\n"
        "xor %0, %6\n"
        "shl %0, %7\n"
        "shr %0, %8\n"
        : "=r"(g_result)
        : "r"(g_result), "r"(1), "r"(2), "r"(3), "r"(4), "r"(5), "r"(6), "r"(7), "r"(8)
        : "cc"
    );
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
    /* Complex inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    __asm__ volatile (
        "# 11-operand asm block\n"
        /* Complex computation using all 11 operands */
        "mov %0, %1\n"           /* result = a */
        "imul %0, %2\n"          /* result *= b */
        "add %0, %3\n"           /* result += c */
        "sub %0, %4\n"           /* result -= d */
        "and %0, %5\n"           /* result &= e */
        "or  %0, %6\n"           /* result |= f */
        "xor %0, %7\n"           /* result ^= g */
        "shl %0, %8\n"           /* result <<= h */
        "shr %0, %9\n"           /* result >>= i */
        "add %0, %10\n"          /* result += j */
        "sub %0, %11\n"          /* result -= k */
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    
    g_result += result;
    
#ifdef __x86_64__
    #ifdef __AVX512VL__
    #include <immintrin.h>
    
    /* Another AVX-512 operation with many parameters */
    __m256i v1 = _mm256_set1_epi32(1);
    __m256i v2 = _mm256_set1_epi32(2);
    __m256i v3 = _mm256_set1_epi32(3);
    __m256i mask_vec = _mm256_set1_epi32(0xFFFFFFFF);
    
    /* Complex blend operation chain */
    __m256i r1 = _mm256_blend_epi32(v1, v2, 0xCC);
    __m256i r2 = _mm256_blend_epi32(v3, r1, 0xAA);
    __m256i r3 = _mm256_slli_epi32(r2, 2);
    __m256i r4 = _mm256_srli_epi32(r3, 1);
    
    g_result += _mm256_extract_epi32(r4, 0);
    #endif
#endif
}

/* Additional test with vector operations that might combine */
__attribute__((noinline, optimize("O3")))
void test_vector_combine(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Complex vector operation that might expand to multi-operand RTL */
    __m512i v1 = _mm512_set_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i v2 = _mm512_set_epi64(8, 7, 6, 5, 4, 3, 2, 1);
    __m512i v3 = _mm512_set_epi64(2, 3, 4, 5, 6, 7, 8, 9);
    __mmask8 k1 = 0xF0;
    __mmask8 k2 = 0x0F;
    
    /* Chain of operations that might be combined */
    __m512i t1 = _mm512_mask_add_epi64(v1, k1, v2, v3);
    __m512i t2 = _mm512_mask_sub_epi64(t1, k2, v3, v1);
    __m512i t3 = _mm512_mask_mul_epi64(t2, k1, t1, t2);
    
    g_result += _mm512_extract_epi64(t3, 0);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_combine();
    
    printf("Result: %d\n", g_result);
    printf("Test completed.\n");
    
    return g_result == 0 ? 0 : 1;
}
