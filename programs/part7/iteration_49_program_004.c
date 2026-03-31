/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile_zero = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* x86_64: Use AVX-512 masked permute with multiple operands */
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic expands to complex RTL with many operands:
       dest, mask, idx, src1, src2, plus implicit control operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    int64_t* res_arr = (int64_t*)&result;
    if (res_arr[0] != 0) {
        printf("10-op x86 result: %ld\n", res_arr[0] + g_volatile_zero);
    }
    
#elif defined(__aarch64__)
    /* ARM NEON: Complex vector operations with multiple lanes */
    #include <arm_neon.h>
    
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex sequence that may combine into multi-operand pattern */
    uint8x16_t t1 = vaddq_u8(a, b);
    uint8x16_t t2 = vaddq_u8(c, d);
    uint8x16_t t3 = vaddq_u8(t1, t2);
    uint8x16_t result = vrev64q_u8(t3);
    
    /* Use result */
    uint8_t res = vgetq_lane_u8(result, 0);
    if (res != 0) {
        printf("10-op ARM result: %d\n", res + g_volatile_zero);
    }
    
#else
    /* Generic: Inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        /* Complex multi-operand pattern */
        "add %[res], %[a], %[b]\n\t"
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
    
    printf("10-op generic result: %d\n", result + g_volatile_zero);
#endif
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* x86_64: Complex AVX-512 masked operation with immediate */
    #include <immintrin.h>
    
    __m512i src = _mm512_set1_epi32(1);
    __m512i a = _mm512_set1_epi32(2);
    __m512i b = _mm512_set1_epi32(3);
    __mmask16 mask = 0xFFFF;
    
    /* This may expand to 11 operands including immediate control */
    __m512i result = _mm512_maskz_fmadd_epi32(mask, src, a, b);
    
    /* Use result */
    int32_t* res_arr = (int32_t*)&result;
    if (res_arr[0] != 0) {
        printf("11-op x86 result: %d\n", res_arr[0] + g_volatile_zero);
    }
    
#elif defined(__aarch64__)
    /* ARM: Complex vector permute with multiple source registers */
    #include <arm_neon.h>
    
    uint8x16_t v0 = vdupq_n_u8(0);
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    
    /* Complex sequence that may require many operands */
    uint8x16_t t1 = vaddq_u8(v0, v1);
    uint8x16_t t2 = vaddq_u8(v2, v3);
    uint8x16_t t3 = vaddq_u8(t1, t2);
    uint8x16_t t4 = vaddq_u8(t3, v4);
    uint8x16_t result = vrev64q_u8(t4);
    
    /* Use result */
    uint8_t res = vgetq_lane_u8(result, 0);
    if (res != 0) {
        printf("11-op ARM result: %d\n", res + g_volatile_zero);
    }
    
#else
    /* Generic: Inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
        /* 11-operand pattern */
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
    
    printf("11-op generic result: %d\n", result + g_volatile_zero);
#endif
}

/* Additional test using OpenMP SIMD pragmas for pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __x86_64__
    #include <immintrin.h>
    
    /* Complex vector operation chain that might merge into multi-operand RTL */
    __m512i v1 = _mm512_set1_epi32(1);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    __m512i v5 = _mm512_set1_epi32(5);
    
    /* Chain of operations that could be combined */
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_add_epi32(v3, v4);
    __m512i r3 = _mm512_add_epi32(r1, r2);
    __m512i result = _mm512_add_epi32(r3, v5);
    
    /* Use result */
    int32_t* res = (int32_t*)&result;
    if (res[0] != 0) {
        printf("Vector chain result: %d\n", res[0] + g_volatile_zero);
    }
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call both test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    /* Ensure functions aren't optimized away */
    if (g_volatile_zero) {
        test_10_operand();
        test_11_operand();
    }
    
    printf("Test completed.\n");
    return 0;
}
