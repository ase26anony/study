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

/* Prevent inlining to ensure local expansion */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to multi-operand RTL */
NOINLINE
uint64_t complex_mul_highpart(uint64_t a, uint64_t b, uint64_t c,
                              uint64_t d, uint64_t e, uint64_t f,
                              uint64_t g, uint64_t h, uint64_t i,
                              uint64_t j) {
    /* Multi-precision arithmetic that might use expand_mult_highpart */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Chain operations to potentially create multi-operand pattern */
    return t1 + t2 + t3 + t4 + t5;
}

/* Function using inline assembly with many operands */
NOINLINE
int64_t multi_operand_asm(int64_t a, int64_t b, int64_t c, int64_t d,
                          int64_t e, int64_t f, int64_t g, int64_t h,
                          int64_t i, int64_t j) {
    int64_t result;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with 10 operands */
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "subq %[d], %[b]\n\t"
        "xorq %[e], %[b]\n\t"
        "orq %[f], %[b]\n\t"
        "andq %[g], %[b]\n\t"
        "shlq $3, %[b]\n\t"
        "shrq $2, %[b]\n\t"
        "movq %[b], %[result]\n\t"
        "addq %[h], %[result]\n\t"
        "subq %[i], %[result]\n\t"
        "addq %[j], %[result]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        "mul %x[result], %x[a], %x[b]\n\t"
        "add %x[result], %x[result], %x[c]\n\t"
        "sub %x[result], %x[result], %x[d]\n\t"
        "eor %x[result], %x[result], %x[e]\n\t"
        "orr %x[result], %x[result], %x[f]\n\t"
        "and %x[result], %x[result], %x[g]\n\t"
        "lsl %x[result], %x[result], #3\n\t"
        "lsr %x[result], %x[result], #2\n\t"
        "add %x[result], %x[result], %x[h]\n\t"
        "sub %x[result], %x[result], %x[i]\n\t"
        "add %x[result], %x[result], %x[j]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a * b + c - d ^ e | f & g;
    result = (result << 3) >> 2;
    result = result + h - i + j;
#endif
    
    return result;
}

/* Vector operations that might expand to multi-operand patterns */
NOINLINE
v4si vector_operations(v4si a, v4si b, v4si c, v4si d,
                       v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression that might use many operands */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = g * h;
    v4si t4 = t1 >> 2;
    v4si t5 = t2 << 1;
    v4si t6 = t3 & a;
    
    return t4 + t5 + t6 + b + c + d;
}

/* Function that mixes different operation types */
NOINLINE
int64_t mixed_operations(int argc, char **argv) {
    int64_t result = 0;
    
    /* Create many variables to force register pressure */
    int64_t v1 = (int64_t)argc;
    int64_t v2 = v1 * 2;
    int64_t v3 = v2 + 1;
    int64_t v4 = v3 << 3;
    int64_t v5 = v4 >> 2;
    int64_t v6 = v5 ^ 0x1234;
    int64_t v7 = v6 | 0x5678;
    int64_t v8 = v7 & 0x9ABC;
    int64_t v9 = v8 + v1;
    int64_t v10 = v9 - v2;
    int64_t v11 = v10 * v3;
    
    /* Call functions that might generate multi-operand RTL */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        result = complex_mul_highpart(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    } else if (argc > 2) {
        /* Path 2: Inline assembly */
        result = multi_operand_asm(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    } else {
        /* Path 3: Mixed operations */
        v4si vec1 = {v1, v2, v3, v4};
        v4si vec2 = {v5, v6, v7, v8};
        v4si vec3 = {v9, v10, v11, v1};
        v4si vec4 = {v2, v3, v4, v5};
        v4si vec5 = {v6, v7, v8, v9};
        v4si vec6 = {v10, v11, v1, v2};
        v4si vec7 = {v3, v4, v5, v6};
        v4si vec8 = {v7, v8, v9, v10};
        
        v4si vec_result = vector_operations(vec1, vec2, vec3, vec4,
                                           vec5, vec6, vec7, vec8);
        
        /* Extract result from vector */
        int *vr = (int*)&vec_result;
        result = vr[0] + vr[1] + vr[2] + vr[3];
    }
    
    /* Additional computation to prevent dead code elimination */
    result += v11;
    
    /* Force use of all variables */
    volatile int64_t dummy = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
    (void)dummy;
    
    return result;
}

/* Main function with different optimization contexts */
int main(int argc, char **argv) {
    int64_t total = 0;
    
    /* Loop to increase coverage chances */
    for (int i = 0; i < (argc > 0 ? argc : 1); i++) {
        /* Call with different arguments to explore different paths */
        total += mixed_operations(argc + i, argv);
        
        /* Alternate between different operation sets */
        if (i % 2 == 0) {
            /* Direct complex computation */
            uint64_t a = (uint64_t)argc + i;
            uint64_t b = a * 3;
            uint64_t c = b + 5;
            uint64_t d = c << 2;
            uint64_t e = d >> 1;
            uint64_t f = e ^ 0xF0F0;
            uint64_t g = f | 0x0F0F;
            uint64_t h = g & 0x3333;
            uint64_t j = h + a;
            uint64_t k = j - b;
            
            total += complex_mul_highpart(a, b, c, d, e, f, g, h, j, k);
        }
    }
    
    printf("Result: %ld\n", (long)total);
    return 0;
}
