/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization from removing our test cases */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

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

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_operations(v4si a, v4si b, v4si c, v4si d,
                                v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = g * h + a;
    v4si t4 = b * c + d;
    
    return (t1 * t2) + (t3 * t4);
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* x86-specific multi-operand operations */
NOINLINE __m128i x86_multi_operand(__m128i a, __m128i b, __m128i c,
                                   __m128i d, __m128i e, __m128i f,
                                   __m128i g, __m128i h, __m128i i,
                                   __m128i j) {
    /* Complex SSE/AVX operation chain */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(c, d);
    __m128i t3 = _mm_mullo_epi32(e, f);
    __m128i t4 = _mm_and_si128(g, h);
    __m128i t5 = _mm_or_si128(i, j);
    
    __m128i r1 = _mm_add_epi32(t1, t2);
    __m128i r2 = _mm_add_epi32(t3, t4);
    
    return _mm_add_epi32(r1, _mm_add_epi32(r2, t5));
}

/* Extended inline assembly with many operands */
NOINLINE uint64_t x86_extended_asm(uint64_t a, uint64_t b, uint64_t c,
                                   uint64_t d, uint64_t e, uint64_t f,
                                   uint64_t g, uint64_t h, uint64_t i,
                                   uint64_t j) {
    uint64_t result;
    
    /* 10-operand inline assembly - might generate multi-operand RTL */
    asm volatile (
        "imulq %[b], %[a]\n\t"
        "addq %[c], %[a]\n\t"
        "imulq %[d], %[a]\n\t"
        "addq %[e], %[a]\n\t"
        "imulq %[f], %[a]\n\t"
        "addq %[g], %[a]\n\t"
        "imulq %[h], %[a]\n\t"
        "addq %[i], %[a]\n\t"
        "xorq %[j], %[a]\n\t"
        : [a] "+r" (a)
        : [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g),
          [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    result = a;
    
    /* Another complex operation to force expansion */
    asm volatile (
        "movq %1, %%rax\n\t"
        "mulq %2\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (result), "r" (a)
        : "%rax", "%rdx", "cc"
    );
    
    return result;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM-specific multi-operand operations */
NOINLINE uint64x2_t arm_multi_operand(uint64x2_t a, uint64x2_t b,
                                      uint64x2_t c, uint64x2_t d,
                                      uint64x2_t e, uint64x2_t f,
                                      uint64x2_t g, uint64x2_t h) {
    /* Complex NEON operation chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = veorq_u64(e, f);
    uint64x2_t t4 = vandq_u64(g, h);
    
    uint64x2_t r1 = vaddq_u64(t1, t2);
    uint64x2_t r2 = vaddq_u64(t3, t4);
    
    return vaddq_u64(r1, r2);
}

/* ARM extended inline assembly */
NOINLINE uint64_t arm_extended_asm(uint64_t a, uint64_t b, uint64_t c,
                                   uint64_t d, uint64_t e, uint64_t f,
                                   uint64_t g, uint64_t h) {
    uint64_t result;
    
    /* Multi-operand inline assembly for ARM */
    asm volatile (
        "mul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "mul %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mul %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "eor %0, %0, %8\n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c),
          "r" (d), "r" (e), "r" (f),
          "r" (g), "r" (h)
        : "cc"
    );
    
    return result;
}

#elif defined(__powerpc__) || defined(__PPC__)
/* PowerPC-specific operations */
NOINLINE unsigned long ppc_multi_operand(unsigned long a, unsigned long b,
                                         unsigned long c, unsigned long d,
                                         unsigned long e, unsigned long f,
                                         unsigned long g, unsigned long h) {
    unsigned long result;
    
    /* PowerPC extended inline assembly */
    asm volatile (
        "mulld %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "mulld %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "mulld %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "xor %0, %0, %8\n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c),
          "r" (d), "r" (e), "r" (f),
          "r" (g), "r" (h)
        : "cc"
    );
    
    return result;
}
#endif

/* Multi-precision arithmetic that might expand to many operands */
NOINLINE uint64_t multi_precision_arithmetic(uint64_t a, uint64_t b,
                                             uint64_t c, uint64_t d) {
    /* 128-bit multiplication using 64-bit parts */
    uint64_t a_hi = a >> 32;
    uint64_t a_lo = a & 0xFFFFFFFF;
    uint64_t b_hi = b >> 32;
    uint64_t b_lo = b & 0xFFFFFFFF;
    
    /* This expands to many operations */
    uint64_t t1 = a_lo * b_lo;
    uint64_t t2 = a_hi * b_lo;
    uint64_t t3 = a_lo * b_hi;
    uint64_t t4 = a_hi * b_hi;
    
    uint64_t lo = t1 + ((t2 & 0xFFFFFFFF) << 32);
    uint64_t hi = t4 + (t2 >> 32) + (t3 >> 32);
    
    /* Mix with other inputs */
    return (lo ^ c) + (hi ^ d);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values */
    uint64_t vals[10];
    for (int i = 0; i < 10; i++) {
        vals[i] = (uint64_t)(argc + i) * 123456789;
    }
    
    /* Test different code paths based on arguments */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        for (int i = 0; i < (argc % 10); i++) {
            result ^= multi_operand_arithmetic(
                vals[0] + i, vals[1] + i, vals[2] + i,
                vals[3] + i, vals[4] + i, vals[5] + i,
                vals[6] + i, vals[7] + i, vals[8] + i,
                vals[9] + i
            );
        }
        
        /* Path 2: Multi-precision arithmetic */
        for (int i = 0; i < (argc % 5); i++) {
            result += multi_precision_arithmetic(
                vals[0] + i, vals[1] + i,
                vals[2] + i, vals[3] + i
            );
        }
    } else {
        /* Path 3: Architecture-specific operations */
#ifdef __x86_64__
        /* Initialize vectors for x86 */
        __m128i vecs[10];
        for (int i = 0; i < 10; i++) {
            vecs[i] = _mm_set_epi32(i+3, i+2, i+1, i);
        }
        
        result = x86_extended_asm(
            vals[0], vals[1], vals[2], vals[3],
            vals[4], vals[5], vals[6], vals[7],
            vals[8], vals[9]
        );
        
        __m128i vec_result = x86_multi_operand(
            vecs[0], vecs[1], vecs[2], vecs[3],
            vecs[4], vecs[5], vecs[6], vecs[7],
            vecs[8], vecs[9]
        );
        
        /* Extract value from vector */
        int temp[4];
        _mm_storeu_si128((__m128i*)temp, vec_result);
        result += temp[0] + temp[1] + temp[2] + temp[3];
        
#elif defined(__aarch64__)
        result = arm_extended_asm(
            vals[0], vals[1], vals[2], vals[3],
            vals[4], vals[5], vals[6], vals[7]
        );
        
#elif defined(__powerpc__) || defined(__PPC__)
        result = ppc_multi_operand(
            vals[0], vals[1], vals[2], vals[3],
            vals[4], vals[5], vals[6], vals[7]
        );
#endif
        
        /* Vector operations (generic) */
        v4si v1 = {vals[0], vals[1], vals[2], vals[3]};
        v4si v2 = {vals[4], vals[5], vals[6], vals[7]};
        v4si v3 = {vals[8], vals[9], vals[0], vals[1]};
        v4si v4 = {vals[2], vals[3], vals[4], vals[5]};
        v4si v5 = {vals[6], vals[7], vals[8], vals[9]};
        v4si v6 = {vals[0], vals[2], vals[4], vals[6]};
        v4si v7 = {vals[1], vals[3], vals[5], vals[7]};
        v4si v8 = {vals[8], vals[9], vals[0], vals[1]};
        
        v4si vec_result = vector_operations(v1, v2, v3, v4, v5, v6, v7, v8);
        result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)result);
    
    /* Additional loop to increase coverage opportunities */
    for (int i = 0; i < argc; i++) {
        uint64_t temp = 0;
        for (int j = 0; j < 10; j++) {
            temp ^= vals[j] * (i + j);
        }
        result ^= temp;
    }
    
    printf("Final result: %lu\n", (unsigned long)result);
    
    return (int)(result & 0x7FFFFFFF);
}
