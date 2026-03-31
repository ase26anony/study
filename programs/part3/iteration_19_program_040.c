/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for various architectures */
#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Function that might trigger 10-operand expansion */
NOINLINE static uint64_t complex_operation_10(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j) {
    uint64_t result = 0;
    
    /* Complex arithmetic that might expand to many operands */
    result = ((a * b) + (c * d) + (e * f) + (g * h) + (i * j));
    result = ((result >> 32) * (a + b + c + d + e)) + ((f + g + h + i + j) << 5);
    
    /* Multi-precision arithmetic */
    uint64_t hi1, lo1, hi2, lo2;
    hi1 = (a * b) >> 32;
    lo1 = (a * b) & 0xFFFFFFFF;
    hi2 = (c * d) >> 32;
    lo2 = (c * d) & 0xFFFFFFFF;
    
    result += (hi1 + lo1 + hi2 + lo2) * (e + f + g + h + i + j);
    
    return result;
}

/* Function that might trigger 11-operand expansion */
NOINLINE static uint64_t complex_operation_11(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j, uint64_t k) {
    uint64_t result = 0;
    
    /* Even more complex expression */
    result = (((a * b * c) + (d * e * f) + (g * h * i)) / (j + k + 1));
    result = ((result << 3) | (result >> 61)) ^ (a + b + c + d + e + f + g + h + i + j + k);
    
    /* Chain of operations */
    uint64_t t1 = a * b;
    uint64_t t2 = c * d;
    uint64_t t3 = e * f;
    uint64_t t4 = g * h;
    uint64_t t5 = i * j;
    uint64_t t6 = k * (a + b);
    
    result += (t1 + t2 + t3 + t4 + t5 + t6) >> 5;
    
    return result;
}

#ifdef __x86_64__
/* x86-specific inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                               uint64_t d, uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h, uint64_t i,
                                               uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        "mov %1, %%rax\n\t"
        "mul %2\n\t"
        "mov %%rax, %0\n\t"
        "mov %%rdx, %3\n\t"
        "add %4, %0\n\t"
        "add %5, %3\n\t"
        "imul %6, %0\n\t"
        "imul %7, %3\n\t"
        "add %8, %0\n\t"
        "add %9, %3\n\t"
        "xor %0, %3\n\t"
        "mov %3, %0"
        : "=r" (result1), "+r" (result2), "+r" (result3)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "rax", "rdx", "cc"
    );
    
    return result1;
}

/* AVX-512 intrinsic that might expand to many operands */
NOINLINE static __m512i avx512_complex_op(__m512i a, __m512i b, __m512i c,
                                          __m512i d, __m512i e, __m512i f) {
    /* Complex sequence of AVX-512 operations */
    __m512i t1 = _mm512_add_epi64(a, b);
    __m512i t2 = _mm512_sub_epi64(c, d);
    __m512i t3 = _mm512_mullo_epi64(e, f);
    __m512i t4 = _mm512_slli_epi64(t1, 3);
    __m512i t5 = _mm512_srli_epi64(t2, 5);
    __m512i t6 = _mm512_xor_si512(t3, t4);
    __m512i result = _mm512_or_si512(t5, t6);
    
    return _mm512_add_epi64(result, _mm512_set1_epi64(42));
}
#endif

#ifdef __aarch64__
/* ARM-specific NEON operations */
NOINLINE static uint64x2_t arm_multi_operand_neon(uint64x2_t a, uint64x2_t b,
                                                  uint64x2_t c, uint64x2_t d,
                                                  uint64x2_t e, uint64x2_t f) {
    /* Complex NEON operation chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vshlq_n_u64(t1, 3);
    uint64x2_t t5 = vshrq_n_u64(t2, 5);
    uint64x2_t t6 = veorq_u64(t3, t4);
    uint64x2_t result = vorrq_u64(t5, t6);
    
    return vaddq_u64(result, vdupq_n_u64(42));
}
#endif

/* Main test function */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values based on argc to get varying inputs */
    uint64_t vals[12];
    for (int i = 0; i < 12; i++) {
        vals[i] = (uint64_t)(argc + i) * 123456789;
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Test 10-operand expansion */
        for (int i = 0; i < (argc % 10); i++) {
            result ^= complex_operation_10(vals[0], vals[1], vals[2], vals[3],
                                          vals[4], vals[5], vals[6], vals[7],
                                          vals[8], vals[9]);
        }
        
        /* Path 2: Test 11-operand expansion */
        if (argc > 2) {
            for (int i = 0; i < (argc % 5); i++) {
                result += complex_operation_11(vals[0], vals[1], vals[2], vals[3],
                                              vals[4], vals[5], vals[6], vals[7],
                                              vals[8], vals[9], vals[10]);
            }
        }
    } else {
        /* Default path with simpler operations */
        result = vals[0] + vals[1] + vals[2];
    }
    
#ifdef __x86_64__
    /* Test x86-specific paths */
    if (argc > 3) {
        result ^= x86_multi_operand_asm(vals[0], vals[1], vals[2], vals[3],
                                       vals[4], vals[5], vals[6], vals[7],
                                       vals[8], vals[9]);
        
        /* Test AVX-512 if available */
        #ifdef __AVX512F__
        __m512i avx_vals[6];
        for (int i = 0; i < 6; i++) {
            avx_vals[i] = _mm512_set1_epi64(vals[i]);
        }
        __m512i avx_result = avx512_complex_op(avx_vals[0], avx_vals[1],
                                               avx_vals[2], avx_vals[3],
                                               avx_vals[4], avx_vals[5]);
        /* Extract and add to result */
        uint64_t avx_extract[8];
        _mm512_storeu_si512(avx_extract, avx_result);
        for (int i = 0; i < 8; i++) {
            result += avx_extract[i];
        }
        #endif
    }
#endif

#ifdef __aarch64__
    /* Test ARM-specific paths */
    if (argc > 4) {
        uint64x2_t neon_vals[6];
        for (int i = 0; i < 6; i++) {
            neon_vals[i] = vdupq_n_u64(vals[i]);
        }
        uint64x2_t neon_result = arm_multi_operand_neon(neon_vals[0], neon_vals[1],
                                                        neon_vals[2], neon_vals[3],
                                                        neon_vals[4], neon_vals[5]);
        /* Extract and add to result */
        uint64_t neon_extract[2];
        vst1q_u64(neon_extract, neon_result);
        result += neon_extract[0] + neon_extract[1];
    }
#endif
    
    /* Use vector extensions for additional coverage */
    if (argc > 5) {
        typedef uint64_t v2ul __attribute__((vector_size(16)));
        v2ul v1 = {vals[0], vals[1]};
        v2ul v2 = {vals[2], vals[3]};
        v2ul v3 = {vals[4], vals[5]};
        v2ul v4 = {vals[6], vals[7]};
        v2ul v5 = {vals[8], vals[9]};
        
        /* Complex vector operation */
        v2ul vec_result = (v1 * v2) + (v3 * v4) + v5;
        vec_result = (vec_result << 3) | (vec_result >> 61);
        vec_result = vec_result ^ v1 ^ v2 ^ v3 ^ v4 ^ v5;
        
        /* Add to main result */
        uint64_t vec_array[2];
        __builtin_memcpy(vec_array, &vec_result, sizeof(vec_result));
        result += vec_array[0] + vec_array[1];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)result);
    
    return (int)(result % 256);
}
