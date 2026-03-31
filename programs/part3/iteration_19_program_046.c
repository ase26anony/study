/* test_optabs_10_11_operands.c
 * 
 * This test aims to trigger GCC's RTL expansion for instructions with
 * 10 or 11 operands, covering uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the generated .expand file for multi-operand RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might eliminate our test patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== TARGET-SPECIFIC MULTI-OPERAND PATTERNS ========== */

#ifdef __x86_64__
/* x86_64 specific patterns */

/* Complex inline assembly with many operands - may expand to 10+ operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                               uint64_t d, uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        /* Complex operation using many input registers */
        "mov %[a], %%rax\n\t"
        "add %[b], %%rax\n\t"
        "imul %[c], %%rax\n\t"
        "add %[d], %%rax\n\t"
        "sub %[e], %%rax\n\t"
        "xor %[f], %%rax\n\t"
        "or %[g], %%rax\n\t"
        "and %[h], %%rax\n\t"
        "add %[i], %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        
        /* Another chain to use more operands */
        "mov %[c], %%rbx\n\t"
        "lea (%%rbx, %[d], 2), %%rbx\n\t"
        "mov %[out1], %%rcx\n\t"
        "add %%rcx, %%rbx\n\t"
        "mov %%rbx, %[out2]\n\t"
        
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "rax", "rbx", "rcx", "cc", "memory"
    );
    
    /* Force use of all results */
    return result1 + result2;
}

/* Multi-precision arithmetic that might expand to many operands */
NOINLINE static uint64_t x86_mult_highpart(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e) {
    /* Complex expression that might use expand_mult_highpart */
    uint64_t t1 = (a * b) >> 32;      /* Might use highpart multiplication */
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (t1 * t2) >> 32;
    uint64_t t4 = (t3 * e) >> 32;
    
    /* Chain operations to increase operand count */
    uint64_t result = t1 + t2 + t3 + t4;
    result = (result * a) >> 32;
    result = (result * b) >> 32;
    result = (result * c) >> 32;
    result = (result * d) >> 32;
    
    return result;
}

#elif defined(__aarch64__)
/* ARM64 specific patterns */

NOINLINE static uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                               uint64_t d, uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2;
    
    /* ARM inline assembly with many operands */
    asm volatile (
        /* Complex operation using multiple registers */
        "add %[out1], %[a], %[b]\n\t"
        "mul %[out1], %[out1], %[c]\n\t"
        "add %[out1], %[out1], %[d]\n\t"
        "sub %[out1], %[out1], %[e]\n\t"
        "eor %[out1], %[out1], %[f]\n\t"
        "orr %[out1], %[out1], %[g]\n\t"
        "and %[out1], %[out1], %[h]\n\t"
        "add %[out1], %[out1], %[i]\n\t"
        
        /* Second result using different combination */
        "madd %[out2], %[a], %[b], %[c]\n\t"
        "madd %[out2], %[out2], %[d], %[e]\n\t"
        
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    return result1 + result2;
}

#else
/* Generic fallback - try to create complex expressions */

NOINLINE static uint64_t generic_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                               uint64_t d, uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h, uint64_t i,
                                               uint64_t j, uint64_t k) {
    /* Very complex expression that might expand to many operands */
    uint64_t t1 = (a * b + c) >> 1;
    uint64_t t2 = (d * e + f) >> 1;
    uint64_t t3 = (g * h + i) >> 1;
    uint64_t t4 = (j * k + a) >> 1;
    
    /* Chain of operations */
    uint64_t result = t1;
    result = (result * t2 + t3) >> 1;
    result = (result * t4 + b) >> 1;
    result = (result * c + d) >> 1;
    result = (result * e + f) >> 1;
    result = (result * g + h) >> 1;
    result = (result * i + j) >> 1;
    result = (result * k + t1) >> 1;
    
    return result;
}

#endif

/* ========== VECTOR OPERATIONS ========== */

/* Vector operations that might expand to multi-operand RTL */
NOINLINE static v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                                          v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a + b;
    v4si t2 = c * d;
    v4si t3 = e & f;
    v4si t4 = t1 | t2;
    v4si t5 = t3 ^ t4;
    v4si t6 = g << 1;
    v4si t7 = t5 >> 2;
    
    /* Chain many operations */
    v4si result = t1 + t2 + t3 + t4 + t5 + t6 + t7;
    result = result * a - b;
    result = result * c + d;
    result = result * e | f;
    result = result & g;
    
    return result;
}

/* ========== TEST FUNCTIONS ========== */

/* Function that uses many parameters to force multi-operand expansion */
NOINLINE static uint64_t test_10_operands(uint64_t a, uint64_t b, uint64_t c,
                                          uint64_t d, uint64_t e, uint64_t f,
                                          uint64_t g, uint64_t h, uint64_t i,
                                          uint64_t j) {
    /* Complex computation using all 10 parameters */
    uint64_t t1 = (a * b) >> (c & 0x3F);
    uint64_t t2 = (d * e) >> (f & 0x3F);
    uint64_t t3 = (g * h) >> (i & 0x3F);
    uint64_t t4 = (j * a) >> (b & 0x3F);
    
    uint64_t result = t1 + t2 + t3 + t4;
    
    /* More operations to increase complexity */
    result = (result * c) / (d ? d : 1);
    result = (result + e) * f;
    result = (result ^ g) | h;
    result = (result & i) + j;
    
    return result;
}

NOINLINE static uint64_t test_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                          uint64_t d, uint64_t e, uint64_t f,
                                          uint64_t g, uint64_t h, uint64_t i,
                                          uint64_t j, uint64_t k) {
    /* Use all 11 parameters in a complex expression */
    uint64_t t1 = a + b + c + d + e;
    uint64_t t2 = f * g * h * i * j;
    uint64_t t3 = k ^ a ^ b ^ c ^ d;
    
    uint64_t result = (t1 * t2) >> (t3 & 0x3F);
    
    /* Chain operations using all parameters */
    result += a * b;
    result -= c * d;
    result |= e * f;
    result &= g * h;
    result ^= i * j;
    result += k;
    
    /* Division/modulus that might expand to many operations */
    if (result != 0) {
        result = (result * 0xAAAAAAAB) >> 33;  /* Division by 3 approximation */
    }
    
    return result;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values */
    uint64_t vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = (uint64_t)(argc + i * 123456789);
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Test with many function arguments */
        result += test_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                   vals[5], vals[6], vals[7], vals[8], vals[9]);
        
        result += test_11_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                   vals[5], vals[6], vals[7], vals[8], vals[9],
                                   vals[10]);
    } else {
        /* Path 2: Target-specific patterns */
        #ifdef __x86_64__
        result += x86_multi_operand_asm(vals[0], vals[1], vals[2], vals[3], vals[4],
                                        vals[5], vals[6], vals[7], vals[8]);
        result += x86_mult_highpart(vals[0], vals[1], vals[2], vals[3], vals[4]);
        #elif defined(__aarch64__)
        result += arm_multi_operand_asm(vals[0], vals[1], vals[2], vals[3], vals[4],
                                        vals[5], vals[6], vals[7], vals[8]);
        #else
        result += generic_multi_operand(vals[0], vals[1], vals[2], vals[3], vals[4],
                                        vals[5], vals[6], vals[7], vals[8],
                                        vals[9], vals[10]);
        #endif
    }
    
    /* Test vector operations */
    v4si vec_a = {vals[0], vals[1], vals[2], vals[3]};
    v4si vec_b = {vals[4], vals[5], vals[6], vals[7]};
    v4si vec_c = {vals[8], vals[9], vals[10], vals[11]};
    v4si vec_d = {vals[12], vals[13], vals[14], vals[15]};
    v4si vec_e = {vals[16], vals[17], vals[18], vals[19]};
    v4si vec_f = {vals[0] + 1, vals[1] + 1, vals[2] + 1, vals[3] + 1};
    v4si vec_g = {vals[4] + 1, vals[5] + 1, vals[6] + 1, vals[7] + 1};
    
    v4si vec_result = vector_multi_operand(vec_a, vec_b, vec_c, vec_d,
                                           vec_e, vec_f, vec_g);
    
    /* Use vector result to prevent dead code elimination */
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    /* Loop with varying inputs to test different expansion scenarios */
    for (int iter = 0; iter < (argc > 2 ? atoi(argv[2]) : 1); iter++) {
        /* Modify values slightly each iteration */
        for (int i = 0; i < 20; i++) {
            vals[i] = vals[i] * 1103515245 + 12345;
        }
        
        /* Call test functions again with new values */
        if (argc > 1) {
            result ^= test_10_operands(vals[0], vals[1], vals[2], vals[3], vals[4],
                                       vals[5], vals[6], vals[7], vals[8], vals[9]);
        }
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
