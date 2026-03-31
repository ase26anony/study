/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for 10 and 11 operand cases
 * in optabs.cc lines 8254-8263.
 *
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the .expand file for multi-operand RTL patterns.
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

/* Extended inline assembly with many operands */
NOINLINE static uint64_t extended_asm_10_operands(uint64_t a, uint64_t b,
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j) {
    uint64_t result;
    
#ifdef __x86_64__
    /* x86_64 extended asm with 10 input operands */
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "addq %3, %%rax\n\t"
        "addq %4, %%rax\n\t"
        "addq %5, %%rax\n\t"
        "addq %6, %%rax\n\t"
        "addq %7, %%rax\n\t"
        "addq %8, %%rax\n\t"
        "addq %9, %%rax\n\t"
        "addq %10, %%rax\n\t"
        "movq %%rax, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j)
        : "rax", "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 extended asm with many operands */
    asm volatile (
        "add %x[res], %x[a], %x[b]\n\t"
        "add %x[res], %x[res], %x[c]\n\t"
        "add %x[res], %x[res], %x[d]\n\t"
        "add %x[res], %x[res], %x[e]\n\t"
        "add %x[res], %x[res], %x[f]\n\t"
        "add %x[res], %x[res], %x[g]\n\t"
        "add %x[res], %x[res], %x[h]\n\t"
        "add %x[res], %x[res], %x[i]\n\t"
        "add %x[res], %x[res], %x[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a + b + c + d + e + f + g + h + i + j;
#endif
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                          v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a * b;
    v4si t2 = c * d;
    v4si t3 = e * f;
    v4si t4 = g * h;
    
    /* Mix with shifts and adds - might expand to many RTL operands */
    v4si result = (t1 >> 4) + (t2 >> 3) + (t3 >> 2) + (t4 >> 1);
    
    /* Additional operations to increase operand count */
    result = result * a + result * b;
    result = result | c | d;
    
    return result;
}

/* Multi-precision arithmetic - often expands to many operands */
NOINLINE static uint64_t multi_precision_mul(uint32_t a, uint32_t b, 
                                             uint32_t c, uint32_t d) {
    /* 64-bit result from 32-bit multiplies */
    uint64_t a64 = a;
    uint64_t b64 = b;
    uint64_t c64 = c;
    uint64_t d64 = d;
    
    /* Complex expression that might use highpart multiplication */
    uint64_t t1 = a64 * b64;
    uint64_t t2 = c64 * d64;
    uint64_t t3 = (a64 + b64) * (c64 + d64);
    
    /* Mix with shifts - might trigger expand_mult_highpart */
    return ((t1 >> 32) * (t2 >> 32)) + (t3 >> 16);
}

/* Function with mixed operations to increase coverage */
NOINLINE static uint64_t mixed_operations(int argc, char **argv) {
    uint64_t results[4] = {0};
    
    /* Initialize operands from argv to make them dynamic */
    uint64_t ops[10];
    for (int i = 0; i < 10 && i < argc; i++) {
        ops[i] = (uint64_t)argv[i][0];
    }
    for (int i = argc; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* Try different code paths based on argc */
    if (argc > 5) {
        /* Path 1: Extended inline assembly */
        results[0] = extended_asm_10_operands(ops[0], ops[1], ops[2], ops[3],
                                             ops[4], ops[5], ops[6], ops[7],
                                             ops[8], ops[9]);
    } else if (argc > 3) {
        /* Path 2: Complex arithmetic */
        results[1] = multi_operand_arithmetic(ops[0], ops[1], ops[2], ops[3],
                                             ops[4], ops[5], ops[6], ops[7],
                                             ops[8], ops[9]);
    } else {
        /* Path 3: Multi-precision */
        results[2] = multi_precision_mul((uint32_t)ops[0], (uint32_t)ops[1],
                                        (uint32_t)ops[2], (uint32_t)ops[3]);
    }
    
    /* Vector operations - might trigger different expansion */
    v4si vec_a = {ops[0], ops[1], ops[2], ops[3]};
    v4si vec_b = {ops[4], ops[5], ops[6], ops[7]};
    v4si vec_c = {ops[0] + 1, ops[1] + 1, ops[2] + 1, ops[3] + 1};
    v4si vec_d = {ops[4] + 1, ops[5] + 1, ops[6] + 1, ops[7] + 1};
    v4si vec_e = {ops[0] + 2, ops[1] + 2, ops[2] + 2, ops[3] + 2};
    v4si vec_f = {ops[4] + 2, ops[5] + 2, ops[6] + 2, ops[7] + 2};
    v4si vec_g = {ops[0] + 3, ops[1] + 3, ops[2] + 3, ops[3] + 3};
    v4si vec_h = {ops[4] + 3, ops[5] + 3, ops[6] + 3, ops[7] + 3};
    
    v4si vec_result = vector_multi_operand(vec_a, vec_b, vec_c, vec_d,
                                          vec_e, vec_f, vec_g, vec_h);
    
    /* Use vector result */
    results[3] = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Combine all results */
    return results[0] + results[1] + results[2] + results[3];
}

int main(int argc, char **argv) {
    uint64_t total = 0;
    
    /* Loop to increase coverage with varying inputs */
    for (int iter = 0; iter < (argc > 1 ? argc : 3); iter++) {
        /* Modify argv content slightly each iteration */
        if (argc > 1 && iter > 0) {
            argv[iter % argc][0] += iter;
        }
        
        /* Call function with mixed operations */
        total += mixed_operations(argc, argv);
        
        /* Alternate between different operation sets */
        if (iter % 2 == 0) {
            /* Direct call to multi-operand arithmetic */
            uint64_t ops[10];
            for (int i = 0; i < 10; i++) {
                ops[i] = (i + iter) * 7;
            }
            total += multi_operand_arithmetic(ops[0], ops[1], ops[2], ops[3],
                                             ops[4], ops[5], ops[6], ops[7],
                                             ops[8], ops[9]);
        }
    }
    
    printf("Result: %lu\n", (unsigned long)total);
    return (int)(total & 0xFF);
}
