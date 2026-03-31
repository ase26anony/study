/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for 10 and 11 operand cases
 * in optabs.cc lines 8254-8263. It uses target-specific intrinsics, inline
 * assembly, and complex arithmetic expressions that may expand to multi-operand
 * RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure local expansion */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t complex_mul_highpart(uint64_t a, uint64_t b, uint64_t c, 
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i) {
    /* Multi-precision arithmetic that might use expand_mult_highpart */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    
    /* Chain of operations that might combine into many operands */
    return ((t1 + t2) * (t3 + t4)) >> 32 + i;
}

/* Function with many operands in expression */
NOINLINE int many_operand_expr(int a, int b, int c, int d, int e,
                               int f, int g, int h, int i, int j) {
    /* Complex expression that might expand to many RTL operands */
    return ((a * b) + (c * d) - (e * f) + (g * h) - (i * j)) >> 3;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE __m256i x86_avx2_multi_operand(__m256i a, __m256i b, __m256i c,
                                       __m256i d, __m256i e, __m256i f,
                                       __m256i g, __m256i h) {
    /* AVX2 operations that might decompose to many operands */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_sub_epi32(c, d);
    __m256i t3 = _mm256_mullo_epi32(e, f);
    __m256i t4 = _mm256_slli_epi32(g, 4);
    
    /* Blend with many operands */
    return _mm256_blend_epi32(t1, t2, 0xAA);
}

/* Inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result;
    
    /* Extended inline assembly with many input/output operands */
    asm volatile (
        /* Complex operation with many registers */
        "mov %[a], %%rax\n\t"
        "mul %[b]\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %[c], %%rax\n\t"
        "mul %[d]\n\t"
        "add %%rax, %[r1]\n\t"
        "mov %[e], %%rax\n\t"
        "mul %[f]\n\t"
        "sub %%rax, %[r1]\n\t"
        "mov %[g], %%rax\n\t"
        "mul %[h]\n\t"
        "add %%rax, %[r1]\n\t"
        "mov %[r1], %%rax\n\t"
        "xor %[i], %%rax\n\t"
        "mov %%rax, %[result]"
        : [result] "=r" (result), [r1] "=&r" (a)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "rax", "rdx", "cc"
    );
    
    return result;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOINLINE int32x4_t arm_neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f,
                                         int32x4_t g, int32x4_t h) {
    /* ARM NEON operations that might use many operands */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vsubq_s32(c, d);
    int32x4_t t3 = vmulq_s32(e, f);
    int32x4_t t4 = vshlq_s32(g, vdupq_n_s32(4));
    
    /* Complex lane operations */
    return vmlaq_laneq_s32(t1, t2, h, 1);
}

/* ARM inline assembly with many operands */
NOINLINE uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result;
    
    asm volatile (
        /* Complex ARM64 operation with many registers */
        "mul %x[result], %x[a], %x[b]\n\t"
        "madd %x[result], %x[c], %x[d], %x[result]\n\t"
        "msub %x[result], %x[e], %x[f], %x[result]\n\t"
        "madd %x[result], %x[g], %x[h], %x[result]\n\t"
        "eor %x[result], %x[result], %x[i]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    return result;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
#include <altivec.h>

NOINLINE vector signed int ppc_altivec_multi_operand(vector signed int a,
                                                    vector signed int b,
                                                    vector signed int c,
                                                    vector signed int d,
                                                    vector signed int e,
                                                    vector signed int f) {
    /* PowerPC AltiVec operations */
    vector signed int t1 = vec_add(a, b);
    vector signed int t2 = vec_sub(c, d);
    vector signed int t3 = vec_mul(e, f);
    
    /* Complex merge operation */
    return vec_mergeh(t1, vec_mergel(t2, t3));
}

#else
/* Generic fallback for other architectures */
NOINLINE int64_t generic_multi_operand(int64_t a, int64_t b, int64_t c,
                                      int64_t d, int64_t e, int64_t f,
                                      int64_t g, int64_t h, int64_t i,
                                      int64_t j) {
    /* Very complex expression that might expand to many RTL operands */
    return (((((a * b) >> 4) + ((c * d) >> 4)) * 
             (((e * f) >> 4) + ((g * h) >> 4))) >> 8) + i - j;
}
#endif

/* Test function that combines multiple approaches */
NOINLINE uint64_t test_multi_operand_expansion(uint64_t a, uint64_t b, 
                                              uint64_t c, uint64_t d,
                                              uint64_t seed) {
    uint64_t result = 0;
    
    /* Use seed to vary the operations */
    if (seed & 1) {
        /* Path 1: Complex arithmetic expressions */
        result += complex_mul_highpart(a, b, c, d, 
                                       seed, seed+1, seed+2, seed+3, seed+4);
    }
    
    if (seed & 2) {
        /* Path 2: Many operand expression */
        result += many_operand_expr((int)a, (int)b, (int)c, (int)d, (int)seed,
                                   (int)(seed+1), (int)(seed+2), (int)(seed+3),
                                   (int)(seed+4), (int)(seed+5));
    }
    
#ifdef __x86_64__
    if (seed & 4) {
        /* Path 3: x86-specific operations */
        __m256i va = _mm256_set_epi64x(a, b, c, d);
        __m256i vb = _mm256_set_epi64x(b, c, d, a);
        __m256i vc = _mm256_set_epi64x(c, d, a, b);
        __m256i vd = _mm256_set_epi64x(d, a, b, c);
        __m256i ve = _mm256_set_epi64x(seed, seed+1, seed+2, seed+3);
        __m256i vf = _mm256_set_epi64x(seed+1, seed+2, seed+3, seed+4);
        __m256i vg = _mm256_set_epi64x(seed+2, seed+3, seed+4, seed+5);
        __m256i vh = _mm256_set_epi64x(seed+3, seed+4, seed+5, seed+6);
        
        __m256i vr = x86_avx2_multi_operand(va, vb, vc, vd, ve, vf, vg, vh);
        result += _mm256_extract_epi64(vr, 0);
    }
    
    if (seed & 8) {
        /* Path 4: x86 inline assembly */
        result += x86_multi_operand_asm(a, b, c, d, seed, seed+1, 
                                       seed+2, seed+3, seed+4);
    }
#elif defined(__aarch64__)
    if (seed & 4) {
        /* Path 3: ARM-specific operations */
        int32x4_t va = vdupq_n_s32(a);
        int32x4_t vb = vdupq_n_s32(b);
        int32x4_t vc = vdupq_n_s32(c);
        int32x4_t vd = vdupq_n_s32(d);
        int32x4_t ve = vdupq_n_s32(seed);
        int32x4_t vf = vdupq_n_s32(seed+1);
        int32x4_t vg = vdupq_n_s32(seed+2);
        int32x4_t vh = vdupq_n_s32(seed+3);
        
        int32x4_t vr = arm_neon_multi_operand(va, vb, vc, vd, ve, vf, vg, vh);
        result += vgetq_lane_s32(vr, 0);
    }
    
    if (seed & 8) {
        /* Path 4: ARM inline assembly */
        result += arm_multi_operand_asm(a, b, c, d, seed, seed+1,
                                       seed+2, seed+3, seed+4);
    }
#else
    if (seed & 4) {
        /* Path 3: Generic fallback */
        result += generic_multi_operand(a, b, c, d, seed, seed+1,
                                       seed+2, seed+3, seed+4, seed+5);
    }
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Use command line arguments to vary execution paths */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    /* Initialize with varying values */
    uint64_t a = 0x123456789ABCDEF0ULL;
    uint64_t b = 0xFEDCBA9876543210ULL;
    uint64_t c = 0x13579BDF2468ACE0ULL;
    uint64_t d = 0x0ECA8642FDB97531ULL;
    
    /* Loop to increase coverage and vary inputs */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed based on iteration and command line */
        uint64_t seed = (uint64_t)(i + argc);
        
        /* Call the test function with varying parameters */
        result ^= test_multi_operand_expansion(a + i, b + i, c + i, d + i, seed);
        
        /* Modify values to explore different code paths */
        a = (a << 1) | (a >> 63);
        b = (b << 5) | (b >> 59);
        c = (c << 13) | (c >> 51);
        d = (d << 17) | (d >> 47);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llX\n", (unsigned long long)result);
    
    return (result == 0) ? 0 : 1;
}
