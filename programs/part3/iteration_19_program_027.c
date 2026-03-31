/* test_optabs_10_11_operands.c
 * 
 * This program attempts to trigger GCC's RTL expansion for
 * 10 and 11-operand instruction patterns in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the generated .expand file for multi-operand patterns.
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

/* Complex arithmetic that might expand to multi-operand RTL */
NOINLINE static uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, 
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j) {
    /* Complex expression that might require many temporaries */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    uint64_t result = (t1 + t2) * (t3 + t4) + t5;
    result = result ^ (a + b + c + d + e + f + g + h + i + j);
    
    /* Force use of all inputs */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e),
                       "+r"(f), "+r"(g), "+r"(h), "+r"(i), "+r"(j));
    
    return result;
}

/* Multi-precision multiplication (64x64 to 128) */
NOINLINE static void mul64x64_128(uint64_t a, uint64_t b, 
                                  uint64_t *hi, uint64_t *lo) {
#ifdef __x86_64__
    /* On x86_64, this might expand to multi-operand pattern */
    __uint128_t result = (__uint128_t)a * (__uint128_t)b;
    *hi = (uint64_t)(result >> 64);
    *lo = (uint64_t)result;
#else
    /* Generic implementation that might also expand to many operations */
    uint64_t a_lo = (uint32_t)a;
    uint64_t a_hi = a >> 32;
    uint64_t b_lo = (uint32_t)b;
    uint64_t b_hi = b >> 32;
    
    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;
    
    uint64_t carry = (uint64_t)(uint32_t)p1 + (uint64_t)(uint32_t)p2;
    carry >>= 32;
    
    *lo = p0 + ((p1 + p2) << 32);
    *hi = p3 + (p1 >> 32) + (p2 >> 32) + carry;
#endif
}

/* Vector operations that might require many operands */
NOINLINE static v4si vector_permute_operation(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector permutation */
    v4si result;
    
#ifdef __x86_64__
    /* Use inline assembly with many operands */
    asm volatile (
        "movdqa %[a], %%xmm0\n\t"
        "movdqa %[b], %%xmm1\n\t"
        "movdqa %[c], %%xmm2\n\t"
        "movdqa %[d], %%xmm3\n\t"
        "paddd %%xmm1, %%xmm0\n\t"
        "paddd %%xmm3, %%xmm2\n\t"
        "psubd %%xmm2, %%xmm0\n\t"
        "movdqa %%xmm0, %[result]\n\t"
        : [result] "=x" (result)
        : [a] "x" (a), [b] "x" (b), [c] "x" (c), [d] "x" (d)
        : "xmm0", "xmm1", "xmm2", "xmm3"
    );
#elif defined(__aarch64__)
    /* ARM NEON with many operands */
    asm volatile (
        "add v0.4s, %0.4s, %1.4s\n\t"
        "add v1.4s, %2.4s, %3.4s\n\t"
        "sub v0.4s, v0.4s, v1.4s\n\t"
        "mov %4.16b, v0.16b\n\t"
        : "=w" (result)
        : "w" (a), "w" (b), "w" (c), "w" (d)
        : "v0", "v1"
    );
#else
    /* Generic implementation */
    result = a + b - c - d;
#endif
    
    return result;
}

/* Extended inline assembly with many operands */
NOINLINE static uint64_t extended_asm_10_operands(
    uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e,
    uint64_t f, uint64_t g, uint64_t h, uint64_t i, uint64_t j) {
    
    uint64_t result;
    
#ifdef __x86_64__
    /* 10-operand inline assembly */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "mov %[b], %%rax\n\t"
        "add %%rax, %[d]\n\t"
        "add %[d], %[f]\n\t"
        "add %[f], %[h]\n\t"
        "add %[h], %[j]\n\t"
        "mov %[j], %[result]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax"
    );
#elif defined(__aarch64__)
    /* ARM version with many operands */
    asm volatile (
        "add %[b], %[b], %[a]\n\t"
        "add %[d], %[d], %[c]\n\t"
        "add %[f], %[f], %[e]\n\t"
        "add %[h], %[h], %[g]\n\t"
        "add %[j], %[j], %[i]\n\t"
        "add %[d], %[d], %[b]\n\t"
        "add %[f], %[f], %[d]\n\t"
        "add %[h], %[h], %[f]\n\t"
        "add %[j], %[j], %[h]\n\t"
        "mov %[result], %[j]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        :
    );
#else
    /* Generic fallback */
    result = a + b + c + d + e + f + g + h + i + j;
#endif
    
    return result;
}

/* Try to trigger 11-operand case */
NOINLINE static uint64_t extended_asm_11_operands(
    uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e,
    uint64_t f, uint64_t g, uint64_t h, uint64_t i, uint64_t j,
    uint64_t k) {
    
    uint64_t result;
    
#ifdef __x86_64__
    /* 11-operand inline assembly */
    asm volatile (
        "mov %[k], %%rax\n\t"
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "add %%rax, %[b]\n\t"
        "add %[b], %[d]\n\t"
        "add %[d], %[f]\n\t"
        "add %[f], %[h]\n\t"
        "add %[h], %[j]\n\t"
        "mov %[j], %[result]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "rax"
    );
#else
    result = a + b + c + d + e + f + g + h + i + j + k;
#endif
    
    return result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize many variables to use as operands */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (uint64_t)(i + argc);
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        result = multi_operand_arithmetic(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
        
        /* Multi-precision multiplication */
        uint64_t hi, lo;
        mul64x64_128(vars[10], vars[11], &hi, &lo);
        result ^= hi ^ lo;
    } else {
        /* Path 2: Vector operations */
        v4si vec_a = {vars[0], vars[1], vars[2], vars[3]};
        v4si vec_b = {vars[4], vars[5], vars[6], vars[7]};
        v4si vec_c = {vars[8], vars[9], vars[10], vars[11]};
        v4si vec_d = {vars[12], vars[13], vars[14], vars[15]};
        
        v4si vec_result = vector_permute_operation(vec_a, vec_b, vec_c, vec_d);
        
        /* Sum vector elements */
        int *ptr = (int*)&vec_result;
        for (int i = 0; i < 4; i++) {
            result += ptr[i];
        }
    }
    
    /* Always test extended assembly operations */
    uint64_t asm_result_10 = extended_asm_10_operands(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    uint64_t asm_result_11 = extended_asm_11_operands(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9],
        vars[10]);
    
    result += asm_result_10 + asm_result_11;
    
    /* Loop with varying data to increase coverage */
    for (int i = 0; i < (argc % 5) + 1; i++) {
        result ^= multi_operand_arithmetic(
            result, vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return (int)(result % 256);
}
