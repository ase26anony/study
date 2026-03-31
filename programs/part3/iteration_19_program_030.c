/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for 10 and 11-operand
 * instruction patterns, covering lines 8254-8263 in optabs.cc.
 * 
 * Compilation for coverage analysis:
 *   gcc -O2 -fdump-rtl-expand -fdump-rtl-combine -c test_optabs_10_11_operands.c
 *   gcc -O3 -fdump-rtl-all -dP -c test_optabs_10_11_operands.c
 *   gcc -O2 -mtune=native -march=native -fdump-rtl-expand -c test_optabs_10_11_operands.c
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

/* Complex arithmetic that might expand to multi-operand patterns */
NOINLINE static uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, 
                                                  uint64_t c, uint64_t d,
                                                  uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h,
                                                  uint64_t i, uint64_t j) {
    /* Complex expression that might require many temporaries */
    uint64_t t1 = (a * b) >> 32;      /* High-part multiplication */
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Chain of operations that might expand to many operands */
    uint64_t result = t1 + t2;
    result = result * t3;
    result = result + t4;
    result = result * t5;
    
    /* Mix with bit manipulations */
    result = (result << 5) | (result >> 59);
    result ^= (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j);
    
    return result;
}

/* Multi-precision arithmetic - often expands to many operations */
NOINLINE static __int128 multi_precision_mul(uint64_t a, uint64_t b, 
                                             uint64_t c, uint64_t d) {
    /* 128-bit multiplication using 64-bit parts */
    __int128 a128 = a;
    __int128 b128 = b;
    __int128 c128 = c;
    __int128 d128 = d;
    
    /* Complex expression that might expand to many RTL operands */
    __int128 result = (a128 * b128) + (c128 * d128);
    result = result * (a128 + b128 + c128 + d128);
    
    /* Additional operations to increase operand count */
    result = result >> 32;
    result = result * (result + 1);
    
    return result;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* x86-specific inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h,
                                               uint64_t i, uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with many input/output operands */
    asm volatile (
        /* Complex operation with many operands */
        "mov %[a], %%rax\n\t"
        "mul %[b]\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %%rdx, %[r2]\n\t"
        "mov %[c], %%rax\n\t"
        "mul %[d]\n\t"
        "add %%rax, %[r1]\n\t"
        "adc %%rdx, %[r2]\n\t"
        "mov %[e], %%rax\n\t"
        "mul %[f]\n\t"
        "add %%rax, %[r1]\n\t"
        "adc %%rdx, %[r2]\n\t"
        "mov %[g], %%rax\n\t"
        "mul %[h]\n\t"
        "add %%rax, %[r1]\n\t"
        "adc %%rdx, %[r2]\n\t"
        "mov %[i], %%rax\n\t"
        "mul %[j]\n\t"
        "add %%rax, %[r1]\n\t"
        "adc %%rdx, %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rdx", "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

/* AVX-512 operations that might expand to many operands */
NOINLINE static __m512i avx512_complex_op(__m512i a, __m512i b, __m512i c,
                                          __m512i d, __m512i e) {
    /* Complex sequence of AVX-512 operations */
    __m512i t1 = _mm512_add_epi64(a, b);
    __m512i t2 = _mm512_sub_epi64(c, d);
    __m512i t3 = _mm512_mullo_epi64(t1, t2);
    __m512i t4 = _mm512_slli_epi64(e, 2);
    __m512i t5 = _mm512_xor_si512(t3, t4);
    __m512i t6 = _mm512_rol_epi64(t5, 13);
    
    return _mm512_add_epi64(t6, _mm512_set1_epi64(42));
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM-specific NEON operations */
NOINLINE static uint64x2_t aarch64_multi_operand_neon(uint64x2_t a, uint64x2_t b,
                                                      uint64x2_t c, uint64x2_t d,
                                                      uint64x2_t e, uint64x2_t f) {
    /* Complex NEON operation chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(t1, t2);
    uint64x2_t t4 = vshlq_u64(e, vdupq_n_u64(2));
    uint64x2_t t5 = veorq_u64(t3, t4);
    uint64x2_t t6 = vrshrq_n_u64(t5, 13);
    
    return vaddq_u64(t6, vdupq_n_u64(42));
}

/* ARM crypto extensions might use many operands */
NOINLINE static uint64_t aarch64_crypto_like(uint64_t a, uint64_t b,
                                             uint64_t c, uint64_t d,
                                             uint64_t e, uint64_t f,
                                             uint64_t g, uint64_t h) {
    uint64_t result;
    
    /* Inline assembly simulating complex operation */
    asm volatile (
        "eor %[a], %[a], %[b]\n\t"
        "eor %[c], %[c], %[d]\n\t"
        "eor %[e], %[e], %[f]\n\t"
        "eor %[g], %[g], %[h]\n\t"
        "add %[a], %[a], %[c]\n\t"
        "add %[e], %[e], %[g]\n\t"
        "add %[res], %[a], %[e]\n\t"
        "ror %[res], %[res], #32\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc"
    );
    
    return result;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
/* PowerPC-specific operations */
NOINLINE static uint64_t powerpc_multi_operand(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h) {
    uint64_t result;
    
    /* PowerPC inline assembly with many operands */
    asm volatile (
        "mulld %0, %1, %2\n\t"
        "mulhdu %3, %4, %5\n\t"
        "add %0, %0, %3\n\t"
        "mulld %3, %6, %7\n\t"
        "add %0, %0, %3\n\t"
        "rotldi %0, %0, 32\n\t"
        : "=&r" (result), "+r" (a), "+r" (b), "=&r" (c)
        : "r" (d), "r" (e), "r" (f), "r" (g), "r" (h)
        : "cc"
    );
    
    return result;
}
#endif

/* Vector operations that might expand to multi-operand RTL */
NOINLINE static v4si vector_complex_op(v4si a, v4si b, v4si c, v4si d,
                                       v4si e, v4si f) {
    /* Complex vector expression */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = t1 * t2;
    v4si t4 = e << 2;
    v4si t5 = t3 ^ t4;
    v4si t6 = (t5 >> 13) | (t5 << (32 - 13));
    
    return t6 + (v4si){42, 42, 42, 42};
}

/* Function that combines multiple expansion strategies */
NOINLINE static uint64_t combined_multi_operand_test(int argc, char **argv) {
    uint64_t a = 0x123456789ABCDEF0ULL;
    uint64_t b = 0xFEDCBA9876543210ULL;
    uint64_t c = 0xAAAAAAAAAAAAAAAAULL;
    uint64_t d = 0x5555555555555555ULL;
    uint64_t e = 0x3333333333333333ULL;
    uint64_t f = 0xCCCCCCCCCCCCCCCCULL;
    uint64_t g = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t h = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t i = 0x00FF00FF00FF00FFULL;
    uint64_t j = 0xFF00FF00FF00FF00ULL;
    
    uint64_t result = 0;
    
    /* Use command-line arguments to vary the execution path */
    if (argc > 10) {
        /* Test with maximum number of operands */
        result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
    } else if (argc > 5) {
        /* Test multi-precision arithmetic */
        __int128 mp_result = multi_precision_mul(a, b, c, d);
        result = (uint64_t)mp_result + (uint64_t)(mp_result >> 64);
    } else {
        /* Default path with mixed operations */
        result = a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j;
    }
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    if (argc > 1) {
        uint64_t asm_result = x86_multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
        result += asm_result;
    }
#elif defined(__aarch64__)
    if (argc > 1) {
        uint64_t neon_result = aarch64_crypto_like(a, b, c, d, e, f, g, h);
        result += neon_result;
    }
#elif defined(__powerpc64__) || defined(__PPC64__)
    if (argc > 1) {
        uint64_t ppc_result = powerpc_multi_operand(a, b, c, d, e, f, g, h);
        result += ppc_result;
    }
#endif
    
    /* Vector operations test */
    if (argc > 2) {
        v4si va = {1, 2, 3, 4};
        v4si vb = {5, 6, 7, 8};
        v4si vc = {9, 10, 11, 12};
        v4si vd = {13, 14, 15, 16};
        v4si ve = {17, 18, 19, 20};
        v4si vf = {21, 22, 23, 24};
        
        v4si vresult = vector_complex_op(va, vb, vc, vd, ve, vf);
        result += vresult[0] + vresult[1] + vresult[2] + vresult[3];
    }
    
    return result;
}

int main(int argc, char **argv) {
    uint64_t final_result = 0;
    
    /* Loop with varying inputs to explore different expansion paths */
    for (int iter = 0; iter < (argc > 0 ? argc : 1); iter++) {
        /* Modify operands slightly each iteration */
        uint64_t base = (uint64_t)iter * 0x1001;
        
        /* Call the test function with modified arguments */
        final_result ^= combined_multi_operand_test(argc + iter, argv);
        
        /* Additional arithmetic to prevent dead code elimination */
        final_result = (final_result << 5) | (final_result >> 59);
        final_result += base;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    return (final_result == 0) ? 1 : 0;
}
