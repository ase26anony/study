/* Test program to trigger 10/11-operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization from eliminating our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t multi_operand_arith(uint64_t a, uint64_t b, uint64_t c,
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
NOINLINE v4si vector_ops(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = a * c + d;
    v4si t3 = b * d + a;
    v4si t4 = c * d + b;
    
    /* Mix all vectors - this might expand to many operands */
    return (t1 & t2) | (t3 ^ t4);
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* x86-specific inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        /* Complex multi-operand operation */
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
    
    return result1 ^ result2 ^ result3;
}

/* AVX-512 operations that might use many operands */
NOINLINE __m512i avx512_complex_op(__m512i a, __m512i b, __m512i c,
                                   __m512i d, __m512i e) {
#if defined(__AVX512F__)
    /* Complex AVX-512 expression with masking */
    __mmask16 mask = 0xAAAA;
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_sub_epi32(c, d);
    __m512i t3 = _mm512_mullo_epi32(t1, t2);
    __m512i t4 = _mm512_maskz_add_epi32(mask, t3, e);
    return _mm512_xor_si512(t4, a);
#else
    return a;
#endif
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM-specific NEON operations */
NOINLINE uint64x2_t arm_multi_operand_neon(uint64x2_t a, uint64x2_t b,
                                           uint64x2_t c, uint64x2_t d,
                                           uint64x2_t e, uint64x2_t f) {
    /* Complex NEON expression */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vaddq_u64(c, d);
    uint64x2_t t3 = vaddq_u64(e, f);
    uint64x2_t t4 = vmulq_u64(t1, t2);
    return veorq_u64(t4, t3);
}

#elif defined(__powerpc64__)
#include <altivec.h>

/* PowerPC AltiVec operations */
NOINLINE vector unsigned long long ppc_multi_operand_vec(vector unsigned long long a,
                                                         vector unsigned long long b,
                                                         vector unsigned long long c,
                                                         vector unsigned long long d) {
    /* Complex AltiVec expression */
    vector unsigned long long t1 = vec_add(a, b);
    vector unsigned long long t2 = vec_add(c, d);
    vector unsigned long long t3 = vec_mul(t1, t2);
    vector unsigned long long t4 = vec_xor(t3, a);
    return vec_add(t4, b);
}
#endif

/* Function that uses different expansion paths based on optimization context */
NOINLINE uint64_t conditional_multi_op(int mode, uint64_t a, uint64_t b,
                                       uint64_t c, uint64_t d, uint64_t e,
                                       uint64_t f, uint64_t g, uint64_t h,
                                       uint64_t i, uint64_t j) {
    if (mode == 0) {
        /* Path 1: Complex arithmetic expression */
        return multi_operand_arith(a, b, c, d, e, f, g, h, i, j);
    } else if (mode == 1) {
#ifdef __x86_64__
        /* Path 2: x86 inline assembly */
        return x86_multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
#else
        return a + b;
#endif
    } else {
        /* Path 3: Mixed operations */
        uint64_t t1 = (a * b + c * d) >> 16;
        uint64_t t2 = (e * f + g * h) >> 16;
        uint64_t t3 = (i * j + a * c) >> 16;
        return (t1 * t2) / (t3 + 1);
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values */
    uint64_t vals[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test different code paths based on command line */
    int mode = (argc > 1) ? atoi(argv[1]) % 3 : 0;
    
    /* Loop to increase coverage */
    for (int iter = 0; iter < (argc > 2 ? atoi(argv[2]) : 100); iter++) {
        /* Modify values slightly each iteration */
        for (int i = 0; i < 10; i++) {
            vals[i] = (vals[i] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Call multi-operand function */
        result ^= conditional_multi_op(mode, 
                                      vals[0], vals[1], vals[2], vals[3],
                                      vals[4], vals[5], vals[6], vals[7],
                                      vals[8], vals[9]);
        
        /* Also test vector operations */
        v4si vec_a = {vals[0], vals[1], vals[2], vals[3]};
        v4si vec_b = {vals[4], vals[5], vals[6], vals[7]};
        v4si vec_c = {vals[8], vals[9], vals[0], vals[1]};
        v4si vec_d = {vals[2], vals[3], vals[4], vals[5]};
        
        v4si vec_result = vector_ops(vec_a, vec_b, vec_c, vec_d);
        
        /* Use vector result to prevent elimination */
        for (int i = 0; i < 4; i++) {
            result += vec_result[i];
        }
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
