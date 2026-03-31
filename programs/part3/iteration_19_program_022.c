/* test_optabs.c - Test program for 10/11 operand RTL expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization from eliminating our test functions */
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
    
    /* Mix them together in a way that prevents optimization */
    uint64_t result = t1 + t2 - t3 * t4 / (t5 + 1);
    result = result ^ (t1 << 3) ^ (t2 << 5) ^ (t3 << 7);
    
    return result;
}

/* Function using inline assembly with many operands */
NOINLINE static uint64_t inline_asm_10_operands(uint64_t a, uint64_t b,
                                                uint64_t c, uint64_t d,
                                                uint64_t e, uint64_t f,
                                                uint64_t g, uint64_t h,
                                                uint64_t i, uint64_t j) {
    uint64_t result;
    
#if defined(__x86_64__)
    /* x86_64 inline assembly with 10 operands */
    asm volatile (
        "mov %1, %0\n\t"
        "imul %2, %0\n\t"
        "add %3, %0\n\t"
        "sub %4, %0\n\t"
        "and %5, %0\n\t"
        "or %6, %0\n\t"
        "xor %7, %0\n\t"
        "shl $3, %0\n\t"
        "add %8, %0\n\t"
        "sub %9, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
#elif defined(__aarch64__)
    /* ARM64 inline assembly with 10 operands */
    asm volatile (
        "mul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "and %0, %0, %5\n\t"
        "orr %0, %0, %6\n\t"
        "eor %0, %0, %7\n\t"
        "lsl %0, %0, #3\n\t"
        "add %0, %0, %8\n\t"
        "sub %0, %0, %9"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
#else
    /* Generic fallback */
    result = a * b + c - d & e | f ^ g;
    result = (result << 3) + h - i;
#endif
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_operations(v4si a, v4si b, v4si c, v4si d,
                                       v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = g * h;
    v4si t4 = a + b + c + d;
    
    /* Mix operations to prevent optimization */
    v4si result = (t1 & t2) | (t3 ^ t4);
    result = result + (a >> 2) - (b << 1);
    
    return result;
}

/* Test function for 64-bit multi-precision arithmetic */
NOINLINE static uint64_t test_mult_highpart(uint64_t a, uint64_t b, uint64_t c) {
    /* This might trigger expand_mult_highpart with many operands */
    uint64_t hi1 = ((__uint128_t)a * b) >> 64;
    uint64_t hi2 = ((__uint128_t)b * c) >> 64;
    uint64_t hi3 = ((__uint128_t)c * a) >> 64;
    
    /* Mix them in a complex way */
    uint64_t result = hi1 + (hi2 << 32) - (hi3 >> 32);
    result = result * a + b * c;
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values based on argc to get varying inputs */
    uint64_t vals[10];
    for (int i = 0; i < 10; i++) {
        vals[i] = (uint64_t)(argc + i) * 0x123456789ABCDEFULL;
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        for (int i = 0; i < 100; i++) {
            result ^= multi_operand_arithmetic(
                vals[0] + i, vals[1] + i, vals[2] + i, vals[3] + i,
                vals[4] + i, vals[5] + i, vals[6] + i, vals[7] + i,
                vals[8] + i, vals[9] + i
            );
        }
    } else {
        /* Path 2: Inline assembly */
        for (int i = 0; i < 100; i++) {
            result ^= inline_asm_10_operands(
                vals[0] + i, vals[1] + i, vals[2] + i, vals[3] + i,
                vals[4] + i, vals[5] + i, vals[6] + i, vals[7] + i,
                vals[8] + i, vals[9] + i
            );
        }
    }
    
    /* Also test vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    v4si vec_e = {17, 18, 19, 20};
    v4si vec_f = {21, 22, 23, 24};
    v4si vec_g = {25, 26, 27, 28};
    v4si vec_h = {29, 30, 31, 32};
    
    v4si vec_result = vector_operations(vec_a, vec_b, vec_c, vec_d,
                                        vec_e, vec_f, vec_g, vec_h);
    
    /* Test multi-precision arithmetic */
    uint64_t mp_result = test_mult_highpart(vals[0], vals[1], vals[2]);
    
    /* Combine all results to prevent dead code elimination */
    uint64_t final_result = result;
    for (int i = 0; i < 4; i++) {
        final_result += vec_result[i];
    }
    final_result += mp_result;
    
    printf("Result: %lu\n", final_result);
    
    return 0;
}
