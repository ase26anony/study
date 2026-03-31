/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, covering lines 8254-8263 in optabs.cc.
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
    /* Complex expression that might require many temporary registers */
    uint64_t result = 0;
    
    /* Multi-step calculation with many intermediate values */
    uint64_t t1 = (a * b) >> 32;      /* Might use highpart multiplication */
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them together */
    result = t1 + t2 - t3 * t4 + t5;
    result = (result << 5) | (result >> 59);  /* Rotate */
    
    /* More operations to increase register pressure */
    result ^= (a + b + c + d + e + f + g + h + i + j);
    
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
    /* x86_64 inline assembly with many input/output registers */
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "subq %3, %%rax\n\t"
        "mulq %4\n\t"
        "addq %5, %%rax\n\t"
        "subq %6, %%rax\n\t"
        "addq %7, %%rax\n\t"
        "subq %8, %%rax\n\t"
        "addq %9, %%rax\n\t"
        "subq %10, %%rax\n\t"
        "movq %%rax, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j)
        : "%rax", "%rdx", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many registers */
    asm volatile (
        "add %x[res], %x[a], %x[b]\n\t"
        "sub %x[res], %x[res], %x[c]\n\t"
        "add %x[res], %x[res], %x[d]\n\t"
        "sub %x[res], %x[res], %x[e]\n\t"
        "add %x[res], %x[res], %x[f]\n\t"
        "sub %x[res], %x[res], %x[g]\n\t"
        "add %x[res], %x[res], %x[h]\n\t"
        "sub %x[res], %x[res], %x[i]\n\t"
        "add %x[res], %x[res], %x[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a + b - c + d - e + f - g + h - i + j;
#endif
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_operations(v4si a, v4si b, v4si c, v4si d,
                                       v4si e, v4si f, v4si g) {
    v4si result;
    
    /* Complex vector expression */
    result = (a * b) + (c * d) - (e * f) + g;
    
    /* Element-wise operations */
    result = result << 2;
    result = result >> 1;
    
    /* Mix with scalar operations */
    int* rp = (int*)&result;
    for (int i = 0; i < 4; i++) {
        rp[i] = rp[i] ^ (i * 0x55555555);
    }
    
    return result;
}

/* Function using GCC builtins that might expand to many operands */
NOINLINE static uint64_t builtin_operations(uint64_t a, uint64_t b,
                                            uint64_t c, uint64_t d) {
    uint64_t result = 0;
    
    /* Use builtins that might require many operands */
#ifdef __GNUC__
    result = __builtin_add_overflow(a, b, &result) ? 0 : result;
    result = __builtin_mul_overflow(result, c, &result) ? 0 : result;
    result = __builtin_sub_overflow(result, d, &result) ? 0 : result;
    
    /* Bit manipulation builtins */
    result = __builtin_bswap64(result);
    result = __builtin_rotateleft64(result, 13);
    result = __builtin_rotateright64(result, 7);
#endif
    
    return result;
}

/* Multi-precision arithmetic that might use expand_mult_highpart */
NOINLINE static uint64_t multiprecision_mul(uint32_t a, uint32_t b,
                                            uint32_t c, uint32_t d) {
    /* 64-bit result from 32-bit multiplies */
    uint64_t a64 = (uint64_t)a * b;
    uint64_t b64 = (uint64_t)c * d;
    
    /* High part multiplications */
    uint64_t high_a = (a >> 16) * (b >> 16);
    uint64_t high_b = (c >> 16) * (d >> 16);
    
    /* Combine results */
    uint64_t result = a64 + b64 + (high_a << 32) + (high_b << 32);
    
    /* More operations to increase complexity */
    result = (result & 0xFFFFFFFF) * (result >> 32);
    result = result ^ (a64 >> 32) ^ (b64 >> 32);
    
    return result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    uint64_t total = 0;
    
    /* Initialize many variables to use as operands */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (uint64_t)i * 0x123456789ABCDEFULL + argc;
    }
    
    /* Test different code paths based on command line arguments */
    if (argc > 1) {
        /* Path 1: Complex arithmetic with many operands */
        for (int i = 0; i < 10; i++) {
            total += multi_operand_arithmetic(
                vars[0], vars[1], vars[2], vars[3], vars[4],
                vars[5], vars[6], vars[7], vars[8], vars[9]
            );
        }
    } else {
        /* Path 2: Inline assembly with many operands */
        for (int i = 0; i < 10; i++) {
            total += inline_asm_10_operands(
                vars[10], vars[11], vars[12], vars[13], vars[14],
                vars[15], vars[16], vars[17], vars[18], vars[19]
            );
        }
    }
    
    /* Always test vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    v4si vec_e = {17, 18, 19, 20};
    v4si vec_f = {21, 22, 23, 24};
    v4si vec_g = {25, 26, 27, 28};
    
    v4si vec_result = vector_operations(vec_a, vec_b, vec_c, vec_d,
                                        vec_e, vec_f, vec_g);
    
    /* Add vector results to total */
    int* vp = (int*)&vec_result;
    for (int i = 0; i < 4; i++) {
        total += vp[i];
    }
    
    /* Test builtin operations */
    total += builtin_operations(vars[0], vars[1], vars[2], vars[3]);
    
    /* Test multi-precision multiplication */
    total += multiprecision_mul(
        (uint32_t)vars[4], (uint32_t)vars[5],
        (uint32_t)vars[6], (uint32_t)vars[7]
    );
    
    /* Mix in command line arguments */
    for (int i = 0; i < argc && i < 10; i++) {
        total += (uint64_t)argv[i][0];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)total);
    
    return (int)(total & 0x7FFFFFFF);
}
