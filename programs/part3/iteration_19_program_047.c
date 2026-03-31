/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, covering uncovered lines in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Then examine the .expand dump file for multi-operand patterns.
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
                                                  uint64_t i, uint64_t j)
{
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
NOINLINE static v4si vector_multi_ops(v4si a, v4si b, v4si c, v4si d,
                                      v4si e, v4si f, v4si g, v4si h)
{
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

NOINLINE static __m256i avx2_multi_operand(__m256i a, __m256i b, __m256i c,
                                          __m256i d, __m256i e, __m256i f,
                                          __m256i g, __m256i h, __m256i i)
{
    /* AVX2 operations that might require many operands */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_sub_epi32(c, d);
    __m256i t3 = _mm256_mullo_epi32(e, f);
    __m256i t4 = _mm256_slli_epi32(g, 3);
    __m256i t5 = _mm256_srli_epi32(h, 2);
    
    /* Complex combination */
    __m256i r1 = _mm256_add_epi32(t1, t2);
    __m256i r2 = _mm256_add_epi32(t3, t4);
    __m256i r3 = _mm256_add_epi32(r1, r2);
    
    return _mm256_add_epi32(r3, t5);
}

/* Extended inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b,
                                              uint64_t c, uint64_t d,
                                              uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h,
                                              uint64_t i, uint64_t j)
{
    uint64_t result1, result2;
    
    /* 10-operand inline assembly pattern */
    asm volatile (
        "mov %[a], %%rax\n\t"
        "add %[b], %%rax\n\t"
        "imul %[c], %%rax\n\t"
        "add %[d], %%rax\n\t"
        "sub %[e], %%rax\n\t"
        "xor %[f], %%rax\n\t"
        "or %[g], %%rax\n\t"
        "and %[h], %%rax\n\t"
        "add %[i], %%rax\n\t"
        "sub %[j], %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        : [out1] "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "rax", "cc"
    );
    
    /* Another with 11 operands including memory operand */
    uint64_t mem_temp = 0x12345678;
    asm volatile (
        "mov %[a], %%r8\n\t"
        "add %[b], %%r8\n\t"
        "imul %[c], %%r8\n\t"
        "add %[d], %%r8\n\t"
        "sub %[e], %%r8\n\t"
        "xor %[f], %%r8\n\t"
        "or %[g], %%r8\n\t"
        "and %[h], %%r8\n\t"
        "add %[i], %%r8\n\t"
        "sub %[j], %%r8\n\t"
        "add %[mem], %%r8\n\t"
        "mov %%r8, %[out2]\n\t"
        : [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [mem] "m" (mem_temp)
        : "r8", "cc"
    );
    
    return result1 + result2;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>

NOINLINE static uint64x2_t neon_multi_operand(uint64x2_t a, uint64x2_t b,
                                             uint64x2_t c, uint64x2_t d,
                                             uint64x2_t e, uint64x2_t f,
                                             uint64x2_t g, uint64x2_t h,
                                             uint64x2_t i, uint64x2_t j)
{
    /* NEON operations that might expand to many operands */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vshlq_u64(g, vdupq_n_u64(3));
    uint64x2_t t5 = vshrq_n_u64(h, 2);
    
    uint64x2_t r1 = vaddq_u64(t1, t2);
    uint64x2_t r2 = vaddq_u64(t3, t4);
    uint64x2_t r3 = vaddq_u64(r1, r2);
    
    return vaddq_u64(r3, t5);
}
#endif

#ifdef __powerpc64__
#include <altivec.h>

NOINLINE static vector unsigned long long altivec_multi_operand(
    vector unsigned long long a, vector unsigned long long b,
    vector unsigned long long c, vector unsigned long long d,
    vector unsigned long long e, vector unsigned long long f,
    vector unsigned long long g, vector unsigned long long h,
    vector unsigned long long i, vector unsigned long long j)
{
    /* AltiVec/VMX operations */
    vector unsigned long long t1 = vec_add(a, b);
    vector unsigned long long t2 = vec_sub(c, d);
    vector unsigned long long t3 = vec_mul(e, f);
    vector unsigned long long t4 = vec_sl(g, (vector unsigned long long){3, 3});
    vector unsigned long long t5 = vec_sr(h, (vector unsigned long long){2, 2});
    
    vector unsigned long long r1 = vec_add(t1, t2);
    vector unsigned long long r2 = vec_add(t3, t4);
    vector unsigned long long r3 = vec_add(r1, r2);
    
    return vec_add(r3, t5);
}
#endif

/* Multi-precision arithmetic that might expand to many operands */
NOINLINE static __int128 multi_precision_ops(uint64_t a, uint64_t b,
                                            uint64_t c, uint64_t d,
                                            uint64_t e, uint64_t f,
                                            uint64_t g, uint64_t h)
{
    /* 128-bit arithmetic using 64-bit parts */
    __int128 a128 = (__int128)a;
    __int128 b128 = (__int128)b;
    __int128 c128 = (__int128)c;
    __int128 d128 = (__int128)d;
    __int128 e128 = (__int128)e;
    __int128 f128 = (__int128)f;
    __int128 g128 = (__int128)g;
    __int128 h128 = (__int128)h;
    
    /* Complex expression that might require many operations */
    __int128 t1 = (a128 * b128) >> 32;
    __int128 t2 = (c128 * d128) >> 32;
    __int128 t3 = (e128 * f128) >> 32;
    __int128 t4 = (g128 * h128) >> 32;
    
    return (t1 + t2) * (t3 + t4);
}

/* Main test function */
int main(int argc, char *argv[])
{
    uint64_t result = 0;
    
    /* Initialize test values */
    uint64_t vals[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        for (int i = 0; i < argc; i++) {
            result += multi_operand_arithmetic(
                vals[0] + i, vals[1] + i, vals[2] + i, vals[3] + i,
                vals[4] + i, vals[5] + i, vals[6] + i, vals[7] + i,
                vals[8] + i, vals[9] + i);
        }
        
        /* Multi-precision arithmetic */
        result += multi_precision_ops(vals[0], vals[1], vals[2], vals[3],
                                     vals[4], vals[5], vals[6], vals[7]);
    } else {
        /* Path 2: Vector operations */
        v4si vec_a = {1, 2, 3, 4};
        v4si vec_b = {5, 6, 7, 8};
        v4si vec_c = {9, 10, 11, 12};
        v4si vec_d = {13, 14, 15, 16};
        v4si vec_e = {17, 18, 19, 20};
        v4si vec_f = {21, 22, 23, 24};
        v4si vec_g = {25, 26, 27, 28};
        v4si vec_h = {29, 30, 31, 32};
        
        v4si vec_result = vector_multi_ops(vec_a, vec_b, vec_c, vec_d,
                                          vec_e, vec_f, vec_g, vec_h);
        
        /* Sum vector elements */
        int *vr = (int*)&vec_result;
        for (int i = 0; i < 4; i++) {
            result += vr[i];
        }
    }
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    if (argc > 2) {
        /* AVX2 test */
        __m256i avx_a = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
        __m256i avx_b = _mm256_set_epi32(9, 10, 11, 12, 13, 14, 15, 16);
        __m256i avx_c = _mm256_set_epi32(17, 18, 19, 20, 21, 22, 23, 24);
        __m256i avx_d = _mm256_set_epi32(25, 26, 27, 28, 29, 30, 31, 32);
        __m256i avx_e = _mm256_set_epi32(33, 34, 35, 36, 37, 38, 39, 40);
        __m256i avx_f = _mm256_set_epi32(41, 42, 43, 44, 45, 46, 47, 48);
        __m256i avx_g = _mm256_set_epi32(49, 50, 51, 52, 53, 54, 55, 56);
        __m256i avx_h = _mm256_set_epi32(57, 58, 59, 60, 61, 62, 63, 64);
        __m256i avx_i = _mm256_set_epi32(65, 66, 67, 68, 69, 70, 71, 72);
        
        __m256i avx_result = avx2_multi_operand(avx_a, avx_b, avx_c, avx_d,
                                               avx_e, avx_f, avx_g, avx_h,
                                               avx_i);
        
        /* Sum elements */
        int *ar = (int*)&avx_result;
        for (int i = 0; i < 8; i++) {
            result += ar[i];
        }
        
        /* Extended inline assembly test */
        result += x86_multi_operand_asm(vals[0], vals[1], vals[2], vals[3],
                                       vals[4], vals[5], vals[6], vals[7],
                                       vals[8], vals[9]);
    }
#endif
    
#ifdef __aarch64__
    if (argc > 3) {
        /* NEON test */
        uint64x2_t neon_a = {1, 2};
        uint64x2_t neon_b = {3, 4};
        uint64x2_t neon_c = {5, 6};
        uint64x2_t neon_d = {7, 8};
        uint64x2_t neon_e = {9, 10};
        uint64x2_t neon_f = {11, 12};
        uint64x2_t neon_g = {13, 14};
        uint64x2_t neon_h = {15, 16};
        uint64x2_t neon_i = {17, 18};
        uint64x2_t neon_j = {19, 20};
        
        uint64x2_t neon_result = neon_multi_operand(neon_a, neon_b, neon_c,
                                                   neon_d, neon_e, neon_f,
                                                   neon_g, neon_h, neon_i,
                                                   neon_j);
        
        uint64_t nr[2];
        vst1q_u64(nr, neon_result);
        result += nr[0] + nr[1];
    }
#endif
    
#ifdef __powerpc64__
    if (argc > 4) {
        /* AltiVec test */
        vector unsigned long long alti_a = {1, 2};
        vector unsigned long long alti_b = {3, 4};
        vector unsigned long long alti_c = {5, 6};
        vector unsigned long long alti_d = {7, 8};
        vector unsigned long long alti_e = {9, 10};
        vector unsigned long long alti_f = {11, 12};
        vector unsigned long long alti_g = {13, 14};
        vector unsigned long long alti_h = {15, 16};
        vector unsigned long long alti_i = {17, 18};
        vector unsigned long long alti_j = {19, 20};
        
        vector unsigned long long alti_result = altivec_multi_operand(
            alti_a, alti_b, alti_c, alti_d, alti_e, alti_f,
            alti_g, alti_h, alti_i, alti_j);
        
        unsigned long long ar[2];
        vec_st(alti_result, 0, (vector unsigned long long*)ar);
        result += ar[0] + ar[1];
    }
#endif
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
