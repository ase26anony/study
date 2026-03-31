/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function attribute to force optimization */
#define FORCE_OPTIMIZE __attribute__((optimize("O3", "no-inline")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test function for 10-operand case using AVX-512 intrinsics */
FORCE_OPTIMIZE
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic typically expands to many operands:
       dest, mask, idx, src1, src2 + various immediates */
    __m512i result = _mm512_mask2_permutex2var_epi64(src1, idx, mask, src2, idx);
    
    /* Use result to prevent elimination */
    global_counter += _mm512_reduce_add_epi64(result);
}

/* Test function for 11-operand case */
FORCE_OPTIMIZE
void test_11_operand_x86(void) {
    /* Use inline assembly with exactly 11 operands */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint64_t result1, result2, result3;
    
    /* Complex inline assembly with 11 operands:
       3 outputs + 8 inputs = 11 total operands */
    asm volatile (
        /* Multi-operand computation */
        "addq %[in1], %[in2]\n\t"
        "adcq %[in3], %[in4]\n\t"
        "mulq %[in5]\n\t"
        "addq %[in6], %%rax\n\t"
        "adcq %[in7], %%rdx\n\t"
        "movq %%rax, %[out1]\n\t"
        "movq %%rdx, %[out2]\n\t"
        "xorq %[in8], %[out3]"
        : [out1] "=r" (result1),
          [out2] "=r" (result2),
          [out3] "=r" (result3)
        : [in1] "r" (a),
          [in2] "r" (b),
          [in3] "r" (c),
          [in4] "r" (d),
          [in5] "r" (e),
          [in6] "r" (f),
          [in7] "r" (g),
          [in8] "r" (h)
        : "rax", "rdx", "cc"
    );
    
    global_counter += result1 + result2 + result3 + i + j + k;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* Test function for 10-operand case using ARM NEON */
FORCE_OPTIMIZE
void test_10_operand_arm(void) {
    /* Complex NEON operation chain */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Multiple operations that might combine */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    uint8x16_t r4 = vmulq_u8(r3, a);
    uint8x16_t r5 = veorq_u8(r4, b);
    
    /* Table lookup with multiple registers - can expand to many operands */
    uint8x16_t result = vqtbl1q_u8(r5, vcreate_u8(0x0706050403020100));
    
    /* Use result */
    global_counter += vgetq_lane_u8(result, 0);
}

/* Test function for 11-operand case for ARM */
FORCE_OPTIMIZE
void test_11_operand_arm(void) {
    /* Inline assembly with 11 operands for ARM */
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    uint64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    uint64_t out1, out2, out3;
    
    asm volatile (
        /* Complex multi-operand computation */
        "add %[out1], %[in1], %[in2]\n\t"
        "adc %[out2], %[in3], %[in4]\n\t"
        "mul %[out3], %[in5], %[in6]\n\t"
        "add %[out1], %[out1], %[in7]\n\t"
        "adc %[out2], %[out2], %[in8]\n\t"
        "eor %[out3], %[out3], %[in9]\n\t"
        "orr %[out1], %[out1], %[in10]"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3)
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "r" (in3),
          [in4] "r" (in4),
          [in5] "r" (in5),
          [in6] "r" (in6),
          [in7] "r" (in7),
          [in8] "r" (in8),
          [in9] "r" (in9),
          [in10] "r" (in10)
        : "cc"
    );
    
    global_counter += out1 + out2 + out3;
}

#else
/* Generic fallback using complex inline assembly */

FORCE_OPTIMIZE
void test_10_operand_generic(void) {
    /* Generic inline assembly with 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9;
    long result1, result2;
    
    asm volatile (
        "add %[res1], %[a], %[b]\n\t"
        "sub %[res2], %[c], %[d]\n\t"
        "mul %[res1], %[res1], %[e]\n\t"
        "div %[res2], %[res2], %[f]\n\t"
        "and %[res1], %[res1], %[g]\n\t"
        "or  %[res2], %[res2], %[h]\n\t"
        "xor %[res1], %[res1], %[i]"
        : [res1] "=r" (result1),
          [res2] "=r" (result2)
        : [a] "r" (op1),
          [b] "r" (op2),
          [c] "r" (op3),
          [d] "r" (op4),
          [e] "r" (op5),
          [f] "r" (op6),
          [g] "r" (op7),
          [h] "r" (op8),
          [i] "r" (op9)
        : "cc"
    );
    
    global_counter += result1 + result2;
}

FORCE_OPTIMIZE
void test_11_operand_generic(void) {
    /* Generic inline assembly with 11 operands */
    long in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    long in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    long out1, out2, out3;
    
    asm volatile (
        "mov %[o1], %[i1]\n\t"
        "add %[o1], %[o1], %[i2]\n\t"
        "mov %[o2], %[i3]\n\t"
        "sub %[o2], %[o2], %[i4]\n\t"
        "mov %[o3], %[i5]\n\t"
        "mul %[o3], %[o3], %[i6]\n\t"
        "add %[o1], %[o1], %[i7]\n\t"
        "sub %[o2], %[o2], %[i8]\n\t"
        "mul %[o3], %[o3], %[i9]\n\t"
        "xor %[o1], %[o1], %[i10]"
        : [o1] "=r" (out1),
          [o2] "=r" (out2),
          [o3] "=r" (out3)
        : [i1] "r" (in1),
          [i2] "r" (in2),
          [i3] "r" (in3),
          [i4] "r" (in4),
          [i5] "r" (in5),
          [i6] "r" (in6),
          [i7] "r" (in7),
          [i8] "r" (in8),
          [i9] "r" (in9),
          [i10] "r" (in10)
        : "cc"
    );
    
    global_counter += out1 + out2 + out3;
}
#endif

/* Additional test using complex vector operations that might combine */
FORCE_OPTIMIZE
void test_vector_combine(void) {
#ifdef __AVX512F__
    /* Chain of AVX-512 operations that might combine into a single pattern */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    /* Complex sequence that might be recognized as a single pattern */
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_mullo_epi32(r1, v3);
    __m512i r3 = _mm512_slli_epi32(r2, 2);
    __m512i r4 = _mm512_and_si512(r3, v4);
    __m512i r5 = _mm512_or_si512(r4, v1);
    
    /* Masked operation with many parameters */
    __mmask16 mask = 0xAAAA;
    __m512i result = _mm512_mask_blend_epi32(mask, r5, v2);
    
    global_counter += _mm512_reduce_add_epi32(result);
#endif
}

/* Main driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
#ifdef __x86_64__
    test_10_operand_x86();
    test_11_operand_x86();
#elif defined(__aarch64__)
    test_10_operand_arm();
    test_11_operand_arm();
#else
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    test_vector_combine();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return global_counter > 0 ? 0 : 1;
}
