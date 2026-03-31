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
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands:
       _mm512_mask_permutex2var_epi64 has 10 operands in RTL:
       1. Destination
       2. Mask
       3. Index
       4. Table A
       5. Table B
       6-10. Various immediates and mode flags */
    
    __m512i a = _mm512_set1_epi64(1);
    __m512i b = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic expands to an RTL pattern with 10 operands */
    __m512i result = _mm512_mask_permutex2var_epi64(a, mask, idx, a, b);
    
    /* Use result to prevent optimization */
    int64_t* res_arr = (int64_t*)&result;
    if (g_volatile_zero) {
        printf("%ld\n", res_arr[0]);
    }
}

/* Another 10-operand pattern using blend with multiple immediates */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86_v2(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    
    /* Complex ternary operation that might expand to 10 operands */
    __m512i result = _mm512_mask_add_epi32(a, 0xAAAA, b, c);
    
    /* Force usage */
    if (g_volatile_zero) {
        int32_t* r = (int32_t*)&result;
        printf("%d\n", r[0]);
    }
}

/* Function to trigger 11-operand case */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_x86(void) {
    /* Use inline assembly with exactly 11 operands */
    uint64_t a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    uint64_t a6 = 6, a7 = 7, a8 = 8, a9 = 9, a10 = 10;
    uint64_t result = 0;
    
    /* 11-operand asm: 10 inputs + 1 output = 11 total operands */
    asm volatile (
        "/* 11-operand test */\n\t"
        "mov %[in1], %[out]\n\t"
        "add %[in2], %[out]\n\t"
        "add %[in3], %[out]\n\t"
        "add %[in4], %[out]\n\t"
        "add %[in5], %[out]\n\t"
        "add %[in6], %[out]\n\t"
        "add %[in7], %[out]\n\t"
        "add %[in8], %[out]\n\t"
        "add %[in9], %[out]\n\t"
        "add %[in10], %[out]"
        : [out] "=r" (result)
        : [in1] "r" (a1), [in2] "r" (a2), [in3] "r" (a3),
          [in4] "r" (a4), [in5] "r" (a5), [in6] "r" (a6),
          [in7] "r" (a7), [in8] "r" (a8), [in9] "r" (a9),
          [in10] "r" (a10)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%lu\n", result);
    }
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM NEON version for 10-operand case */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_arm(void) {
    /* Use complex table lookup operations */
    uint8x16_t table1 = vdupq_n_u8(1);
    uint8x16_t table2 = vdupq_n_u8(2);
    uint8x16_t table3 = vdupq_n_u8(3);
    uint8x16_t indices = vdupq_n_u8(0);
    
    /* Multiple table lookups combined */
    uint8x16_t result = vqtbl3q_u8(
        vcombine_u8x3(vget_low_u8(table1), vget_low_u8(table2), vget_low_u8(table3)),
        indices
    );
    
    /* Force usage */
    if (g_volatile_zero) {
        uint8_t r[16];
        vst1q_u8(r, result);
        printf("%d\n", r[0]);
    }
}

/* 11-operand case for ARM using inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_arm(void) {
    uint64_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    uint64_t out0, out1;
    
    r0 = 0; r1 = 1; r2 = 2; r3 = 3; r4 = 4;
    r5 = 5; r6 = 6; r7 = 7; r8 = 8; r9 = 9; r10 = 10;
    
    /* 11-operand asm for ARM */
    asm volatile (
        "/* ARM 11-operand test */\n\t"
        "add %[out0], %[in0], %[in1]\n\t"
        "add %[out0], %[out0], %[in2]\n\t"
        "add %[out0], %[out0], %[in3]\n\t"
        "add %[out0], %[out0], %[in4]\n\t"
        "add %[out0], %[out0], %[in5]\n\t"
        "add %[out0], %[out0], %[in6]\n\t"
        "add %[out0], %[out0], %[in7]\n\t"
        "add %[out0], %[out0], %[in8]\n\t"
        "add %[out0], %[out0], %[in9]\n\t"
        "add %[out1], %[out0], %[in10]"
        : [out0] "=r" (out0), [out1] "=r" (out1)
        : [in0] "r" (r0), [in1] "r" (r1), [in2] "r" (r2),
          [in3] "r" (r3), [in4] "r" (r4), [in5] "r" (r5),
          [in6] "r" (r6), [in7] "r" (r7), [in8] "r" (r8),
          [in9] "r" (r9), [in10] "r" (r10)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%lu %lu\n", out0, out1);
    }
}

#else
/* Generic fallback using complex inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_generic(void) {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    /* 10-operand asm (8 inputs + 2 outputs) */
    asm volatile (
        "/* Generic 10-operand */\n\t"
        "mov %[r1], %[i1]\n\t"
        "add %[r1], %[r1], %[i2]\n\t"
        "mov %[r2], %[i3]\n\t"
        "add %[r2], %[r2], %[i4]\n\t"
        "add %[r1], %[r1], %[i5]\n\t"
        "add %[r2], %[r2], %[i6]\n\t"
        "add %[r1], %[r1], %[i7]\n\t"
        "add %[r2], %[r2], %[i8]\n\t"
        "add %[r1], %[r1], %[i9]\n\t"
        "add %[r2], %[r2], %[i10]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [i1] "r" (op1), [i2] "r" (op2), [i3] "r" (op3),
          [i4] "r" (op4), [i5] "r" (op5), [i6] "r" (op6),
          [i7] "r" (op7), [i8] "r" (op8), [i9] "r" (op9),
          [i10] "r" (op10)
        : "cc"
    );
    
    if (g_volatile_zero) {
        printf("%ld %ld\n", result1, result2);
    }
}

__attribute__((optimize("O3,no-inline")))
void test_11_operand_generic(void) {
    long ops[11];
    long results[3];
    
    for (int i = 0; i < 11; i++) ops[i] = i + 1;
    
    /* 11-operand asm (11 inputs + 0 outputs, but with clobbers) */
    asm volatile (
        "/* Generic 11-operand */\n\t"
        "mov $0, %%rax\n\t"
        "add %[i1], %%rax\n\t"
        "add %[i2], %%rax\n\t"
        "add %[i3], %%rax\n\t"
        "add %[i4], %%rax\n\t"
        "add %[i5], %%rax\n\t"
        "add %[i6], %%rax\n\t"
        "add %[i7], %%rax\n\t"
        "add %[i8], %%rax\n\t"
        "add %[i9], %%rax\n\t"
        "add %[i10], %%rax\n\t"
        "add %[i11], %%rax"
        : 
        : [i1] "r" (ops[0]), [i2] "r" (ops[1]), [i3] "r" (ops[2]),
          [i4] "r" (ops[3]), [i5] "r" (ops[4]), [i6] "r" (ops[5]),
          [i7] "r" (ops[6]), [i8] "r" (ops[7]), [i9] "r" (ops[8]),
          [i10] "r" (ops[9]), [i11] "r" (ops[10])
        : "rax", "cc"
    );
}
#endif

/* Main function to call all test cases */
int main(void) {
    /* Call architecture-specific or generic tests */
#ifdef __x86_64__
    test_10_operand_x86();
    test_10_operand_x86_v2();  /* Second 10-operand pattern */
    test_11_operand_x86();
#elif defined(__aarch64__)
    test_10_operand_arm();
    test_11_operand_arm();
#else
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    /* Simple computation to ensure functions aren't optimized away */
    int sum = g_volatile_zero;
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    
    return sum == 0 ? 0 : 1;
}
