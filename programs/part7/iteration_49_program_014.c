/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* ==================== x86_64 AVX-512 specific code ==================== */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function to trigger 10-operand case using AVX-512 intrinsics */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 expands to many operands:
       - dest
       - mask
       - idx
       - src1
       - src2
       Plus various immediates and registers */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
}

/* Function to trigger 11-operand case using inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_x86(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint64_t result1, result2, result3, result4;
    
    /* Inline assembly with 11 operands:
       5 inputs, 4 outputs, 2 clobbers = 11 total */
    asm volatile (
        /* Complex operation with many operands */
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "imul %[d], %[r2]\n\t"
        "mov %[e], %[r3]\n\t"
        "sub %[f], %[r3]\n\t"
        "mov %[g], %[r4]\n\t"
        "xor %[h], %[r4]\n\t"
        /* Use remaining inputs */
        "add %[i], %[r1]\n\t"
        "add %[j], %[r2]\n\t"
        "add %[k], %[r3]\n\t"
        : [r1] "=&r" (result1),
          [r2] "=&r" (result2),
          [r3] "=&r" (result3),
          [r4] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc", "memory"
    );
    
    global_counter += result1 + result2 + result3 + result4;
}

/* ==================== AArch64/ARM specific code ==================== */
#elif defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>

/* Function to trigger 10-operand case using NEON intrinsics */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_arm(void) {
    /* Complex NEON operation chain */
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    
    /* Multiple operations that might combine into complex pattern */
    uint8x16_t r1 = vaddq_u8(v1, v2);
    uint8x16_t r2 = vaddq_u8(v3, v4);
    uint8x16_t r3 = vmulq_u8(r1, r2);
    
    /* Table lookup with many operands */
    uint8x16_t table_result = vqtbl1q_u8(r3, vcreate_u8(0x0706050403020100));
    
    /* Extract and add to prevent optimization */
    uint8_t lane = vgetq_lane_u8(table_result, 0);
    global_counter += lane;
}

/* Function to trigger 11-operand case using inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_arm(void) {
    uint32_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint32_t r1, r2, r3, r4;
    
    /* Inline assembly with 11 operands for ARM */
    asm volatile (
        /* Complex multi-operand sequence */
        "add %[r1], %[a], %[b]\n\t"
        "mul %[r2], %[c], %[d]\n\t"
        "sub %[r3], %[e], %[f]\n\t"
        "eor %[r4], %[g], %[h]\n\t"
        /* Use remaining operands */
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r2], %[r2], %[j]\n\t"
        "add %[r3], %[r3], %[k]\n\t"
        : [r1] "=&r" (r1),
          [r2] "=&r" (r2),
          [r3] "=&r" (r3),
          [r4] "=&r" (r4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += r1 + r2 + r3 + r4;
}

/* ==================== Generic fallback ==================== */
#else

/* Generic functions using complex inline assembly with many operands */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long r1, r2, r3;
    
    /* 10 operands: 7 inputs, 3 outputs */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "imul %[d], %[r2]\n\t"
        "mov %[e], %[r3]\n\t"
        "add %[f], %[r3]\n\t"
        "add %[g], %[r1]\n\t"
        "add %[h], %[r2]\n\t"
        "add %[i], %[r3]\n\t"
        "add %[j], %[r1]\n\t"
        : [r1] "=&r" (r1),
          [r2] "=&r" (r2),
          [r3] "=&r" (r3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    global_counter += r1 + r2 + r3;
}

__attribute__((optimize("O3,no-inline")))
void test_11_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long r1, r2, r3, r4;
    
    /* 11 operands: 7 inputs, 4 outputs */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "imul %[d], %[r2]\n\t"
        "mov %[e], %[r3]\n\t"
        "sub %[f], %[r3]\n\t"
        "mov %[g], %[r4]\n\t"
        "xor %[h], %[r4]\n\t"
        "add %[i], %[r1]\n\t"
        "add %[j], %[r2]\n\t"
        "add %[k], %[r3]\n\t"
        : [r1] "=&r" (r1),
          [r2] "=&r" (r2),
          [r3] "=&r" (r3),
          [r4] "=&r" (r4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += r1 + r2 + r3 + r4;
}

#endif

/* ==================== Main driver ==================== */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call architecture-specific or generic test functions */
#ifdef __x86_64__
    test_10_operand_x86();
    test_11_operand_x86();
    printf("x86_64 tests executed\n");
#elif defined(__aarch64__) || defined(__arm__)
    test_10_operand_arm();
    test_11_operand_arm();
    printf("ARM tests executed\n");
#else
    test_10_operand_generic();
    test_11_operand_generic();
    printf("Generic tests executed\n");
#endif
    
    printf("Global counter: %d\n", global_counter);
    return 0;
}
