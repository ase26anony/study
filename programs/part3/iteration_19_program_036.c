/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for 10 and 11-operand
 * instruction patterns, covering the uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -fdump-rtl-combine -c test.c
 * Or for more details: gcc -O3 -fdump-rtl-all -dP -c test.c
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
    
    /* Multi-step computation that could expand to many RTL operands */
    result = a * b + c * d - e * f + g * h * i / j;
    result = (result << 5) | (result >> 59); /* rotate */
    result = result ^ (a + b + c + d + e + f + g + h + i + j);
    
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
    /* x86_64 inline assembly with many operands */
    asm volatile (
        /* Simulate a complex multi-operand operation */
        "movq %1, %%rax\n\t"
        "imulq %2, %%rax\n\t"
        "addq %3, %%rax\n\t"
        "subq %4, %%rax\n\t"
        "xorq %5, %%rax\n\t"
        "orq %6, %%rax\n\t"
        "andq %7, %%rax\n\t"
        "addq %8, %%rax\n\t"
        "subq %9, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "%rax", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with many operands */
    asm volatile (
        /* Complex arithmetic sequence */
        "mul %x[res], %x[a], %x[b]\n\t"
        "add %x[res], %x[res], %x[c]\n\t"
        "sub %x[res], %x[res], %x[d]\n\t"
        "eor %x[res], %x[res], %x[e]\n\t"
        "orr %x[res], %x[res], %x[f]\n\t"
        "and %x[res], %x[res], %x[g]\n\t"
        "add %x[res], %x[res], %x[h]\n\t"
        "sub %x[res], %x[res], %x[i]\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a * b + c - d ^ e | f & g + h - i;
#endif
    
    return result + j; /* 10th operand used outside asm */
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                          v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si result;
    
    /* This might expand to many element-wise operations */
    result = a * b + c * d - e * f * g;
    result = (result << 2) | (result >> 30);
    result = result ^ a ^ b ^ c ^ d ^ e ^ f ^ g;
    
    return result;
}

/* Function that might trigger expand_mult_highpart with many operands */
NOINLINE static uint64_t highpart_multiplication(uint64_t a, uint64_t b,
                                                 uint64_t c, uint64_t d,
                                                 uint64_t e, uint64_t f,
                                                 uint64_t g, uint64_t h) {
    /* Complex multi-precision arithmetic */
    uint64_t t1 = (a * b) >> 32;  /* Might use expand_mult_highpart */
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    
    return t1 + t2 - t3 * t4;
}

/* Mixed operations to increase chances of hitting 10/11 operands */
NOINLINE static uint64_t mixed_operations(int argc, char **argv) {
    uint64_t operands[11];
    uint64_t result = 0;
    
    /* Initialize operands from various sources */
    for (int i = 0; i < 11; i++) {
        operands[i] = (uint64_t)(i + 1);
        if (i < argc && argv[i]) {
            operands[i] ^= (uint64_t)argv[i][0];
        }
    }
    
    /* Try different code paths based on argc */
    if (argc > 10) {
        /* Path 1: Direct call with 10 operands */
        result = inline_asm_10_operands(operands[0], operands[1], operands[2],
                                       operands[3], operands[4], operands[5],
                                       operands[6], operands[7], operands[8],
                                       operands[9]);
    } else if (argc > 5) {
        /* Path 2: Complex arithmetic */
        result = multi_operand_arithmetic(operands[0], operands[1], operands[2],
                                         operands[3], operands[4], operands[5],
                                         operands[6], operands[7], operands[8],
                                         operands[9]);
    } else {
        /* Path 3: Highpart multiplications */
        result = highpart_multiplication(operands[0], operands[1], operands[2],
                                        operands[3], operands[4], operands[5],
                                        operands[6], operands[7]);
        result += operands[8] + operands[9] + operands[10]; /* Use all 11 */
    }
    
    return result;
}

/* Test function with vector operations */
NOINLINE static v4si test_vector_path(int variant) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si e = {17, 18, 19, 20};
    v4si f = {21, 22, 23, 24};
    v4si g = {25, 26, 27, 28};
    
    if (variant == 0) {
        return vector_multi_operand(a, b, c, d, e, f, g);
    } else {
        /* Alternative vector expression */
        return a * b + (c & d) | (e ^ f) * g;
    }
}

int main(int argc, char **argv) {
    uint64_t final_result = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    
    /* Loop to increase coverage chances */
    for (int iteration = 0; iteration < (argc > 1 ? argc : 3); iteration++) {
        /* Call mixed operations with different argument counts */
        uint64_t res1 = mixed_operations(argc, argv);
        
        /* Test vector path */
        v4si vec_res = test_vector_path(iteration % 2);
        uint64_t vec_sum = vec_res[0] + vec_res[1] + vec_res[2] + vec_res[3];
        
        /* Combine results */
        final_result ^= res1 + vec_sum + iteration;
        
        /* Force register pressure with many variables */
        uint64_t r0 = final_result * 3;
        uint64_t r1 = final_result / 5;
        uint64_t r2 = final_result << 2;
        uint64_t r3 = final_result >> 3;
        uint64_t r4 = final_result ^ 0x12345678;
        uint64_t r5 = final_result | 0x87654321;
        uint64_t r6 = final_result & 0xF0F0F0F0;
        uint64_t r7 = final_result + 0x11111111;
        uint64_t r8 = final_result - 0x22222222;
        uint64_t r9 = final_result * 7;
        
        /* Use all variables to prevent optimization */
        final_result = r0 + r1 - r2 * r3 / (r4 + 1) | r5 & r6 ^ r7 + r8 - r9;
    }
    
    printf("Final result: %lu\n", (unsigned long)final_result);
    
    /* Additional test with exactly 10 arguments */
    if (argc > 10) {
        uint64_t direct_res = inline_asm_10_operands(
            (uint64_t)argv[1][0], (uint64_t)argv[2][0],
            (uint64_t)argv[3][0], (uint64_t)argv[4][0],
            (uint64_t)argv[5][0], (uint64_t)argv[6][0],
            (uint64_t)argv[7][0], (uint64_t)argv[8][0],
            (uint64_t)argv[9][0], (uint64_t)argv[10][0]
        );
        printf("Direct 10-operand result: %lu\n", (unsigned long)direct_res);
    }
    
    return (final_result == 0) ? 0 : 1;
}
