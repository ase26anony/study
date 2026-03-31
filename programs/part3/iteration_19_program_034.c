/* Test program to trigger 10/11-operand RTL expansion in GCC's optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOOPT uint64_t multi_operand_arith(uint64_t a, uint64_t b, uint64_t c,
                                   uint64_t d, uint64_t e, uint64_t f,
                                   uint64_t g, uint64_t h, uint64_t i,
                                   uint64_t j) {
    /* Complex expression that might require many temporaries */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Multi-precision arithmetic */
    uint64_t result = t1 + t2;
    result = (result * t3) >> 32;
    result += t4;
    result = (result * t5) >> 32;
    
    /* Force use of all inputs */
    result ^= a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j;
    
    return result;
}

/* Vector operations that might expand to many operands */
NOOPT v4si vector_multi_op(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = b * c + d;
    v4si t3 = c * d + a;
    v4si t4 = d * a + b;
    
    /* Permutation-like operation */
    v4si result = __builtin_shuffle(t1, t2, (v4si){0, 1, 2, 3});
    result += __builtin_shuffle(t3, t4, (v4si){3, 2, 1, 0});
    
    /* Element-wise complex operations */
    result = (result >> 4) | (result << 28);
    result = result * a - b * c + d;
    
    return result;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOOPT __m256i x86_multi_operand_intrinsic(__m256i a, __m256i b, __m256i c,
                                          __m256i d, __m256i e, __m256i f) {
    /* AVX2 operations that might expand to many operands */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_mullo_epi32(c, d);
    __m256i t3 = _mm256_slli_epi32(e, 4);
    __m256i t4 = _mm256_srli_epi32(f, 4);
    
    /* Blend with multiple operands */
    __m256i result = _mm256_blend_epi32(t1, t2, 0xCC);
    result = _mm256_blend_epi32(result, t3, 0xAA);
    result = _mm256_blend_epi32(result, t4, 0xF0);
    
    /* Permute with many operands */
    result = _mm256_permutevar8x32_epi32(result, 
        _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0));
    
    return result;
}

/* Extended inline assembly with many operands */
NOOPT uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                     uint64_t d, uint64_t e, uint64_t f,
                                     uint64_t g, uint64_t h, uint64_t i,
                                     uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* 10-operand inline assembly */
    asm volatile (
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "add %[c], %%rax\n\t"
        "sub %[d], %%rax\n\t"
        "xor %[e], %%rax\n\t"
        "or %[f], %%rax\n\t"
        "and %[g], %%rax\n\t"
        "shl $4, %%rax\n\t"
        "shr $2, %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        : [out1] "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g)
        : "rax", "cc"
    );
    
    /* Another with different operands */
    asm volatile (
        "lea (%[a], %[b], 4), %%rax\n\t"
        "add %[c], %%rax\n\t"
        "sub %[d], %%rax\n\t"
        "imul %[e], %%rax\n\t"
        "add %[f], %%rax\n\t"
        "sub %[g], %%rax\n\t"
        "xor %[h], %%rax\n\t"
        "or %[i], %%rax\n\t"
        "and %[j], %%rax\n\t"
        "mov %%rax, %[out2]\n\t"
        : [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "cc"
    );
    
    return result1 + result2;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>

NOOPT uint64x2_t arm_multi_operand_intrinsic(uint64x2_t a, uint64x2_t b,
                                             uint64x2_t c, uint64x2_t d,
                                             uint64x2_t e, uint64x2_t f) {
    /* ARM NEON operations with many operands */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vmulq_u64(c, d);
    uint64x2_t t3 = vshlq_u64(e, vdupq_n_u64(4));
    uint64x2_t t4 = vrshrq_n_u64(f, 2);
    
    /* Multiple blend operations */
    uint64x2_t result = vbslq_u64(vdupq_n_u64(0xFFFFFFFFFFFFFFFF), t1, t2);
    result = vbslq_u64(vdupq_n_u64(0xAAAAAAAAAAAAAAAA), result, t3);
    result = vbslq_u64(vdupq_n_u64(0xCCCCCCCCCCCCCCCC), result, t4);
    
    /* Rearrangement */
    result = vextq_u64(result, result, 1);
    
    return result;
}
#endif

#ifdef __powerpc64__
#include <altivec.h>

NOOPT vector unsigned long long ppc_multi_operand_intrinsic(
    vector unsigned long long a, vector unsigned long long b,
    vector unsigned long long c, vector unsigned long long d,
    vector unsigned long long e, vector unsigned long long f) {
    
    /* PowerPC AltiVec operations */
    vector unsigned long long t1 = vec_add(a, b);
    vector unsigned long long t2 = vec_mul(c, d);
    vector unsigned long long t3 = vec_sl(e, (vector unsigned long long){4, 4});
    vector unsigned long long t4 = vec_sr(f, (vector unsigned long long){2, 2});
    
    /* Multiple blend/select operations */
    vector unsigned long long mask1 = vec_splat_u64(0xFFFFFFFFFFFFFFFF);
    vector unsigned long long mask2 = vec_splat_u64(0xAAAAAAAAAAAAAAAA);
    vector unsigned long long mask3 = vec_splat_u64(0xCCCCCCCCCCCCCCCC);
    
    vector unsigned long long result = vec_sel(t1, t2, mask1);
    result = vec_sel(result, t3, mask2);
    result = vec_sel(result, t4, mask3);
    
    /* Permute */
    result = vec_perm(result, result, (vector unsigned char)
        {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7});
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test data */
    uint64_t vals[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test multi-operand arithmetic */
    for (int i = 0; i < (argc > 1 ? atoi(argv[1]) : 10); i++) {
        /* Modify inputs slightly each iteration */
        for (int j = 0; j < 10; j++) {
            vals[j] = (vals[j] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Call multi-operand function */
        result += multi_operand_arith(vals[0], vals[1], vals[2], vals[3],
                                      vals[4], vals[5], vals[6], vals[7],
                                      vals[8], vals[9]);
        
        /* Test vector operations */
        v4si vec_a = {vals[0], vals[1], vals[2], vals[3]};
        v4si vec_b = {vals[4], vals[5], vals[6], vals[7]};
        v4si vec_c = {vals[8], vals[9], vals[0], vals[1]};
        v4si vec_d = {vals[2], vals[3], vals[4], vals[5]};
        
        v4si vec_result = vector_multi_op(vec_a, vec_b, vec_c, vec_d);
        result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        /* Architecture-specific tests */
#ifdef __x86_64__
        if (argc > 2) {
            /* Test x86 multi-operand operations */
            result += x86_multi_operand_asm(vals[0], vals[1], vals[2], vals[3],
                                            vals[4], vals[5], vals[6], vals[7],
                                            vals[8], vals[9]);
            
            /* Test AVX2 intrinsics */
            __m256i avx_a = _mm256_set_epi64x(vals[0], vals[1], vals[2], vals[3]);
            __m256i avx_b = _mm256_set_epi64x(vals[4], vals[5], vals[6], vals[7]);
            __m256i avx_c = _mm256_set_epi64x(vals[8], vals[9], vals[0], vals[1]);
            __m256i avx_d = _mm256_set_epi64x(vals[2], vals[3], vals[4], vals[5]);
            __m256i avx_e = _mm256_set_epi64x(vals[6], vals[7], vals[8], vals[9]);
            __m256i avx_f = _mm256_set_epi64x(vals[0], vals[1], vals[2], vals[3]);
            
            __m256i avx_result = x86_multi_operand_intrinsic(avx_a, avx_b, avx_c,
                                                             avx_d, avx_e, avx_f);
            uint64_t *avx_res = (uint64_t*)&avx_result;
            result += avx_res[0] + avx_res[1] + avx_res[2] + avx_res[3];
        }
#endif
        
#ifdef __aarch64__
        if (argc > 2) {
            /* Test ARM NEON operations */
            uint64x2_t neon_a = {vals[0], vals[1]};
            uint64x2_t neon_b = {vals[2], vals[3]};
            uint64x2_t neon_c = {vals[4], vals[5]};
            uint64x2_t neon_d = {vals[6], vals[7]};
            uint64x2_t neon_e = {vals[8], vals[9]};
            uint64x2_t neon_f = {vals[0], vals[1]};
            
            uint64x2_t neon_result = arm_multi_operand_intrinsic(neon_a, neon_b,
                                                                 neon_c, neon_d,
                                                                 neon_e, neon_f);
            result += vgetq_lane_u64(neon_result, 0) + 
                      vgetq_lane_u64(neon_result, 1);
        }
#endif
        
#ifdef __powerpc64__
        if (argc > 2) {
            /* Test PowerPC AltiVec operations */
            vector unsigned long long altivec_a = {vals[0], vals[1]};
            vector unsigned long long altivec_b = {vals[2], vals[3]};
            vector unsigned long long altivec_c = {vals[4], vals[5]};
            vector unsigned long long altivec_d = {vals[6], vals[7]};
            vector unsigned long long altivec_e = {vals[8], vals[9]};
            vector unsigned long long altivec_f = {vals[0], vals[1]};
            
            vector unsigned long long altivec_result = 
                ppc_multi_operand_intrinsic(altivec_a, altivec_b, altivec_c,
                                           altivec_d, altivec_e, altivec_f);
            result += altivec_result[0] + altivec_result[1];
        }
#endif
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return 0;
}
