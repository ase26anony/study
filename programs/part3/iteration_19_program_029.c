/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for 10 and 11-operand
 * instruction patterns, covering lines 8254-8263 in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Additional flags: -fdump-rtl-combine -fdump-rtl-all -dP
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
                                                  uint64_t i, uint64_t j)
{
    /* Complex expression that GCC might break into many operations */
    uint64_t result = 0;
    
    /* Multi-precision style computation */
    result = ((a * b) >> 32) + ((c * d) >> 32);
    result += ((e * f) >> 32) + ((g * h) >> 32);
    result += ((i * j) >> 32);
    
    /* Additional operations to increase operand count */
    result = (result * a) / (b + 1);
    result = (result + c) * (d + 1) / (e + 1);
    
    return result;
}

/* Function using inline assembly with many operands */
NOINLINE static uint64_t inline_asm_10_operands(uint64_t a, uint64_t b,
                                                uint64_t c, uint64_t d,
                                                uint64_t e, uint64_t f,
                                                uint64_t g, uint64_t h,
                                                uint64_t i, uint64_t j)
{
    uint64_t result;
    
#if defined(__x86_64__)
    /* x86_64 inline assembly with many operands */
    asm volatile (
        /* Complex multi-operand sequence */
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "imul %3, %0\n\t"
        "add %4, %0\n\t"
        "imul %5, %0\n\t"
        "add %6, %0\n\t"
        "imul %7, %0\n\t"
        "add %8, %0\n\t"
        "imul %9, %0\n\t"
        "add %10, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j)
        : "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "add %0, %1, %2\n\t"
        "mul %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "mul %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mul %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "mul %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a + b * c + d * e + f * g + h * i + j;
#endif
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                          v4si e, v4si f, v4si g, v4si h)
{
    v4si result;
    
    /* Complex vector expression */
    result = a * b + c * d;
    result = result * e + f * g;
    result = (result >> 2) + h;
    result = result * a - b + c - d + e - f + g - h;
    
    /* Element-wise operations */
    result[0] = (a[0] * b[0] + c[0] * d[0]) >> e[0];
    result[1] = (a[1] * b[1] + c[1] * d[1]) >> e[1];
    result[2] = (a[2] * b[2] + c[2] * d[2]) >> e[2];
    result[3] = (a[3] * b[3] + c[3] * d[3]) >> e[3];
    
    return result;
}

/* Function using compiler builtins that might expand to many operands */
NOINLINE static uint64_t builtin_multi_operand(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h,
                                               uint64_t i, uint64_t j,
                                               uint64_t k)
{
    uint64_t result = 0;
    
    /* Chain of builtins/operations */
    result = __builtin_add_overflow(a, b, &result) ? 0 : result;
    result = __builtin_mul_overflow(result, c, &result) ? 0 : result;
    result = __builtin_add_overflow(result, d, &result) ? 0 : result;
    result = __builtin_mul_overflow(result, e, &result) ? 0 : result;
    result = __builtin_add_overflow(result, f, &result) ? 0 : result;
    result = __builtin_mul_overflow(result, g, &result) ? 0 : result;
    result = __builtin_add_overflow(result, h, &result) ? 0 : result;
    result = __builtin_mul_overflow(result, i, &result) ? 0 : result;
    result = __builtin_add_overflow(result, j, &result) ? 0 : result;
    result = __builtin_mul_overflow(result, k, &result) ? 0 : result;
    
    /* Additional bit manipulation */
    result = (result << 5) | (result >> 59);
    result = __builtin_bswap64(result);
    result = __builtin_popcountll(result) + result;
    
    return result;
}

/* Mixed operations to increase chances of hitting 10/11 operand cases */
NOINLINE static uint64_t mixed_operations(uint64_t ops[11])
{
    uint64_t temp[11];
    
    /* Copy to force register allocation */
    for (int idx = 0; idx < 11; idx++) {
        temp[idx] = ops[idx] + idx;
    }
    
    /* Complex expression with many operands */
    uint64_t result = ((temp[0] * temp[1]) >> temp[2]) +
                      ((temp[3] * temp[4]) >> temp[5]) +
                      ((temp[6] * temp[7]) >> temp[8]) +
                      ((temp[9] * temp[10]) >> 1);
    
    /* More operations to increase operand usage */
    result = (result & temp[0]) | (result & temp[1]) |
             (result & temp[2]) | (result & temp[3]);
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[])
{
    uint64_t result = 0;
    
    /* Initialize operands with varying values */
    uint64_t operands[11];
    for (int i = 0; i < 11; i++) {
        operands[i] = (uint64_t)(argc + i) * 0x123456789ABCDEFULL;
    }
    
    /* Vector operands */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    v4si vec_e = {17, 18, 19, 20};
    v4si vec_f = {21, 22, 23, 24};
    v4si vec_g = {25, 26, 27, 28};
    v4si vec_h = {29, 30, 31, 32};
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        for (int i = 0; i < (argc % 5); i++) {
            result += multi_operand_arithmetic(
                operands[0], operands[1], operands[2], operands[3],
                operands[4], operands[5], operands[6], operands[7],
                operands[8], operands[9]
            );
        }
    } else {
        /* Path 2: Inline assembly */
        result = inline_asm_10_operands(
            operands[0], operands[1], operands[2], operands[3],
            operands[4], operands[5], operands[6], operands[7],
            operands[8], operands[9]
        );
    }
    
    /* Always test vector operations */
    v4si vec_result = vector_multi_operand(
        vec_a, vec_b, vec_c, vec_d,
        vec_e, vec_f, vec_g, vec_h
    );
    
    /* Sum vector elements */
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    /* Test builtin chain (11 operands) */
    if (argc > 2) {
        result += builtin_multi_operand(
            operands[0], operands[1], operands[2], operands[3],
            operands[4], operands[5], operands[6], operands[7],
            operands[8], operands[9], operands[10]
        );
    }
    
    /* Test mixed operations */
    result += mixed_operations(operands);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)result);
    
    return (int)(result & 0xFF);
}
