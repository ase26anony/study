/* test_optabs_10_11_operands.c
 * 
 * This program attempts to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, targeting the uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the generated .expand file for multi-operand RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure local expansion */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE static uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, 
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j) {
    /* Complex expression that might require many temporary registers */
    uint64_t result = 0;
    
    /* Multi-step calculation that could expand to many operands */
    result = a * b + c * d + e * f + g * h + i * j;
    result = (result << 5) | (result >> 59);  /* Rotate */
    result = result * 0x9e3779b97f4a7c15ULL;  /* Multiplication by constant */
    result = result ^ (result >> 33);
    result = result * 0xff51afd7ed558ccdULL;
    result = result ^ (result >> 33);
    
    return result;
}

/* Function using inline assembly with many operands */
NOINLINE static uint64_t inline_asm_10_operands(uint64_t a, uint64_t b,
                                                uint64_t c, uint64_t d,
                                                uint64_t e, uint64_t f,
                                                uint64_t g, uint64_t h,
                                                uint64_t i, uint64_t j) {
    uint64_t result;
    
#ifdef __x86_64__
    /* x86-64 inline assembly with many operands */
    asm volatile (
        "movq %1, %0\n\t"
        "addq %2, %0\n\t"
        "addq %3, %0\n\t"
        "addq %4, %0\n\t"
        "addq %5, %0\n\t"
        "addq %6, %0\n\t"
        "addq %7, %0\n\t"
        "addq %8, %0\n\t"
        "addq %9, %0\n\t"
        "addq %10, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a + b + c + d + e + f + g + h + i + j;
#endif
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                          v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si result;
    
    /* This might expand to many operations */
    result = a * b + c * d;
    result = result + e * f;
    result = result + g * h;
    
    /* Element-wise operations */
    result = (result << 2) | (result >> 30);
    
    return result;
}

/* Function that uses many parameters and local variables */
NOINLINE static uint64_t many_parameter_function(
    uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4,
    uint64_t p5, uint64_t p6, uint64_t p7, uint64_t p8,
    uint64_t p9, uint64_t p10, uint64_t p11) {
    
    /* Many local variables to force register pressure */
    uint64_t l1 = p1 * 0x123456789ABCDEFULL;
    uint64_t l2 = p2 * 0xFEDCBA987654321ULL;
    uint64_t l3 = p3 * 0x13579BDF02468ACEULL;
    uint64_t l4 = p4 * 0xEC86420FDB97531ULL;
    uint64_t l5 = p5 * 0x55AA55AA55AA55AAULL;
    uint64_t l6 = p6 * 0xAA55AA55AA55AA55ULL;
    uint64_t l7 = p7 * 0x3333333333333333ULL;
    uint64_t l8 = p8 * 0xCCCCCCCCCCCCCCCCULL;
    uint64_t l9 = p9 * 0x0F0F0F0F0F0F0F0FULL;
    uint64_t l10 = p10 * 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t l11 = p11 * 0xFFFFFFFFFFFFFFFFULL;
    
    /* Complex expression using all variables */
    uint64_t result = (l1 + l2) * (l3 + l4);
    result += (l5 + l6) * (l7 + l8);
    result += (l9 + l10) * l11;
    
    /* More operations to increase operand count */
    result = ((result << 19) | (result >> 45)) ^ 0xDEADBEEFCAFEBABEULL;
    
    return result;
}

/* Multi-precision arithmetic that might need many operands */
NOINLINE static void multi_precision_calc(uint64_t a_high, uint64_t a_low,
                                          uint64_t b_high, uint64_t b_low,
                                          uint64_t *result_high,
                                          uint64_t *result_low) {
    /* 128-bit multiplication using 64-bit parts */
    uint64_t t1 = a_low * b_low;
    uint64_t t2 = a_high * b_low;
    uint64_t t3 = a_low * b_high;
    uint64_t t4 = a_high * b_high;
    
    /* Carry propagation - this might expand to many operations */
    uint64_t carry = 0;
    *result_low = t1;
    
    /* Add with carry - might trigger multi-operand expansion */
    uint64_t sum1 = t2 + t3;
    if (sum1 < t2) carry = 1;
    
    uint64_t sum2 = (*result_low >> 32) + (t1 >> 32) + (sum1 << 32);
    *result_high = t4 + (sum1 >> 32) + (sum2 >> 32) + carry;
    *result_low = (sum2 << 32) | (*result_low & 0xFFFFFFFF);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    uint64_t values[11];
    
    /* Initialize with pseudo-random values based on argc */
    for (int i = 0; i < 11; i++) {
        values[i] = (uint64_t)(argc + i) * 0x123456789ABCDEFULL;
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        result = multi_operand_arithmetic(values[0], values[1], values[2],
                                         values[3], values[4], values[5],
                                         values[6], values[7], values[8],
                                         values[9]);
    } else if (argc > 2) {
        /* Path 2: Inline assembly */
        result = inline_asm_10_operands(values[0], values[1], values[2],
                                       values[3], values[4], values[5],
                                       values[6], values[7], values[8],
                                       values[9]);
    } else if (argc > 3) {
        /* Path 3: Many parameter function (11 operands) */
        result = many_parameter_function(values[0], values[1], values[2],
                                        values[3], values[4], values[5],
                                        values[6], values[7], values[8],
                                        values[9], values[10]);
    } else {
        /* Path 4: Multi-precision arithmetic */
        uint64_t high, low;
        multi_precision_calc(values[0], values[1], values[2], values[3],
                            &high, &low);
        result = high ^ low;
    }
    
    /* Vector operations - always executed */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    v4si vec_e = {17, 18, 19, 20};
    v4si vec_f = {21, 22, 23, 24};
    v4si vec_g = {25, 26, 27, 28};
    v4si vec_h = {29, 30, 31, 32};
    
    v4si vec_result = vector_multi_operand(vec_a, vec_b, vec_c, vec_d,
                                          vec_e, vec_f, vec_g, vec_h);
    
    /* Use vector result to affect final output */
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    /* Loop with varying inputs to increase coverage */
    for (int i = 0; i < (argc % 5); i++) {
        result ^= multi_operand_arithmetic(result, i, i+1, i+2, i+3,
                                          i+4, i+5, i+6, i+7, i+8);
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
