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
                                                  uint64_t i, uint64_t j)
{
    /* Complex expression that might require many temporary operands */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    return ((t1 + t2) * (t3 + t4)) ^ t5;
}

/* Function using extended inline assembly with many operands */
NOINLINE static uint64_t extended_asm_10_operands(uint64_t a, uint64_t b,
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j)
{
    uint64_t result;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with 10 operands */
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "subq %3, %%rax\n\t"
        "imulq %4, %%rax\n\t"
        "addq %5, %%rax\n\t"
        "subq %6, %%rax\n\t"
        "imulq %7, %%rax\n\t"
        "addq %8, %%rax\n\t"
        "subq %9, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "rax", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "add %x[res], %x[a], %x[b]\n\t"
        "sub %x[res], %x[res], %x[c]\n\t"
        "mul %x[res], %x[res], %x[d]\n\t"
        "add %x[res], %x[res], %x[e]\n\t"
        "sub %x[res], %x[res], %x[f]\n\t"
        "mul %x[res], %x[res], %x[g]\n\t"
        "add %x[res], %x[res], %x[h]\n\t"
        "sub %x[res], %x[res], %x[i]\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a + b - c * d + e - f * g + h - i;
#endif
    
    return result + j; /* 11th operand used outside asm */
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                          v4si e, v4si f, v4si g, v4si h)
{
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = g * h;
    
    /* Permutation-like operation */
    v4si result;
    
    /* Manual shuffle that might expand to many operations */
    result[0] = t1[0] + t2[3] + t3[1];
    result[1] = t1[1] + t2[2] + t3[0];
    result[2] = t1[2] + t2[1] + t3[3];
    result[3] = t1[3] + t2[0] + t3[2];
    
    return result;
}

/* Function using compiler builtins that might expand to many operands */
NOINLINE static uint64_t builtin_multi_operand(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d)
{
    /* Use builtins that might decompose to many RTL operands */
    uint64_t result = 0;
    
    /* __builtin_clzll might expand differently on different arches */
    result += __builtin_clzll(a);
    result += __builtin_ctzll(b);
    result += __builtin_popcountll(c);
    result += __builtin_parityll(d);
    
    /* Complex bit manipulation */
    result = ((result << 32) | (result >> 32));
    result = ((result & 0xAAAAAAAAAAAAAAAA) >> 1) | 
             ((result & 0x5555555555555555) << 1);
    
    return result;
}

/* Multi-precision arithmetic that might need many operands */
NOINLINE static void wide_multiply(uint64_t a, uint64_t b,
                                   uint64_t *hi, uint64_t *lo)
{
    /* 128-bit multiplication using 64-bit parts */
    uint64_t a_lo = a & 0xFFFFFFFF;
    uint64_t a_hi = a >> 32;
    uint64_t b_lo = b & 0xFFFFFFFF;
    uint64_t b_hi = b >> 32;
    
    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;
    
    uint64_t carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);
    
    *lo = (p0 & 0xFFFFFFFF) | (carry << 32);
    *hi = p3 + (p1 >> 32) + (p2 >> 32) + (carry >> 32);
}

/* Main test function */
NOINLINE static uint64_t test_multi_operand_expansion(int variant, 
                                                     uint64_t seed)
{
    uint64_t a = seed + 1;
    uint64_t b = seed + 2;
    uint64_t c = seed + 3;
    uint64_t d = seed + 4;
    uint64_t e = seed + 5;
    uint64_t f = seed + 6;
    uint64_t g = seed + 7;
    uint64_t h = seed + 8;
    uint64_t i = seed + 9;
    uint64_t j = seed + 10;
    
    uint64_t result = 0;
    
    switch (variant % 4) {
        case 0:
            /* Test complex arithmetic expansion */
            result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
            break;
            
        case 1:
            /* Test inline assembly with many operands */
            result = extended_asm_10_operands(a, b, c, d, e, f, g, h, i, j);
            break;
            
        case 2:
            /* Test builtin expansions */
            result = builtin_multi_operand(a, b, c, d);
            result += builtin_multi_operand(e, f, g, h);
            break;
            
        case 3:
            /* Test multi-precision arithmetic */
            {
                uint64_t hi1, lo1, hi2, lo2;
                wide_multiply(a, b, &hi1, &lo1);
                wide_multiply(c, d, &hi2, &lo2);
                result = hi1 + lo1 + hi2 + lo2 + e + f + g + h + i + j;
            }
            break;
    }
    
    return result;
}

int main(int argc, char *argv[])
{
    uint64_t total = 0;
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Test different expansion paths */
    for (int i = 0; i < iterations; i++) {
        /* Vary the test variant based on iteration */
        int variant = i % 4;
        
        /* Use argc to influence the seed, preventing constant folding */
        uint64_t seed = (uint64_t)(argc + i) * 123456789;
        
        /* Call the multi-operand test function */
        uint64_t result = test_multi_operand_expansion(variant, seed);
        
        /* Accumulate to prevent dead code elimination */
        total += result;
        
        /* Add some branching to influence optimization */
        if (result % 2 == 0) {
            total += 1;
        }
    }
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %lu\n", (unsigned long)total);
    
    /* Additional test with vector types */
    if (argc > 2) {
        v4si vec1 = {1, 2, 3, 4};
        v4si vec2 = {5, 6, 7, 8};
        v4si vec3 = {9, 10, 11, 12};
        v4si vec4 = {13, 14, 15, 16};
        v4si vec5 = {17, 18, 19, 20};
        v4si vec6 = {21, 22, 23, 24};
        v4si vec7 = {25, 26, 27, 28};
        v4si vec8 = {29, 30, 31, 32};
        
        v4si vec_result = vector_multi_operand(vec1, vec2, vec3, vec4,
                                              vec5, vec6, vec7, vec8);
        
        /* Use vector result to prevent optimization */
        total += vec_result[0] + vec_result[1] + 
                 vec_result[2] + vec_result[3];
        
        printf("Vector result sum: %lu\n", (unsigned long)total);
    }
    
    return (int)(total % 256);
}
