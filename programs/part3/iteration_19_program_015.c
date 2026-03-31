/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations from eliminating our test code */
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
    /* Complex expression that might require many temporaries */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    return ((t1 + t2) * (t3 + t4)) ^ t5;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_multi_ops(v4si a, v4si b, v4si c, v4si d,
                               v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = (t1 & t2) | g;
    v4si t4 = (t1 ^ t2) & g;
    return t3 * t4 + a - b;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE __m128i x86_multi_operand_intrinsic(__m128i a, __m128i b, __m128i c,
                                            __m128i d, __m128i e, __m128i f,
                                            __m128i g, __m128i h) {
    /* Complex chain of intrinsics that might expand to many operands */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(c, d);
    __m128i t3 = _mm_mullo_epi32(e, f);
    __m128i t4 = _mm_and_si128(g, h);
    __m128i t5 = _mm_or_si128(t1, t2);
    __m128i t6 = _mm_xor_si128(t3, t4);
    return _mm_add_epi32(t5, t6);
}

/* Extended inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i,
                                       uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Multi-operand inline assembly - may expand to RTL with many operands */
    asm volatile (
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %[c], %%rbx\n\t"
        "imul %[d], %%rbx\n\t"
        "add %%rbx, %[r1]\n\t"
        "mov %[e], %%rcx\n\t"
        "imul %[f], %%rcx\n\t"
        "mov %%rcx, %[r2]\n\t"
        "mov %[g], %%rdx\n\t"
        "imul %[h], %%rdx\n\t"
        "add %%rdx, %[r2]\n\t"
        "mov %[i], %%rsi\n\t"
        "imul %[j], %%rsi\n\t"
        "mov %%rsi, %[r3]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rbx", "rcx", "rdx", "rsi", "cc"
    );
    
    return result1 + result2 + result3;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOINLINE uint64x2_t arm_multi_operand_intrinsic(uint64x2_t a, uint64x2_t b,
                                               uint64x2_t c, uint64x2_t d,
                                               uint64x2_t e, uint64x2_t f) {
    /* ARM NEON intrinsics chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vandq_u64(t1, t2);
    return vorrq_u64(t3, t4);
}

/* ARM inline assembly with many operands */
NOINLINE uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h) {
    uint64_t result;
    
    asm volatile (
        "add %[a], %[b], %[tmp1]\n\t"
        "sub %[c], %[d], %[tmp2]\n\t"
        "mul %[e], %[f], %[tmp3]\n\t"
        "and %[tmp1], %[tmp2], %[tmp4]\n\t"
        "orr %[tmp3], %[tmp4], %[result]\n\t"
        "add %[result], %[g], %[result]\n\t"
        "sub %[result], %[h], %[result]\n\t"
        : [result] "=&r" (result), [tmp1] "=&r" (a), [tmp2] "=&r" (b),
          [tmp3] "=&r" (c), [tmp4] "=&r" (d)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc"
    );
    
    return result;
}

#elif defined(__powerpc__) || defined(__PPC__)
/* PowerPC specific multi-operand operations */
NOINLINE uint64_t ppc_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                   uint64_t d, uint64_t e, uint64_t f,
                                   uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result;
    
    /* PowerPC inline assembly with many operands */
    asm volatile (
        "mulld %[result], %[a], %[b]\n\t"
        "mulhdu %[tmp1], %[c], %[d]\n\t"
        "add %[result], %[result], %[tmp1]\n\t"
        "mulld %[tmp2], %[e], %[f]\n\t"
        "add %[result], %[result], %[tmp2]\n\t"
        "mulhdu %[tmp3], %[g], %[h]\n\t"
        "add %[result], %[result], %[tmp3]\n\t"
        "xor %[result], %[result], %[i]\n\t"
        : [result] "=&r" (result), [tmp1] "=&r" (a), [tmp2] "=&r" (b),
          [tmp3] "=&r" (c)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    return result;
}
#endif

/* Multi-precision arithmetic that might expand to many operands */
NOINLINE uint64_t multi_precision_mul(uint32_t a, uint32_t b, uint32_t c,
                                      uint32_t d, uint32_t e, uint32_t f,
                                      uint32_t g, uint32_t h) {
    /* 64-bit multiplication using 32-bit parts */
    uint64_t a64 = (uint64_t)a * b;
    uint64_t c64 = (uint64_t)c * d;
    uint64_t e64 = (uint64_t)e * f;
    uint64_t g64 = (uint64_t)g * h;
    
    /* Complex combination */
    return (a64 >> 32) + (c64 >> 16) + (e64 >> 8) + (g64 >> 4);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values based on argc to get some variation */
    uint64_t vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = (uint64_t)(argc + i) * 123456789;
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic with many operands */
        for (int i = 0; i < (argc % 5); i++) {
            result ^= multi_operand_arithmetic(
                vals[0], vals[1], vals[2], vals[3], vals[4],
                vals[5], vals[6], vals[7], vals[8], vals[9]
            );
        }
    } else {
        /* Path 2: Vector operations */
        v4si vec1 = {vals[0], vals[1], vals[2], vals[3]};
        v4si vec2 = {vals[4], vals[5], vals[6], vals[7]};
        v4si vec3 = {vals[8], vals[9], vals[10], vals[11]};
        v4si vec4 = {vals[12], vals[13], vals[14], vals[15]};
        v4si vec5 = {vals[16], vals[17], vals[18], vals[19]};
        
        v4si vresult = vector_multi_ops(vec1, vec2, vec3, vec4, vec5, vec1, vec2);
        result = vresult[0] + vresult[1] + vresult[2] + vresult[3];
    }
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    if (argc > 2) {
        /* Test x86 multi-operand inline assembly */
        result += x86_multi_operand_asm(
            vals[0], vals[1], vals[2], vals[3], vals[4],
            vals[5], vals[6], vals[7], vals[8], vals[9]
        );
    }
#elif defined(__aarch64__)
    if (argc > 2) {
        /* Test ARM multi-operand inline assembly */
        result += arm_multi_operand_asm(
            vals[0], vals[1], vals[2], vals[3],
            vals[4], vals[5], vals[6], vals[7]
        );
    }
#elif defined(__powerpc__) || defined(__PPC__)
    if (argc > 2) {
        /* Test PowerPC multi-operand operations */
        result += ppc_multi_operand(
            vals[0], vals[1], vals[2], vals[3], vals[4],
            vals[5], vals[6], vals[7], vals[8]
        );
    }
#endif
    
    /* Always test multi-precision arithmetic */
    result += multi_precision_mul(
        (uint32_t)vals[0], (uint32_t)vals[1], (uint32_t)vals[2],
        (uint32_t)vals[3], (uint32_t)vals[4], (uint32_t)vals[5],
        (uint32_t)vals[6], (uint32_t)vals[7]
    );
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)result);
    
    return (int)(result % 256);
}
