/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization from eliminating our test cases */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i,
                                           uint64_t j) {
    /* Complex expression that might require many temporary operands */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    return ((t1 + t2) * (t3 + t4)) ^ t5;
}

/* Multi-precision multiplication that might expand to many operands */
NOINLINE void wide_multiply(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo) {
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
    
    *lo = (p0 & 0xFFFFFFFF) | ((carry & 0xFFFFFFFF) << 32);
    *hi = (p1 >> 32) + (p2 >> 32) + p3 + (carry >> 32);
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_ops(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector expression */
    v4si t1 = a * b;
    v4si t2 = c * d;
    v4si t3 = a + c;
    v4si t4 = b + d;
    
    /* Mix operations to prevent optimization */
    return (t1 & t2) | (t3 ^ t4);
}

/* Target-specific inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result;
    
#ifdef __x86_64__
    /* Extended inline assembly with many input/output operands */
    asm volatile (
        "movq %1, %%rax\n\t"
        "mulq %2\n\t"
        "addq %3, %%rax\n\t"
        "adcq %4, %%rdx\n\t"
        "addq %5, %%rax\n\t"
        "adcq %6, %%rdx\n\t"
        "addq %7, %%rax\n\t"
        "adcq %8, %%rdx\n\t"
        "addq %9, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "rax", "rdx", "cc"
    );
#else
    /* Fallback for non-x86 */
    result = a * b + c + d + e + f + g + h + i;
#endif
    
    return result;
}

#ifdef __aarch64__
#include <arm_neon.h>
NOINLINE int32x4_t arm_neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                          int32x4_t d, int32x4_t e, int32x4_t f) {
    /* Complex NEON operations that might expand to many operands */
    int32x4_t t1 = vqdmulhq_s32(a, b);
    int32x4_t t2 = vqdmulhq_s32(c, d);
    int32x4_t t3 = vqaddq_s32(t1, t2);
    int32x4_t t4 = vqdmulhq_s32(e, f);
    
    return vqaddq_s32(t3, t4);
}
#endif

/* Test function that tries different expansion paths based on optimization level hints */
NOINLINE uint64_t test_expansion_path(uint64_t seed, int use_complex) {
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
    
    if (use_complex) {
        /* Path 1: Complex arithmetic that might expand to many operands */
        uint64_t hi, lo;
        wide_multiply(a, b, &hi, &lo);
        return multi_operand_arithmetic(hi, lo, c, d, e, f, g, h, i, j);
    } else {
        /* Path 2: Inline assembly path */
        return x86_multi_operand_asm(a, b, c, d, e, f, g, h, i);
    }
}

int main(int argc, char *argv[]) {
    uint64_t result = 0;
    int use_complex = 0;
    
    /* Use command line argument to choose code path */
    if (argc > 1) {
        use_complex = atoi(argv[1]);
    }
    
    /* Initialize with some values */
    uint64_t values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = (uint64_t)i * 123456789;
    }
    
    /* Try to trigger different expansion patterns */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the inputs slightly each iteration */
        uint64_t seed = (uint64_t)iter * 987654321;
        
        /* Call test function with different optimization contexts */
        result ^= test_expansion_path(seed, use_complex);
        
        /* Also test vector operations */
        v4si vec_a = {seed, seed + 1, seed + 2, seed + 3};
        v4si vec_b = {seed + 4, seed + 5, seed + 6, seed + 7};
        v4si vec_c = {seed + 8, seed + 9, seed + 10, seed + 11};
        v4si vec_d = {seed + 12, seed + 13, seed + 14, seed + 15};
        
        v4si vec_result = vector_ops(vec_a, vec_b, vec_c, vec_d);
        
        /* Use vector result to prevent dead code elimination */
        for (int j = 0; j < 4; j++) {
            result += vec_result[j];
        }
        
#ifdef __aarch64__
        /* Test ARM NEON if available */
        int32x4_t neon_a = {seed, seed + 1, seed + 2, seed + 3};
        int32x4_t neon_b = {seed + 4, seed + 5, seed + 6, seed + 7};
        int32x4_t neon_c = {seed + 8, seed + 9, seed + 10, seed + 11};
        int32x4_t neon_d = {seed + 12, seed + 13, seed + 14, seed + 15};
        int32x4_t neon_e = {seed + 16, seed + 17, seed + 18, seed + 19};
        int32x4_t neon_f = {seed + 20, seed + 21, seed + 22, seed + 23};
        
        int32x4_t neon_result = arm_neon_multi_operand(neon_a, neon_b, neon_c,
                                                      neon_d, neon_e, neon_f);
        
        /* Use NEON result */
        int32_t neon_array[4];
        vst1q_s32(neon_array, neon_result);
        for (int j = 0; j < 4; j++) {
            result += neon_array[j];
        }
#endif
    }
    
    /* Print result to prevent optimization */
    printf("Result: %lu\n", (unsigned long)result);
    
    return 0;
}
