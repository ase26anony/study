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

/* Multi-precision multiplication (64x64 -> 128) */
NOINLINE static void wide_multiply(uint64_t a, uint64_t b, 
                                   uint64_t *hi, uint64_t *lo) {
    /* This often expands to multiple operations with many operands */
    uint64_t a_hi = a >> 32;
    uint64_t a_lo = a & 0xFFFFFFFF;
    uint64_t b_hi = b >> 32;
    uint64_t b_lo = b & 0xFFFFFFFF;
    
    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;
    
    uint64_t carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);
    
    *hi = p3 + (p1 >> 32) + (p2 >> 32) + (carry >> 32);
    *lo = (carry << 32) | (p0 & 0xFFFFFFFF);
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_permute(v4si a, v4si b, v4si mask) {
    /* Complex vector permutation */
    v4si result;
    
    /* Manual permutation - might expand to many operations */
    result[0] = (mask[0] & 1) ? a[0] : b[0];
    result[1] = (mask[1] & 1) ? a[1] : b[1];
    result[2] = (mask[2] & 1) ? a[2] : b[2];
    result[3] = (mask[3] & 1) ? a[3] : b[3];
    
    return result;
}

/* Target-specific inline assembly with many operands */
NOINLINE static uint64_t target_specific_multi_operand(void) {
    uint64_t result = 0;
    
#if defined(__x86_64__)
    /* x86_64: Extended inline assembly with many operands */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t f = 6, g = 7, h = 8, i = 9, j = 10;
    
    asm volatile (
        /* Complex multi-operand operation simulation */
        "mov %[a], %%rax\n\t"
        "add %[b], %%rax\n\t"
        "imul %[c], %%rax\n\t"
        "add %[d], %%rax\n\t"
        "sub %[e], %%rax\n\t"
        "xor %[f], %%rax\n\t"
        "or %[g], %%rax\n\t"
        "and %[h], %%rax\n\t"
        "shl $3, %%rax\n\t"
        "add %[i], %%rax\n\t"
        "sub %[j], %%rax\n\t"
        "mov %%rax, %[result]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "rax", "cc"
    );
    
#elif defined(__aarch64__)
    /* ARM64: Multiple register operations */
    uint64_t regs[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    asm volatile (
        /* Load multiple registers and perform operations */
        "ldr x0, [%[regs], #0]\n\t"
        "ldr x1, [%[regs], #8]\n\t"
        "ldr x2, [%[regs], #16]\n\t"
        "ldr x3, [%[regs], #24]\n\t"
        "ldr x4, [%[regs], #32]\n\t"
        "add x0, x0, x1\n\t"
        "add x2, x2, x3\n\t"
        "mul x0, x0, x4\n\t"
        "ldr x5, [%[regs], #40]\n\t"
        "ldr x6, [%[regs], #48]\n\t"
        "ldr x7, [%[regs], #56]\n\t"
        "ldr x8, [%[regs], #64]\n\t"
        "add x5, x5, x6\n\t"
        "add x7, x7, x8\n\t"
        "mul x5, x5, x7\n\t"
        "add x0, x0, x5\n\t"
        "str x0, [%[result]]"
        : 
        : [regs] "r" (regs), [result] "r" (&result)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "memory", "cc"
    );
    
#elif defined(__powerpc64__)
    /* PowerPC: Multiple register operations */
    uint64_t r3 = 1, r4 = 2, r5 = 3, r6 = 4, r7 = 5;
    uint64_t r8 = 6, r9 = 7, r10 = 8, r11 = 9, r12 = 10;
    
    asm volatile (
        /* PowerPC multi-register operation */
        "add %[r3], %[r3], %[r4]\n\t"
        "mulld %[r3], %[r3], %[r5]\n\t"
        "add %[r3], %[r3], %[r6]\n\t"
        "subf %[r3], %[r7], %[r3]\n\t"
        "xor %[r3], %[r3], %[r8]\n\t"
        "or %[r3], %[r3], %[r9]\n\t"
        "and %[r3], %[r3], %[r10]\n\t"
        "sldi %[r3], %[r3], 3\n\t"
        "add %[r3], %[r3], %[r11]\n\t"
        "subf %[r3], %[r12], %[r3]\n\t"
        "mr %[result], %[r3]"
        : [result] "=r" (result)
        : [r3] "r" (r3), [r4] "r" (r4), [r5] "r" (r5), [r6] "r" (r6),
          [r7] "r" (r7), [r8] "r" (r8), [r9] "r" (r9), [r10] "r" (r10),
          [r11] "r" (r11), [r12] "r" (r12)
        : "cc"
    );
#endif
    
    return result;
}

/* Complex bit manipulation with many operands */
NOINLINE static uint64_t bit_manipulation_chain(uint64_t x) {
    /* Chain of bit operations that might expand to many RTL operands */
    uint64_t a = (x >> 0) & 0xFF;
    uint64_t b = (x >> 8) & 0xFF;
    uint64_t c = (x >> 16) & 0xFF;
    uint64_t d = (x >> 24) & 0xFF;
    uint64_t e = (x >> 32) & 0xFF;
    uint64_t f = (x >> 40) & 0xFF;
    uint64_t g = (x >> 48) & 0xFF;
    uint64_t h = (x >> 56) & 0xFF;
    
    /* Complex mixing */
    uint64_t t1 = (a * b) ^ (c * d);
    uint64_t t2 = (e * f) ^ (g * h);
    uint64_t t3 = (a ^ c ^ e ^ g) * (b ^ d ^ f ^ h);
    
    return (t1 + t2) * t3;
}

/* Main test function */
NOINLINE static uint64_t test_multi_operand_expansion(int variant) {
    uint64_t result = 0;
    
    switch (variant % 4) {
        case 0: {
            /* Test complex arithmetic */
            result = multi_operand_arithmetic(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
        }
        case 1: {
            /* Test wide multiplication */
            uint64_t hi, lo;
            wide_multiply(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL, &hi, &lo);
            result = hi ^ lo;
            break;
        }
        case 2: {
            /* Test vector operations */
            v4si vec1 = {1, 2, 3, 4};
            v4si vec2 = {5, 6, 7, 8};
            v4si mask = {0, 1, 0, 1};
            v4si vec_result = vector_permute(vec1, vec2, mask);
            result = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
            break;
        }
        case 3: {
            /* Test target-specific assembly */
            result = target_specific_multi_operand();
            break;
        }
    }
    
    /* Additional bit manipulation to prevent dead code elimination */
    result = bit_manipulation_chain(result);
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t total = 0;
    
    /* Use command-line arguments to vary the execution path */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize with varying values */
    uint64_t seed = (uint64_t)(argv[0] ? argv[0][0] : 'A');
    
    printf("Testing multi-operand RTL expansion with %d iterations...\n", iterations);
    
    /* Loop to increase coverage chance */
    for (int i = 0; i < iterations; i++) {
        /* Vary the input to trigger different expansion paths */
        uint64_t input = seed + i * 0x123456789ABCDEFULL;
        
        /* Call test function with different variants */
        uint64_t result = test_multi_operand_expansion(i % 4);
        
        /* Mix with input to prevent optimization */
        result ^= bit_manipulation_chain(input);
        
        /* Accumulate to prevent dead code elimination */
        total += result;
        
        /* Force memory operations */
        volatile uint64_t *dummy = (volatile uint64_t *)&result;
        (void)*dummy;
    }
    
    printf("Result checksum: %llu\n", (unsigned long long)total);
    
    /* Return non-zero only if total is 0 (unlikely) */
    return total == 0 ? 1 : 0;
}
