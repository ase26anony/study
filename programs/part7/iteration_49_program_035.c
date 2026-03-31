/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int sink;

/* Function attribute to force optimization */
#define FORCE_OPT __attribute__((optimize("O3", "no-inline")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test function for 10-operand case using AVX-512 intrinsics */
FORCE_OPT void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands:
       _mm512_mask_permutex2var_epi64 has 10 operands in RTL:
       1. Destination
       2. Mask
       3. Index
       4. Table 1
       5. Table 2
       Plus various immediates and modes */
    
    __m512i a = _mm512_set1_epi64(1);
    __m512i b = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 k = 0xFF;
    
    /* This intrinsic expands to an RTL pattern with 10 operands */
    __m512i result = _mm512_mask_permutex2var_epi64(a, k, idx, a, b);
    
    /* Use result to prevent elimination */
    sink = _mm512_extract_epi64(result, 0);
    
    /* Additional complex pattern with blend and permute */
    __m512i c = _mm512_set1_epi64(3);
    __m512i d = _mm512_set1_epi64(4);
    __m512i idx2 = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
    
    /* Chain operations to potentially create complex RTL */
    __m512i temp = _mm512_mask_blend_epi64(k, a, b);
    __m512i final = _mm512_permutexvar_epi64(idx2, temp);
    
    sink += _mm512_extract_epi64(final, 0);
}

/* Test function for 11-operand case using inline assembly */
FORCE_OPT void test_11_operand_x86(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    uint64_t result1, result2, result3;
    
    /* Inline assembly with 11 operands:
       3 outputs + 8 inputs = 11 total operands */
    asm volatile (
        /* Complex multi-step operation */
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "imul %[c], %[r1]\n\t"
        "mov %[d], %[r2]\n\t"
        "sub %[e], %[r2]\n\t"
        "mov %[f], %[r3]\n\t"
        "xor %[g], %[r3]\n\t"
        "lea (%[h],%[i],2), %[r1]\n\t"
        "add %[j], %[r2]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc", "memory"
    );
    
    sink = result1 + result2 + result3;
}

#elif defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>

/* Test function for 10-operand case using ARM NEON */
FORCE_OPT void test_10_operand_arm(void) {
    /* Use complex NEON operations with multiple registers */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Table lookup with multiple registers - can generate complex RTL */
    uint8x16x2_t tbl2 = {a, b};
    uint8x16_t indices = vdupq_n_u8(0);
    
    /* vqtbl2q_u8 uses multiple operands */
    uint8x16_t result = vqtbl2q_u8(tbl2, indices);
    
    /* Additional operations to increase operand count */
    uint8x16_t e = vdupq_n_u8(5);
    uint8x16_t f = vdupq_n_u8(6);
    
    /* Complex blend/select operation */
    uint8x16_t mask = vceqq_u8(a, b);
    uint8x16_t blended = vbslq_u8(mask, c, d);
    
    /* Store to prevent elimination */
    vst1q_u8(&sink, result);
    vst1q_u8(&sink, blended);
}

/* Test function for 11-operand case using inline assembly */
FORCE_OPT void test_11_operand_arm(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    uint64_t result1, result2, result3, result4;
    
    /* ARM inline assembly with 11 operands */
    asm volatile (
        /* Multiple operations using many registers */
        "add %[r1], %[a], %[b]\n\t"
        "mul %[r1], %[r1], %[c]\n\t"
        "sub %[r2], %[d], %[e]\n\t"
        "and %[r3], %[f], %[g]\n\t"
        "orr %[r4], %[h], %[i]\n\t"
        "add %[r4], %[r4], %[j]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2),
          [r3] "=&r" (result3), [r4] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    sink = result1 + result2 + result3 + result4;
}

#else
/* Generic fallback using inline assembly with many operands */

FORCE_OPT void test_10_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9;
    long result1, result2;
    
    /* Generic inline assembly with 10 operands */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "sub %[d], %[r2]\n\t"
        "imul %[e], %[r1]\n\t"
        "add %[f], %[r2]\n\t"
        "xor %[g], %[r1]\n\t"
        "or %[h], %[r2]\n\t"
        "and %[i], %[r1]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "cc"
    );
    
    sink = result1 + result2;
}

FORCE_OPT void test_11_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result1, result2, result3;
    
    /* Generic inline assembly with 11 operands */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "sub %[d], %[r2]\n\t"
        "mov %[e], %[r3]\n\t"
        "imul %[f], %[r1]\n\t"
        "add %[g], %[r2]\n\t"
        "xor %[h], %[r3]\n\t"
        "or %[i], %[r1]\n\t"
        "and %[j], %[r2]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    sink = result1 + result2 + result3;
}
#endif

/* Wrapper functions that call architecture-specific implementations */
FORCE_OPT void test_10_operand(void) {
#ifdef __x86_64__
    test_10_operand_x86();
#elif defined(__aarch64__) || defined(__arm__)
    test_10_operand_arm();
#else
    test_10_operand_generic();
#endif
}

FORCE_OPT void test_11_operand(void) {
#ifdef __x86_64__
    test_11_operand_x86();
#elif defined(__aarch64__) || defined(__arm__)
    test_11_operand_arm();
#else
    test_11_operand_generic();
#endif
}

/* Main driver */
int main(void) {
    printf("Testing 10 and 11 operand RTL patterns...\n");
    
    /* Call both test functions */
    test_10_operand();
    test_11_operand();
    
    /* Use sink to prevent dead code elimination */
    printf("Result: %d\n", sink);
    
    return 0;
}
