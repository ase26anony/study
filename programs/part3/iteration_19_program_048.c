/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, covering uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the generated .expand file for multi-operand RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
    /* Complex expression that might require many temporary operands */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    return ((t1 + t2) * (t3 + t4)) ^ t5;
}

/* Function using inline assembly with many operands */
NOINLINE static uint64_t inline_asm_10_operands(uint64_t a, uint64_t b,
                                                uint64_t c, uint64_t d,
                                                uint64_t e, uint64_t f,
                                                uint64_t g, uint64_t h,
                                                uint64_t i, uint64_t j) {
    uint64_t result;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with 10 operands */
    asm volatile (
        "imulq %[b], %[a]\n\t"
        "imulq %[d], %[c]\n\t"
        "imulq %[f], %[e]\n\t"
        "imulq %[h], %[g]\n\t"
        "imulq %[j], %[i]\n\t"
        "addq %[c], %[a]\n\t"
        "addq %[g], %[e]\n\t"
        "addq %[i], %[e]\n\t"
        "imulq %[e], %[a]\n\t"
        "movq %[a], %[result]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "mul %[a], %[a], %[b]\n\t"
        "mul %[c], %[c], %[d]\n\t"
        "mul %[e], %[e], %[f]\n\t"
        "mul %[g], %[g], %[h]\n\t"
        "mul %[i], %[i], %[j]\n\t"
        "add %[a], %[a], %[c]\n\t"
        "add %[e], %[e], %[g]\n\t"
        "add %[e], %[e], %[i]\n\t"
        "mul %[a], %[a], %[e]\n\t"
        "mov %[result], %[a]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
#endif
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c,
                                          v4si d, v4si e) {
    /* Complex vector expression */
    v4si t1 = a * b;
    v4si t2 = c * d;
    v4si t3 = e * a;
    v4si t4 = b * c;
    v4si t5 = d * e;
    
    /* Mix with shifts and adds */
    v4si result = (t1 >> 4) + (t2 >> 3) + (t3 >> 2) + (t4 >> 1) + t5;
    
    /* Additional operations to increase operand count */
    result = result * a + result * b + result * c + result * d + result * e;
    
    return result;
}

/* Function that might trigger 11-operand expansion */
NOINLINE static uint64_t potential_11_operands(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h,
                                               uint64_t i, uint64_t j,
                                               uint64_t k) {
    /* Very complex expression with 11 input variables */
    uint64_t t1 = (a * b + c) >> (d & 0x1F);
    uint64_t t2 = (e * f + g) >> (h & 0x1F);
    uint64_t t3 = (i * j + k) >> (a & 0x1F);
    
    /* Mix with conditional operations */
    uint64_t result = t1;
    if (t2 > t3) {
        result += t2 * t3;
    } else {
        result += t3 * t1;
    }
    
    /* Additional arithmetic chain */
    result = result * a + result * b + result * c + 
             result * d + result * e + result * f;
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values */
    uint64_t vals[11];
    for (int i = 0; i < 11; i++) {
        vals[i] = (uint64_t)(argc + i) * 123456789;
    }
    
    /* Test different code paths based on argc */
    if (argc > 1) {
        /* Path 1: Test 10-operand inline assembly */
        for (int i = 0; i < (argc % 10); i++) {
            result ^= inline_asm_10_operands(
                vals[0] + i, vals[1] + i, vals[2] + i, vals[3] + i,
                vals[4] + i, vals[5] + i, vals[6] + i, vals[7] + i,
                vals[8] + i, vals[9] + i
            );
        }
    } else {
        /* Path 2: Test complex arithmetic */
        for (int i = 0; i < 5; i++) {
            result ^= multi_operand_arithmetic(
                vals[0] + i, vals[1] + i, vals[2] + i, vals[3] + i,
                vals[4] + i, vals[5] + i, vals[6] + i, vals[7] + i,
                vals[8] + i, vals[9] + i
            );
        }
    }
    
    /* Test 11-operand path if argc > 2 */
    if (argc > 2) {
        result += potential_11_operands(
            vals[0], vals[1], vals[2], vals[3], vals[4],
            vals[5], vals[6], vals[7], vals[8], vals[9], vals[10]
        );
    }
    
    /* Test vector operations */
    v4si vec_a = {vals[0], vals[1], vals[2], vals[3]};
    v4si vec_b = {vals[4], vals[5], vals[6], vals[7]};
    v4si vec_c = {vals[8], vals[9], vals[0], vals[1]};
    v4si vec_d = {vals[2], vals[3], vals[4], vals[5]};
    v4si vec_e = {vals[6], vals[7], vals[8], vals[9]};
    
    v4si vec_result = vector_multi_operand(vec_a, vec_b, vec_c, vec_d, vec_e);
    
    /* Use vector result to prevent dead code elimination */
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    
    return (int)(result % 256);
}
