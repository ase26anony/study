/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile_zero = 0;

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function to trigger 10-operand case using AVX-512 intrinsics */
__attribute__((noinline, target("avx512f,avx512vl")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 masked permute with multiple operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 expands to many operands:
       dest, mask, idx, src1, src2 + various immediates */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    int64_t* res_ptr = (int64_t*)&result;
    if (g_volatile_zero) {
        printf("%ld", res_ptr[0]);
    }
}

/* Another 10-operand pattern using blend with multiple sources */
__attribute__((noinline, target("avx512f")))
void test_10_operand_x86_v2(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __mmask8 mask = 0xAA;
    
    /* Complex ternary operation that may expand to many operands */
    __m512d result = _mm512_mask3_fmadd_pd(a, b, c, mask);
    
    if (g_volatile_zero) {
        double* res_ptr = (double*)&result;
        printf("%f", res_ptr[0]);
    }
}

/* Function to trigger 11-operand case using inline asm */
__attribute__((noinline))
void test_11_operand_x86(void) {
    uint64_t op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, op11;
    uint64_t out1, out2;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10; op11 = 11;
    
    /* Inline asm with 11 operands: 9 inputs, 2 outputs */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        "add %[in5], %[out1]\n\t"
        "sub %[in6], %[out2]\n\t"
        "or %[in7], %[out1]\n\t"
        "and %[in8], %[out2]\n\t"
        "xor %[in9], %[out1]\n\t"
        "add %[in10], %[out2]\n\t"
        "sub %[in11], %[out1]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2)
        : [in1] "r" (op1), [in2] "r" (op2), [in3] "r" (op3),
          [in4] "r" (op4), [in5] "r" (op5), [in6] "r" (op6),
          [in7] "r" (op7), [in8] "r" (op8), [in9] "r" (op9),
          [in10] "r" (op10), [in11] "r" (op11)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%lu %lu", out1, out2);
    }
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* Function for AArch64 to trigger 10-operand case */
__attribute__((noinline))
void test_10_operand_aarch64(void) {
    /* Complex NEON operation with multiple vector registers */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Chain operations that might combine into multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vsubq_u8(c, d);
    uint8x16_t r3 = vmulq_u8(r1, r2);
    uint8x16_t result = veorq_u8(r3, a);
    
    /* Table lookup with multiple operands */
    uint8x16_t table_result = vqtbl1q_u8(result, vdupq_n_u8(0));
    
    if (g_volatile_zero) {
        uint8_t* ptr = (uint8_t*)&table_result;
        printf("%d", ptr[0]);
    }
}

/* Inline asm for AArch64 with 11 operands */
__attribute__((noinline))
void test_11_operand_aarch64(void) {
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    uint64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10, in11 = 11;
    uint64_t out1, out2;
    
    asm volatile (
        "mov %[out1], %[in1]\n\t"
        "add %[out1], %[out1], %[in2]\n\t"
        "mov %[out2], %[in3]\n\t"
        "mul %[out2], %[out2], %[in4]\n\t"
        "add %[out1], %[out1], %[in5]\n\t"
        "sub %[out2], %[out2], %[in6]\n\t"
        "orr %[out1], %[out1], %[in7]\n\t"
        "and %[out2], %[out2], %[in8]\n\t"
        "eor %[out1], %[out1], %[in9]\n\t"
        "add %[out2], %[out2], %[in10]\n\t"
        "sub %[out1], %[out1], %[in11]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8), [in9] "r" (in9),
          [in10] "r" (in10), [in11] "r" (in11)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%lu %lu", out1, out2);
    }
}

#endif /* __aarch64__ */

/* Generic fallback using complex inline assembly */
__attribute__((noinline))
void test_10_operand_generic(void) {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2, result3;
    
    /* Complex asm with 10 input operands and 3 outputs */
    asm volatile (
        "mov %[r1], %[i1]\n\t"
        "add %[r1], %[r1], %[i2]\n\t"
        "mov %[r2], %[i3]\n\t"
        "imul %[r2], %[r2], %[i4]\n\t"
        "mov %[r3], %[i5]\n\t"
        "add %[r3], %[r3], %[i6]\n\t"
        "sub %[r1], %[r1], %[i7]\n\t"
        "or %[r2], %[r2], %[i8]\n\t"
        "and %[r3], %[r3], %[i9]\n\t"
        "xor %[r1], %[r1], %[i10]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [i1] "r" (op1), [i2] "r" (op2), [i3] "r" (op3),
          [i4] "r" (op4), [i5] "r" (op5), [i6] "r" (op6),
          [i7] "r" (op7), [i8] "r" (op8), [i9] "r" (op9),
          [i10] "r" (op10)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%ld %ld %ld", result1, result2, result3);
    }
}

__attribute__((noinline))
void test_11_operand_generic(void) {
    long in[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    long out1, out2, out3;
    
    /* Asm with 11 input memory operands */
    asm volatile (
        "mov %[o1], %[i0]\n\t"
        "add %[o1], %[o1], %[i1]\n\t"
        "mov %[o2], %[i2]\n\t"
        "imul %[o2], %[o2], %[i3]\n\t"
        "mov %[o3], %[i4]\n\t"
        "add %[o3], %[o3], %[i5]\n\t"
        "sub %[o1], %[o1], %[i6]\n\t"
        "or %[o2], %[o2], %[i7]\n\t"
        "and %[o3], %[o3], %[i8]\n\t"
        "xor %[o1], %[o1], %[i9]\n\t"
        "add %[o2], %[o2], %[i10]"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3)
        : [i0] "r" (in[0]), [i1] "r" (in[1]), [i2] "r" (in[2]),
          [i3] "r" (in[3]), [i4] "r" (in[4]), [i5] "r" (in[5]),
          [i6] "r" (in[6]), [i7] "r" (in[7]), [i8] "r" (in[8]),
          [i9] "r" (in[9]), [i10] "r" (in[10])
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%ld %ld %ld", out1, out2, out3);
    }
}

/* Main driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call architecture-specific tests */
#ifdef __x86_64__
    test_10_operand_x86();
    test_10_operand_x86_v2();
    test_11_operand_x86();
#elif defined(__aarch64__)
    test_10_operand_aarch64();
    test_11_operand_aarch64();
#endif
    
    /* Always call generic tests */
    test_10_operand_generic();
    test_11_operand_generic();
    
    printf("Done.\n");
    return 0;
}
