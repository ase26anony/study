/* test_optabs_multiop.c - Test program for GCC optabs.cc 10/11 operand expansion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing our test code */
#define NOOPT __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOOPT uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    /* Complex expression that might require many temporary registers */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Multi-precision calculation */
    uint64_t result = t1 + t2;
    result = result * t3;
    result = result + t4;
    result = result - t5;
    
    /* Force use of all inputs */
    result ^= (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j);
    
    return result;
}

/* Vector operations that might expand to many operands */
NOOPT v4si vector_multi_op(v4si a, v4si b, v4si c, v4si d,
                           v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = g << 2;
    v4si t4 = t1 | t2;
    v4si t5 = t3 & t4;
    v4si t6 = t5 ^ a ^ b ^ c ^ d ^ e ^ f ^ g;
    
    return t6;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* x86-specific multi-operand inline assembly */
NOOPT uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                     uint64_t d, uint64_t e, uint64_t f,
                                     uint64_t g, uint64_t h, uint64_t i,
                                     uint64_t j) {
    uint64_t result;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        "mov %1, %%rax\n\t"
        "imul %2, %%rax\n\t"
        "mov %%rax, %0\n\t"
        "add %3, %0\n\t"
        "sub %4, %0\n\t"
        "xor %5, %0\n\t"
        "or %6, %0\n\t"
        "and %7, %0\n\t"
        "add %8, %0\n\t"
        "sub %9, %0\n\t"
        "xor %10, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "rax", "cc"
    );
    
    return result;
}

/* AVX-512 operations that might require many operands */
NOOPT __m512i avx512_multi_op(__m512i a, __m512i b, __m512i c,
                              __m512i d, __m512i e, __m512i f) {
    __m512i t1 = _mm512_add_epi64(a, b);
    __m512i t2 = _mm512_sub_epi64(c, d);
    __m512i t3 = _mm512_mullo_epi64(e, f);
    __m512i t4 = _mm512_and_si512(t1, t2);
    __m512i t5 = _mm512_or_si512(t3, t4);
    __m512i t6 = _mm512_xor_si512(t5, a);
    
    return _mm512_add_epi64(t6, _mm512_add_epi64(b, _mm512_add_epi64(c, 
           _mm512_add_epi64(d, _mm512_add_epi64(e, f)))));
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM-specific multi-operand operations */
NOOPT uint64x2_t arm_multi_operand(uint64x2_t a, uint64x2_t b, uint64x2_t c,
                                   uint64x2_t d, uint64x2_t e, uint64x2_t f) {
    /* Complex NEON operations chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vorrq_u64(t1, t2);
    uint64x2_t t5 = vandq_u64(t3, t4);
    uint64x2_t t6 = veorq_u64(t5, a);
    
    /* Additional operations to increase operand count */
    t6 = vaddq_u64(t6, b);
    t6 = vaddq_u64(t6, c);
    t6 = vaddq_u64(t6, d);
    t6 = vaddq_u64(t6, e);
    t6 = vaddq_u64(t6, f);
    
    return t6;
}

#elif defined(__powerpc__) || defined(__PPC__)
/* PowerPC-specific operations */
NOOPT unsigned long long ppc_multi_operand(unsigned long long a,
                                           unsigned long long b,
                                           unsigned long long c,
                                           unsigned long long d,
                                           unsigned long long e,
                                           unsigned long long f,
                                           unsigned long long g,
                                           unsigned long long h,
                                           unsigned long long i,
                                           unsigned long long j) {
    unsigned long long result;
    
    /* PowerPC extended inline assembly */
    asm volatile (
        "mulld %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "subf %0, %4, %0\n\t"
        "xor %0, %0, %5\n\t"
        "or %0, %0, %6\n\t"
        "and %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "subf %0, %9, %0\n\t"
        "xor %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Test function that combines multiple expansion strategies */
NOOPT uint64_t test_multi_operand_expansion(int variant, uint64_t seed) {
    uint64_t result = seed;
    
    /* Initialize many variables to use as operands */
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
    
    switch (variant % 4) {
        case 0:
            /* Complex arithmetic expression */
            result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
            break;
            
        case 1:
#ifdef __x86_64__
            /* x86-specific inline assembly */
            result = x86_multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
#elif defined(__aarch64__) || defined(__powerpc__) || defined(__PPC__)
            /* Fallback for other architectures */
            result = a * b + c * d - e * f + g * h - i * j;
#endif
            break;
            
        case 2:
            /* Vector operations */
            v4si va = {a, b, c, d};
            v4si vb = {e, f, g, h};
            v4si vc = {i, j, a, b};
            v4si vd = {c, d, e, f};
            v4si ve = {g, h, i, j};
            v4si vf = {a, c, e, g};
            v4si vg = {b, d, f, h};
            
            v4si vresult = vector_multi_op(va, vb, vc, vd, ve, vf, vg);
            result = vresult[0] + vresult[1] + vresult[2] + vresult[3];
            break;
            
        case 3:
            /* Mixed operations to stress the expander */
            result = ((a * b) >> 32) + ((c * d) >> 32) - ((e * f) >> 32);
            result = result * ((g * h) >> 32) / ((i * j) >> 32 + 1);
            result = result ^ a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j;
            break;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t total = 0;
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Testing multi-operand expansion with %d iterations...\n", iterations);
    
    /* Test different code paths to trigger various expansion patterns */
    for (int i = 0; i < iterations; i++) {
        uint64_t seed = i + argc;
        
        /* Call with different variants to exercise different paths */
        total += test_multi_operand_expansion(0, seed);
        total += test_multi_operand_expansion(1, seed + 1);
        total += test_multi_operand_expansion(2, seed + 2);
        total += test_multi_operand_expansion(3, seed + 3);
        
        /* Add some branching to affect optimization */
        if (i % 2 == 0) {
            total += test_multi_operand_expansion(0, total);
        } else {
            total += test_multi_operand_expansion(1, total);
        }
    }
    
    printf("Result checksum: %lu\n", (unsigned long)total);
    printf("Test completed.\n");
    
    return 0;
}
