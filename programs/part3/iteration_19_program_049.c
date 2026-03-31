/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimization from eliminating our test cases */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t test_multi_precision_mul(uint64_t a, uint64_t b, uint64_t c, 
                                          uint64_t d, uint64_t e, uint64_t f,
                                          uint64_t g, uint64_t h, uint64_t i,
                                          uint64_t j) {
    /* Complex expression that might require multi-operand expansion */
    uint64_t result = 0;
    
    /* Multiple operations that could be combined into a single RTL pattern */
    result = a * b + c * d + e * f + g * h + i * j;
    result = result * a + result * b + result * c;
    result = (result >> 32) | (result << 32);  /* Rotate */
    
    return result;
}

/* Function using inline assembly with many operands */
NOINLINE uint64_t test_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        /* Complex multi-operand operation - different for each architecture */
        #ifdef __x86_64__
        "movq %[a], %%rax\n\t"
        "mulq %[b]\n\t"
        "movq %%rax, %[r1]\n\t"
        "movq %%rdx, %[r2]\n\t"
        "movq %[c], %%rax\n\t"
        "mulq %[d]\n\t"
        "addq %%rax, %[r1]\n\t"
        "adcq %%rdx, %[r2]\n\t"
        "movq %[e], %%rax\n\t"
        "mulq %[f]\n\t"
        "addq %%rax, %[r1]\n\t"
        "adcq %%rdx, %[r2]\n\t"
        "movq %[g], %%rax\n\t"
        "mulq %[h]\n\t"
        "addq %%rax, %[r1]\n\t"
        "adcq %%rdx, %[r2]\n\t"
        "movq %[i], %%rax\n\t"
        "mulq %[j]\n\t"
        "addq %%rax, %[r1]\n\t"
        "adcq %%rdx, %[r2]\n\t"
        "movq %[r1], %[r3]\n\t"
        "movq %[r2], %[r1]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rdx", "cc"
        );
        
    #elif defined(__aarch64__)
    "mul %[r1], %[a], %[b]\n\t"
    "mul %[r2], %[c], %[d]\n\t"
    "add %[r1], %[r1], %[r2]\n\t"
    "mul %[r2], %[e], %[f]\n\t"
    "add %[r1], %[r1], %[r2]\n\t"
    "mul %[r2], %[g], %[h]\n\t"
    "add %[r1], %[r1], %[r2]\n\t"
    "mul %[r2], %[i], %[j]\n\t"
    "add %[r1], %[r1], %[r2]\n\t"
    "mov %[r3], %[r1]\n\t"
    "lsr %[r2], %[r1], #32\n\t"
    "lsl %[r1], %[r1], #32\n\t"
    "orr %[r1], %[r1], %[r2]\n\t"
    : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
    : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
      [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
      [i] "r" (i), [j] "r" (j)
    : "cc"
    );
    
    #else
    /* Generic fallback */
    result1 = a * b + c * d + e * f + g * h + i * j;
    result2 = result1 >> 32;
    result3 = result1 << 32;
    #endif
    
    return result1 + result2 + result3;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si test_vector_ops(v4si a, v4si b, v4si c, v4si d, v4si e) {
    v4si result;
    
    /* Complex vector expression */
    result = a * b + c * d;
    result = result + e;
    result = result * a - result * b;
    result = (result << 2) | (result >> 30);  /* Vector rotate-ish */
    
    /* Permutation-like operation */
    result = __builtin_shuffle(result, a, (v4si){1, 0, 3, 2});
    result = __builtin_shuffle(result, b, (v4si){2, 3, 0, 1});
    
    return result;
}

/* Test fused multiply-add chains */
NOINLINE double test_fma_chain(double a, double b, double c, double d,
                              double e, double f, double g, double h,
                              double i, double j) {
    double result;
    
    /* Chain of FMA-like operations */
    result = a * b + c;
    result = result * d + e;
    result = result * f + g;
    result = result * h + i;
    result = result * j + a;  /* Use a again to create dependency */
    
    #ifdef __FP_FAST_FMA
    /* Use builtin FMA if available */
    result = __builtin_fma(result, b, c);
    result = __builtin_fma(result, d, e);
    result = __builtin_fma(result, f, g);
    #endif
    
    return result;
}

/* Bit manipulation with many operands */
NOINLINE uint64_t test_bit_ops(uint64_t a, uint64_t b, uint64_t c,
                              uint64_t d, uint64_t e, uint64_t f,
                              uint64_t g, uint64_t h, uint64_t i,
                              uint64_t j, uint64_t k) {
    /* 11 operands total including the return */
    uint64_t result;
    
    result = (a & b) | (c & d);
    result = result ^ (e & f);
    result = (result << g) | (result >> (64 - g));
    result = result + (h & i) - (j & k);
    result = (result * a) >> (b & 63);
    
    /* More complex bit manipulation */
    result = ((result << 1) & 0xAAAAAAAAAAAAAAAA) | ((result >> 1) & 0x5555555555555555);
    result = result ^ a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k;
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    uint64_t values[11];
    
    /* Initialize with pseudo-random values based on argc */
    for (int i = 0; i < 11; i++) {
        values[i] = (uint64_t)(argc + i) * 0x9E3779B97F4A7C15ULL;
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Multi-precision multiplication */
        result += test_multi_precision_mul(values[0], values[1], values[2],
                                          values[3], values[4], values[5],
                                          values[6], values[7], values[8],
                                          values[9]);
        
        /* Path 2: Inline assembly with many operands */
        result += test_multi_operand_asm(values[1], values[2], values[3],
                                        values[4], values[5], values[6],
                                        values[7], values[8], values[9],
                                        values[0]);
    } else {
        /* Path 3: Bit operations with 11 operands */
        result += test_bit_ops(values[0], values[1], values[2], values[3],
                              values[4], values[5], values[6], values[7],
                              values[8], values[9], values[10]);
        
        /* Path 4: FMA chain */
        result += (uint64_t)test_fma_chain(
            (double)values[0], (double)values[1], (double)values[2],
            (double)values[3], (double)values[4], (double)values[5],
            (double)values[6], (double)values[7], (double)values[8],
            (double)values[9]);
    }
    
    /* Vector operations - always executed */
    v4si vec_a = {values[0], values[1], values[2], values[3]};
    v4si vec_b = {values[4], values[5], values[6], values[7]};
    v4si vec_c = {values[8], values[9], values[0], values[1]};
    v4si vec_d = {values[2], values[3], values[4], values[5]};
    v4si vec_e = {values[6], values[7], values[8], values[9]};
    
    v4si vec_result = test_vector_ops(vec_a, vec_b, vec_c, vec_d, vec_e);
    
    /* Use vector result to affect final output */
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    /* Loop with varying inputs to increase coverage */
    for (int iter = 0; iter < (argc > 2 ? atoi(argv[2]) : 1); iter++) {
        for (int i = 0; i < 11; i++) {
            values[i] += iter * 0x123456789ABCDEFULL;
        }
        
        /* Mix different test functions */
        result ^= test_multi_precision_mul(values[0], values[1], values[2],
                                          values[3], values[4], values[5],
                                          values[6], values[7], values[8],
                                          values[9]);
        
        if (iter % 2 == 0) {
            result ^= test_bit_ops(values[10], values[0], values[1], values[2],
                                  values[3], values[4], values[5], values[6],
                                  values[7], values[8], values[9]);
        }
    }
    
    printf("Result: %llu\n", (unsigned long long)result);
    return (int)(result & 0x7FFFFFFF);
}
