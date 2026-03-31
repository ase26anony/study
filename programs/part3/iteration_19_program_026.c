/* test_optabs_10_11_operands.c
 * 
 * This program attempts to trigger GCC's RTL expansion for 10 and 11-operand
 * instruction patterns to cover the uncovered blocks in optabs.cc.
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

/* Complex arithmetic that might expand to many operations */
NOINLINE static uint64_t complex_mul_highpart(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j) {
    /* Multi-precision arithmetic that might use expand_mult_highpart */
    uint64_t t1 = a * b;
    uint64_t t2 = c * d;
    uint64_t t3 = e * f;
    uint64_t t4 = g * h;
    uint64_t t5 = i * j;
    
    /* Complex combination that might require many operands */
    return ((t1 >> 32) + (t2 >> 32) + (t3 >> 32) + (t4 >> 32) + (t5 >> 32));
}

/* Function using inline assembly with many operands */
NOINLINE static uint64_t asm_many_operands(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i,
                                           uint64_t j) {
    uint64_t result;
    
#ifdef __x86_64__
    /* x86_64 inline assembly with 10 operands */
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "imulq %[d], %[e]\n\t"
        "addq %[f], %[e]\n\t"
        "imulq %[g], %[h]\n\t"
        "addq %[i], %[h]\n\t"
        "addq %[b], %[h]\n\t"
        "addq %[e], %[h]\n\t"
        "addq %[j], %[h]\n\t"
        "movq %[h], %[result]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
#elif defined(__aarch64__)
    /* AArch64 inline assembly with many operands */
    asm volatile (
        "mul %[b], %[a], %[b]\n\t"
        "add %[b], %[b], %[c]\n\t"
        "mul %[e], %[d], %[e]\n\t"
        "add %[e], %[e], %[f]\n\t"
        "mul %[h], %[g], %[h]\n\t"
        "add %[h], %[h], %[i]\n\t"
        "add %[h], %[h], %[b]\n\t"
        "add %[h], %[h], %[e]\n\t"
        "add %[result], %[h], %[j]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a * b + c + d * e + f + g * h + i + j;
#endif
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_permute_ops(v4si a, v4si b, v4si c, v4si d,
                                        v4si mask1, v4si mask2, v4si mask3,
                                        v4si mask4, v4si mask5, v4si mask6) {
    /* Complex vector permutation/selection */
    v4si t1 = __builtin_shuffle(a, b, mask1);
    v4si t2 = __builtin_shuffle(c, d, mask2);
    v4si t3 = __builtin_shuffle(t1, t2, mask3);
    v4si t4 = __builtin_shuffle(a, c, mask4);
    v4si t5 = __builtin_shuffle(b, d, mask5);
    v4si result = __builtin_shuffle(t3, t4, mask6);
    
    /* Additional arithmetic to force expansion */
    result = result + t5;
    result = result * a;
    result = result - b;
    result = result & c;
    result = result | d;
    
    return result;
}

/* Function that mixes many operations in one expression */
NOINLINE static uint64_t mixed_many_ops(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j, uint64_t k) {
    /* Expression with 11 operands that might expand to a single RTL pattern */
    uint64_t result = ((a * b) + (c * d) + (e * f) + (g * h) + (i * j)) / k;
    
    /* Add more complex operations to increase operand count */
    result = (result << (a & 7)) | (result >> (64 - (a & 7)));
    result = result ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k;
    
    return result;
}

/* Try to trigger expand_mult_highpart with many operands */
NOINLINE static uint64_t test_mult_highpart(uint64_t a, uint64_t b, uint64_t c,
                                            uint64_t d, uint64_t e, uint64_t f,
                                            uint64_t g, uint64_t h, uint64_t i,
                                            uint64_t j, uint64_t k) {
    /* Multiple high-part multiplications */
    uint64_t h1 = ((a * b) >> 32);
    uint64_t h2 = ((c * d) >> 32);
    uint64_t h3 = ((e * f) >> 32);
    uint64_t h4 = ((g * h) >> 32);
    uint64_t h5 = ((i * j) >> 32);
    
    /* Combine with 11th operand */
    return (h1 + h2 + h3 + h4 + h5) * k;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize operands with different values to avoid constant folding */
    uint64_t ops[11];
    for (int i = 0; i < 11; i++) {
        ops[i] = (uint64_t)(argc + i + 1) * 123456789;
    }
    
    /* Vector operands */
    v4si vec_a = {ops[0], ops[1], ops[2], ops[3]};
    v4si vec_b = {ops[4], ops[5], ops[6], ops[7]};
    v4si vec_c = {ops[8], ops[9], ops[0], ops[1]};
    v4si vec_d = {ops[2], ops[3], ops[4], ops[5]};
    v4si mask1 = {1, 0, 3, 2};
    v4si mask2 = {2, 3, 0, 1};
    v4si mask3 = {3, 2, 1, 0};
    v4si mask4 = {0, 1, 2, 3};
    v4si mask5 = {1, 2, 3, 0};
    v4si mask6 = {2, 1, 0, 3};
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex multiplication with high parts */
        for (int i = 0; i < (argc % 5); i++) {
            result += complex_mul_highpart(ops[0], ops[1], ops[2], ops[3],
                                          ops[4], ops[5], ops[6], ops[7],
                                          ops[8], ops[9]);
        }
        
        /* Path 2: Inline assembly with many operands */
        if (argc > 2) {
            result += asm_many_operands(ops[0], ops[1], ops[2], ops[3],
                                       ops[4], ops[5], ops[6], ops[7],
                                       ops[8], ops[9]);
        }
    } else {
        /* Path 3: Mixed operations with 11 operands */
        result = mixed_many_ops(ops[0], ops[1], ops[2], ops[3], ops[4],
                               ops[5], ops[6], ops[7], ops[8], ops[9],
                               ops[10]);
        
        /* Path 4: Vector operations */
        v4si vec_result = vector_permute_ops(vec_a, vec_b, vec_c, vec_d,
                                            mask1, mask2, mask3, mask4,
                                            mask5, mask6);
        for (int i = 0; i < 4; i++) {
            result += vec_result[i];
        }
        
        /* Path 5: Test mult_highpart with 11 operands */
        if (argc == 0) {  /* Always false, but compiler doesn't know */
            result += test_mult_highpart(ops[0], ops[1], ops[2], ops[3],
                                        ops[4], ops[5], ops[6], ops[7],
                                        ops[8], ops[9], ops[10]);
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)result);
    
    return (int)(result % 256);
}
