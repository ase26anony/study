/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for 10 and 11-operand
 * instruction patterns in optabs.cc (lines 8254-8263).
 * 
 * Compilation for RTL analysis:
 *   gcc -O2 -fdump-rtl-expand -fdump-rtl-combine -c test.c
 *   gcc -O3 -fdump-rtl-all -dP -c test.c
 *   gcc -O2 -mtune=native -march=native -fdump-rtl-expand -c test.c
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
NOINLINE
uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, uint64_t c,
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
    uint64_t result = (t1 + t2) * (t3 + t4) + t5;
    result = (result << 5) | (result >> 59); /* rotate */
    result ^= (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j);
    
    return result;
}

/* Vector operations that might expand to multi-operand patterns */
NOINLINE
v4si vector_multi_operand(v4si a, v4si b, v4si c, v4si d,
                          v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = (t1 & t2) | g;
    v4si t4 = (t1 << 2) ^ (t2 >> 1);
    
    return t3 * t4 - a + b - c + d - e + f - g;
}

#ifdef __x86_64__
/* x86-specific inline assembly with many operands */
NOINLINE
uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                               uint64_t d, uint64_t e, uint64_t f,
                               uint64_t g, uint64_t h, uint64_t i,
                               uint64_t j) {
    uint64_t result1, result2;
    
    /* 10-operand inline assembly pattern */
    asm volatile (
        /* Complex multi-step operation using many registers */
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "add %[c], %%rax\n\t"
        "mov %[d], %%rbx\n\t"
        "imul %[e], %%rbx\n\t"
        "add %[f], %%rbx\n\t"
        "xor %%rbx, %%rax\n\t"
        "mov %[g], %%rcx\n\t"
        "add %[h], %%rcx\n\t"
        "sub %[i], %%rcx\n\t"
        "or  %[j], %%rcx\n\t"
        "imul %%rcx, %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        : [out1] "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "rax", "rbx", "rcx", "cc"
    );
    
    /* Another variant with different register usage */
    asm volatile (
        "lea (%[a], %[b], 2), %%rax\n\t"
        "lea (%[c], %[d], 4), %%rbx\n\t"
        "lea (%[e], %[f], 8), %%rcx\n\t"
        "lea (%[g], %[h], 1), %%rdx\n\t"
        "add %%rbx, %%rax\n\t"
        "add %%rcx, %%rax\n\t"
        "add %%rdx, %%rax\n\t"
        "sub %[i], %%rax\n\t"
        "add %[j], %%rax\n\t"
        "mov %%rax, %[out2]"
        : [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    return result1 ^ result2;
}

#include <x86intrin.h>
/* AVX/AVX2 operations that might expand to many operands */
NOINLINE
__m256i avx_multi_operand(__m256i a, __m256i b, __m256i c, __m256i d,
                          __m256i e, __m256i f) {
    /* Complex AVX expression */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_sub_epi32(c, d);
    __m256i t3 = _mm256_mullo_epi32(e, f);
    __m256i t4 = _mm256_slli_epi32(t1, 3);
    __m256i t5 = _mm256_srli_epi32(t2, 2);
    
    return _mm256_xor_si256(
        _mm256_or_si256(t3, t4),
        _mm256_and_si256(t5, _mm256_set1_epi32(0xFFFFFFFF))
    );
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>
/* ARM-specific NEON operations */
NOINLINE
uint64x2_t aarch64_multi_operand(uint64x2_t a, uint64x2_t b, uint64x2_t c,
                                 uint64x2_t d, uint64x2_t e, uint64x2_t f) {
    /* Complex NEON expression */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = veorq_u64(e, f);
    uint64x2_t t4 = vshlq_u64(t1, vdupq_n_u64(2));
    uint64x2_t t5 = vshrq_u64(t2, vdupq_n_u64(1));
    
    return vaddq_u64(vmulq_u64(t3, t4), t5);
}

/* ARM inline assembly with many operands */
NOINLINE
uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                               uint64_t d, uint64_t e, uint64_t f,
                               uint64_t g, uint64_t h, uint64_t i,
                               uint64_t j) {
    uint64_t result;
    
    asm volatile (
        "add %[a], %[b], %[a]\n\t"
        "add %[c], %[d], %[c]\n\t"
        "add %[e], %[f], %[e]\n\t"
        "add %[g], %[h], %[g]\n\t"
        "mul %[a], %[c], %[a]\n\t"
        "mul %[e], %[g], %[e]\n\t"
        "add %[a], %[e], %[a]\n\t"
        "add %[i], %[j], %[i]\n\t"
        "mul %[result], %[a], %[i]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values based on argc for variability */
    uint64_t vals[10];
    for (int i = 0; i < 10; i++) {
        vals[i] = (uint64_t)(argc + i) * 0x123456789ABCDEFULL;
    }
    
    /* Test 1: Complex arithmetic (potential for multi-operand expansion) */
    result ^= multi_operand_arithmetic(vals[0], vals[1], vals[2], vals[3],
                                       vals[4], vals[5], vals[6], vals[7],
                                       vals[8], vals[9]);
    
    /* Test 2: Vector operations */
    v4si vec_a = {vals[0], vals[1], vals[2], vals[3]};
    v4si vec_b = {vals[4], vals[5], vals[6], vals[7]};
    v4si vec_c = {vals[8], vals[9], vals[0], vals[1]};
    v4si vec_d = {vals[2], vals[3], vals[4], vals[5]};
    v4si vec_e = {vals[6], vals[7], vals[8], vals[9]};
    v4si vec_f = {vals[0], vals[1], vals[2], vals[3]};
    v4si vec_g = {vals[4], vals[5], vals[6], vals[7]};
    
    v4si vec_result = vector_multi_operand(vec_a, vec_b, vec_c, vec_d,
                                           vec_e, vec_f, vec_g);
    result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Test 3: Architecture-specific multi-operand patterns */
#ifdef __x86_64__
    if (argc > 1) {
        /* Use inline assembly path */
        result ^= x86_multi_operand_asm(vals[0], vals[1], vals[2], vals[3],
                                        vals[4], vals[5], vals[6], vals[7],
                                        vals[8], vals[9]);
        
        /* Test AVX if available */
        if (argc > 2) {
            __m256i avx_a = _mm256_set_epi32(vals[0], vals[1], vals[2], vals[3],
                                             vals[4], vals[5], vals[6], vals[7]);
            __m256i avx_b = _mm256_set_epi32(vals[8], vals[9], vals[0], vals[1],
                                             vals[2], vals[3], vals[4], vals[5]);
            __m256i avx_c = _mm256_set_epi32(vals[6], vals[7], vals[8], vals[9],
                                             vals[0], vals[1], vals[2], vals[3]);
            __m256i avx_d = _mm256_set_epi32(vals[4], vals[5], vals[6], vals[7],
                                             vals[8], vals[9], vals[0], vals[1]);
            __m256i avx_e = _mm256_set_epi32(vals[2], vals[3], vals[4], vals[5],
                                             vals[6], vals[7], vals[8], vals[9]);
            __m256i avx_f = _mm256_set_epi32(vals[0], vals[1], vals[2], vals[3],
                                             vals[4], vals[5], vals[6], vals[7]);
            
            __m256i avx_result = avx_multi_operand(avx_a, avx_b, avx_c,
                                                   avx_d, avx_e, avx_f);
            int *avx_ptr = (int*)&avx_result;
            for (int i = 0; i < 8; i++) {
                result += avx_ptr[i];
            }
        }
    }
#endif
    
#ifdef __aarch64__
    if (argc > 1) {
        /* Use ARM inline assembly */
        result ^= arm_multi_operand_asm(vals[0], vals[1], vals[2], vals[3],
                                        vals[4], vals[5], vals[6], vals[7],
                                        vals[8], vals[9]);
        
        /* Test NEON if available */
        if (argc > 2) {
            uint64x2_t neon_a = {vals[0], vals[1]};
            uint64x2_t neon_b = {vals[2], vals[3]};
            uint64x2_t neon_c = {vals[4], vals[5]};
            uint64x2_t neon_d = {vals[6], vals[7]};
            uint64x2_t neon_e = {vals[8], vals[9]};
            uint64x2_t neon_f = {vals[0], vals[1]};
            
            uint64x2_t neon_result = aarch64_multi_operand(neon_a, neon_b, neon_c,
                                                           neon_d, neon_e, neon_f);
            uint64_t *neon_ptr = (uint64_t*)&neon_result;
            result += neon_ptr[0] + neon_ptr[1];
        }
    }
#endif
    
    /* Additional loop to increase coverage with varying inputs */
    for (int i = 0; i < (argc % 10); i++) {
        /* Slightly modify values each iteration */
        for (int j = 0; j < 10; j++) {
            vals[j] = (vals[j] * 0x1234567 + i) & 0xFFFFFFFFFFFFFFFULL;
        }
        
        result ^= multi_operand_arithmetic(vals[0], vals[1], vals[2], vals[3],
                                           vals[4], vals[5], vals[6], vals[7],
                                           vals[8], vals[9]);
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return (int)(result & 0x7FFFFFFF);
}
