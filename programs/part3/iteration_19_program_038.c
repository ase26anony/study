/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the 10 and 11 operand cases in optabs.cc
 * by generating complex operations that expand to multi-operand RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might simplify our complex operations */
#define NOOPT __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOOPT uint64_t complex_mul_highpart(uint64_t a, uint64_t b, uint64_t c, 
                                     uint64_t d, uint64_t e, uint64_t f,
                                     uint64_t g, uint64_t h, uint64_t i) {
    /* Multi-precision arithmetic that might use expand_mult_highpart */
    uint64_t t1 = a * b;
    uint64_t t2 = c * d;
    uint64_t t3 = e * f;
    uint64_t t4 = g * h;
    
    /* Complex expression that might need many temporaries */
    return ((t1 * t2) >> 32) + ((t3 * t4) >> 32) + (i * i) >> 32;
}

/* Function using inline assembly with many operands */
NOOPT uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                 uint64_t d, uint64_t e, uint64_t f,
                                 uint64_t g, uint64_t h, uint64_t i,
                                 uint64_t j) {
    uint64_t result1, result2;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with many operands */
    asm volatile (
        /* Complex operation using many registers */
        "movq %[a], %%rax\n\t"
        "mulq %[b]\n\t"
        "movq %%rax, %[r1]\n\t"
        "movq %%rdx, %[r2]\n\t"
        "movq %[c], %%rax\n\t"
        "mulq %[d]\n\t"
        "addq %[r1], %%rax\n\t"
        "adcq %[r2], %%rdx\n\t"
        "movq %%rax, %[r1]\n\t"
        "movq %%rdx, %[r2]\n\t"
        "movq %[e], %%rax\n\t"
        "mulq %[f]\n\t"
        "addq %[r1], %%rax\n\t"
        "adcq $0, %%rdx\n\t"
        "addq %[g], %%rax\n\t"
        "adcq %[h], %%rdx\n\t"
        "addq %[i], %%rax\n\t"
        "adcq %[j], %%rdx\n\t"
        "movq %%rax, %[r1]\n\t"
        "movq %%rdx, %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "rax", "rdx", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        /* Complex 128-bit arithmetic using many registers */
        "umulh %[r2], %[a], %[b]\n\t"
        "mul   %[r1], %[a], %[b]\n\t"
        "umulh x9, %[c], %[d]\n\t"
        "mul   x10, %[c], %[d]\n\t"
        "adds  %[r1], %[r1], x10\n\t"
        "adc   %[r2], %[r2], x9\n\t"
        "umulh x9, %[e], %[f]\n\t"
        "mul   x10, %[e], %[f]\n\t"
        "adds  %[r1], %[r1], x10\n\t"
        "adc   %[r2], %[r2], x9\n\t"
        "adds  %[r1], %[r1], %[g]\n\t"
        "adc   %[r2], %[r2], %[h]\n\t"
        "adds  %[r1], %[r1], %[i]\n\t"
        "adc   %[r2], %[r2], %[j]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "x9", "x10", "cc"
    );
#else
    /* Generic fallback - still complex arithmetic */
    result1 = a * b + c * d + e * f + g + i;
    result2 = ((a * b) >> 32) + ((c * d) >> 32) + h + j;
#endif
    
    return result1 + result2;
}

/* Vector operations that might expand to multi-operand patterns */
NOOPT v4si vector_complex_op(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector operations */
    v4si t1 = a * b;
    v4si t2 = c * d;
    v4si t3 = (a + b) * (c - d);
    v4si t4 = (a >> 2) | (b << 2);
    
    /* This complex expression might need many operands during expansion */
    return (t1 + t2) * (t3 - t4) + (a & b) | (c ^ d);
}

/* Function that uses many parameters and local variables */
NOOPT uint64_t many_operand_function(uint64_t p1, uint64_t p2, uint64_t p3,
                                     uint64_t p4, uint64_t p5, uint64_t p6,
                                     uint64_t p7, uint64_t p8, uint64_t p9,
                                     uint64_t p10) {
    /* Force register pressure and complex expansions */
    uint64_t v1 = p1 * p2;
    uint64_t v2 = p3 * p4;
    uint64_t v3 = p5 * p6;
    uint64_t v4 = p7 * p8;
    uint64_t v5 = p9 * p10;
    
    /* Complex chain of operations */
    uint64_t r1 = (v1 >> 32) + (v2 >> 32);
    uint64_t r2 = (v3 >> 32) + (v4 >> 32);
    uint64_t r3 = (v5 >> 32);
    
    /* Mix everything together */
    return ((r1 * r2) >> 16) + ((r2 * r3) >> 16) + ((r3 * r1) >> 16) +
           ((p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10) & 0xFFFF);
}

/* Try to trigger expand_mult_highpart with constant divisor */
NOOPT uint64_t division_by_constant(uint64_t a, uint64_t b) {
    /* Division by constants often expands to multi-operand sequences */
    uint64_t t1 = a / 7;    /* Might use expand_divmod */
    uint64_t t2 = b / 13;   /* Might use expand_divmod */
    uint64_t t3 = a % 19;   /* Might use expand_divmod */
    uint64_t t4 = b % 23;   /* Might use expand_divmod */
    
    return (t1 * t2) + (t3 * t4);
}

int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize many variables to use as operands */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (uint64_t)(argc + i) * 0x123456789ABCDEFULL;
    }
    
    /* Different code paths based on arguments */
    if (argc > 1) {
        /* Path 1: Complex arithmetic with many operands */
        for (int i = 0; i < (argc % 10); i++) {
            result ^= complex_mul_highpart(
                vars[0] + i, vars[1] + i, vars[2] + i,
                vars[3] + i, vars[4] + i, vars[5] + i,
                vars[6] + i, vars[7] + i, vars[8] + i
            );
        }
        
        /* Use inline assembly with many operands */
        result += multi_operand_asm(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]
        );
    } else {
        /* Path 2: Vector operations and division by constants */
        v4si vec_a = {vars[0], vars[1], vars[2], vars[3]};
        v4si vec_b = {vars[4], vars[5], vars[6], vars[7]};
        v4si vec_c = {vars[8], vars[9], vars[10], vars[11]};
        v4si vec_d = {vars[12], vars[13], vars[14], vars[15]};
        
        v4si vec_result = vector_complex_op(vec_a, vec_b, vec_c, vec_d);
        result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        /* Division by constants */
        result += division_by_constant(vars[16], vars[17]);
    }
    
    /* Always call the many-operand function */
    result += many_operand_function(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9]
    );
    
    /* Mix in some more operations to prevent dead code elimination */
    for (int i = 0; i < argc; i++) {
        result = (result << 5) | (result >> 59);  /* Rotate right */
        result ^= vars[i % 20];
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return (int)(result & 0x7FFFFFFF);
}
