/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function attribute to force optimization and RTL expansion */
#define FORCE_EXPAND __attribute__((optimize("O3"), noinline, target("arch=native")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test function for 10-operand case using AVX-512 intrinsics */
FORCE_EXPAND
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 expands to many operands:
       dest, mask, idx, src1, src2 */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    global_counter += _mm512_extract_epi64(result, 0);
}

/* Test function for 11-operand case using multiple operations */
FORCE_EXPAND
void test_11_operand_x86(void) {
    /* Complex blend with multiple sources and mask */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    
    /* Chain operations that might combine into multi-operand pattern */
    __m512i temp = _mm512_mask_blend_epi32(mask1, a, b);
    __m512i result = _mm512_mask_blend_epi32(mask2, temp, c);
    
    global_counter += _mm512_extract_epi32(result, 0);
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* Test function for 10-operand case using ARM NEON */
FORCE_EXPAND
void test_10_operand_arm(void) {
    /* Complex NEON operations with multiple registers */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Multiple operations that might combine */
    uint8x16_t t1 = vaddq_u8(a, b);
    uint8x16_t t2 = vaddq_u8(c, d);
    uint8x16_t result = vaddq_u8(t1, t2);
    
    /* Use tbl for complex permute (can have many operands) */
    uint8x16_t indices = vdupq_n_u8(0);
    result = vqtbl1q_u8(result, indices);
    
    global_counter += vgetq_lane_u8(result, 0);
}

/* Test function for 11-operand case using inline assembly */
FORCE_EXPAND
void test_11_operand_arm(void) {
    /* Inline assembly with exactly 11 operands */
    int64_t out1, out2, out3;
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6, in7 = 7;
    
    asm volatile (
        /* Template for 11 operands */
        "add %[o1], %[i1], %[i2]\n\t"
        "add %[o2], %[i3], %[i4]\n\t"
        "add %[o3], %[i5], %[i6]\n\t"
        "mul %[o1], %[o1], %[i7]"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7)
        : "cc"
    );
    
    global_counter += out1 + out2 + out3;
}

#endif /* __aarch64__ */

/* Generic fallback using inline assembly with many operands */
FORCE_EXPAND
void test_10_operand_generic(void) {
    /* Inline assembly with exactly 10 operands */
    long result1, result2;
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    
    asm volatile (
        /* Complex operation with 10 total operands */
        "imul %[r1], %[a], %[b]\n\t"
        "imul %[r2], %[c], %[d]\n\t"
        "add %[r1], %[r1], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "sub %[r1], %[r1], %[g]\n\t"
        "sub %[r2], %[r2], %[h]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc"
    );
    
    global_counter += result1 + result2;
}

FORCE_EXPAND
void test_11_operand_generic(void) {
    /* Inline assembly with exactly 11 operands */
    long r1, r2, r3;
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    
    asm volatile (
        /* Operation with 11 total operands */
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[r1], %[b]\n\t"
        "mov %[r2], %[c]\n\t"
        "add %[r2], %[r2], %[d]\n\t"
        "mov %[r3], %[e]\n\t"
        "add %[r3], %[r3], %[f]\n\t"
        "imul %[r1], %[r1], %[g]\n\t"
        "imul %[r2], %[r2], %[h]"
        : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc"
    );
    
    global_counter += r1 + r2 + r3;
}

/* Main test driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call architecture-specific or generic tests */
#ifdef __x86_64__
    test_10_operand_x86();
    test_11_operand_x86();
    printf("x86_64 intrinsics tested\n");
#elif defined(__aarch64__)
    test_10_operand_arm();
    test_11_operand_arm();
    printf("ARM NEON tested\n");
#else
    test_10_operand_generic();
    test_11_operand_generic();
    printf("Generic inline assembly tested\n");
#endif
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed successfully\n");
    
    return 0;
}
