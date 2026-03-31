/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for 10 and 11-operand
 * instruction patterns by using various target-specific intrinsics,
 * inline assembly, and complex arithmetic operations.
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
NOINLINE
uint64_t complex_mul_highpart(uint64_t a, uint64_t b, uint64_t c, 
                              uint64_t d, uint64_t e, uint64_t f,
                              uint64_t g, uint64_t h, uint64_t i, uint64_t j) {
    /* Multi-precision arithmetic that might use 10+ operands */
    uint64_t t1 = a * b;
    uint64_t t2 = c * d;
    uint64_t t3 = e * f;
    uint64_t t4 = g * h;
    uint64_t t5 = i * j;
    
    /* Complex expression that might expand to many operations */
    return ((t1 >> 32) * (t2 >> 32)) + 
           ((t3 >> 32) * (t4 >> 32)) + 
           (t5 >> 32);
}

/* Vector operations that might use many operands */
NOINLINE
v4si vector_permute_ops(v4si a, v4si b, v4si c, v4si d,
                        v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector permutation/arithmetic */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = e * f;
    v4si t4 = g / (h + (v4si){1,1,1,1});
    
    /* Mix operations - might expand to many operands */
    return (t1 & t2) | (t3 ^ t4);
}

/* Target-specific inline assembly with many operands */
NOINLINE
uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                               uint64_t d, uint64_t e, uint64_t f,
                               uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result;
    
#ifdef __x86_64__
    /* Extended inline assembly with 10 operands (9 inputs + 1 output) */
    asm volatile (
        "imulq %[b], %[a]\n\t"
        "addq %[c], %[a]\n\t"
        "imulq %[e], %[d]\n\t"
        "addq %[f], %[d]\n\t"
        "imulq %[h], %[g]\n\t"
        "addq %[i], %[g]\n\t"
        "xorq %[d], %[a]\n\t"
        "xorq %[g], %[a]\n\t"
        : [a] "+r" (a), [d] "+r" (d), [g] "+r" (g), "=r" (result)
        : [b] "r" (b), [c] "r" (c), [e] "r" (e), [f] "r" (f),
          [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    result = a;
#elif defined(__aarch64__)
    /* ARM-specific with many operands */
    asm volatile (
        "mul %x[a], %x[a], %x[b]\n\t"
        "add %x[a], %x[a], %x[c]\n\t"
        "mul %x[d], %x[d], %x[e]\n\t"
        "add %x[d], %x[d], %x[f]\n\t"
        "mul %x[g], %x[g], %x[h]\n\t"
        "add %x[g], %x[g], %x[i]\n\t"
        "eor %x[a], %x[a], %x[d]\n\t"
        "eor %x[a], %x[a], %x[g]\n\t"
        : [a] "+r" (a), [d] "+r" (d), [g] "+r" (g), "=r" (result)
        : [b] "r" (b), [c] "r" (c), [e] "r" (e), [f] "r" (f),
          [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    result = a;
#else
    /* Generic fallback */
    result = a * b + c + d * e + f + g * h + i;
#endif
    
    return result;
}

/* Another variant with exactly 11 operands */
NOINLINE
uint64_t eleven_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                            uint64_t d, uint64_t e, uint64_t f,
                            uint64_t g, uint64_t h, uint64_t i,
                            uint64_t j, uint64_t k) {
    uint64_t result;
    
#ifdef __x86_64__
    /* 11 operands: 10 inputs + 1 output */
    asm volatile (
        "movq %[a], %%rax\n\t"
        "mulq %[b]\n\t"
        "addq %[c], %%rax\n\t"
        "adcq %[d], %%rdx\n\t"
        "addq %[e], %%rax\n\t"
        "adcq %[f], %%rdx\n\t"
        "addq %[g], %%rax\n\t"
        "adcq %[h], %%rdx\n\t"
        "addq %[i], %%rax\n\t"
        "adcq %[j], %%rdx\n\t"
        "addq %[k], %%rax\n\t"
        "movq %%rax, %[result]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "rax", "rdx", "cc"
    );
#elif defined(__aarch64__)
    /* ARM 64-bit with 11 operands */
    asm volatile (
        "mul %x[tmp1], %x[a], %x[b]\n\t"
        "add %x[tmp1], %x[tmp1], %x[c]\n\t"
        "add %x[tmp1], %x[tmp1], %x[d]\n\t"
        "mul %x[tmp2], %x[e], %x[f]\n\t"
        "add %x[tmp2], %x[tmp2], %x[g]\n\t"
        "add %x[tmp2], %x[tmp2], %x[h]\n\t"
        "mul %x[result], %x[i], %x[j]\n\t"
        "add %x[result], %x[result], %x[k]\n\t"
        "add %x[result], %x[result], %x[tmp1]\n\t"
        "add %x[result], %x[result], %x[tmp2]\n\t"
        : [result] "=r" (result), [tmp1] "=&r" (a), [tmp2] "=&r" (b)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
#else
    result = a * b + c + d + e * f + g + h + i * j + k;
#endif
    
    return result;
}

/* Use compiler builtins that might expand to many operands */
NOINLINE
long builtin_operations(long a, long b, long c, long d,
                        long e, long f, long g, long h,
                        long i, long j) {
    /* Chain of builtins/arithmetic */
    long t1 = __builtin_mul_overflow(a, b, &a) ? 0 : a;
    long t2 = __builtin_add_overflow(c, d, &c) ? 0 : c;
    long t3 = __builtin_sub_overflow(e, f, &e) ? 0 : e;
    long t4 = __builtin_mul_overflow(g, h, &g) ? 0 : g;
    
    /* Complex expression */
    return (t1 & t2) | (t3 ^ t4) + i * j;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t sum = 0;
    
    /* Initialize operands with different values */
    uint64_t vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = (uint64_t)(argc + i * 3);
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic expansion */
        for (int iter = 0; iter < (argc % 10); iter++) {
            sum += complex_mul_highpart(vals[0], vals[1], vals[2], vals[3],
                                        vals[4], vals[5], vals[6], vals[7],
                                        vals[8], vals[9]);
        }
        
        /* Path 2: Inline assembly with many operands */
        if (argc > 2) {
            sum += x86_multi_operand_asm(vals[0], vals[1], vals[2], vals[3],
                                         vals[4], vals[5], vals[6], vals[7],
                                         vals[8]);
        }
    } else {
        /* Path 3: 11-operand inline assembly */
        sum += eleven_operand_asm(vals[0], vals[1], vals[2], vals[3],
                                  vals[4], vals[5], vals[6], vals[7],
                                  vals[8], vals[9], vals[10]);
        
        /* Path 4: Builtin operations */
        sum += builtin_operations(vals[0], vals[1], vals[2], vals[3],
                                  vals[4], vals[5], vals[6], vals[7],
                                  vals[8], vals[9]);
    }
    
    /* Vector operations - might trigger different expansions */
    v4si vec1 = {vals[0], vals[1], vals[2], vals[3]};
    v4si vec2 = {vals[4], vals[5], vals[6], vals[7]};
    v4si vec3 = {vals[8], vals[9], vals[10], vals[11]};
    v4si vec4 = {vals[12], vals[13], vals[14], vals[15]};
    
    v4si vec_result = vector_permute_ops(vec1, vec2, vec3, vec4,
                                         vec1, vec2, vec3, vec4);
    
    /* Use the result to prevent dead code elimination */
    for (int i = 0; i < 4; i++) {
        sum += vec_result[i];
    }
    
    printf("Result: %lu\n", (unsigned long)sum);
    
    return (int)(sum % 256);
}
