/* test_multi_operand.c - Test program for GCC optabs.cc 10/11-operand expansion */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test code */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Vector types for various architectures */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex arithmetic that might expand to many operands */
NOOPT uint64_t complex_mul_highpart(uint64_t a, uint64_t b, uint64_t c, 
                                     uint64_t d, uint64_t e, uint64_t f,
                                     uint64_t g, uint64_t h, uint64_t i,
                                     uint64_t j) {
    /* This complex expression might trigger multi-operand expansion */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    return t1 + t2 + t3 + t4 + t5 + (a & b) + (c & d) + (e & f) + (g & h) + (i & j);
}

/* Multi-precision arithmetic */
NOOPT void multi_precision_add(uint64_t *out, const uint64_t *a, const uint64_t *b, 
                               int size, uint64_t carry_in, uint64_t *flags,
                               uint64_t mask1, uint64_t mask2, uint64_t mask3,
                               uint64_t mask4) {
    uint64_t carry = carry_in;
    for (int i = 0; i < size; i++) {
        uint64_t sum = a[i] + b[i] + carry;
        out[i] = sum;
        carry = (sum < a[i]) || (sum < b[i]) || (carry && sum == ~(uint64_t)0);
    }
    *flags = carry | (mask1 & mask2) | (mask3 & mask4);
}

/* Vector permutation that might need many operands */
NOOPT v4si vector_permute_complex(v4si a, v4si b, v4si mask, 
                                  v4si c, v4si d, v4si e,
                                  v4si f, v4si g, v4si h,
                                  v4si i) {
    v4si result = __builtin_shuffle(a, b, mask);
    result += c + d + e + f + g + h + i;
    return result;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOOPT __m256i x86_avx2_complex_op(__m256i a, __m256i b, __m256i c,
                                  __m256i d, __m256i e, __m256i f,
                                  __m256i g, __m256i h, __m256i i,
                                  __m256i j) {
    /* FMA chain that might expand to many operands */
    __m256i t1 = _mm256_madd_epi16(a, b);
    __m256i t2 = _mm256_madd_epi16(c, d);
    __m256i t3 = _mm256_madd_epi16(e, f);
    __m256i t4 = _mm256_madd_epi16(g, h);
    __m256i t5 = _mm256_madd_epi16(i, j);
    
    return _mm256_add_epi32(_mm256_add_epi32(t1, t2),
                           _mm256_add_epi32(_mm256_add_epi32(t3, t4), t5));
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
        "and %[e], %%rax\n\t"
        "or %[f], %%rax\n\t"
        "xor %[g], %%rax\n\t"
        "shl %[h], %%rax\n\t"
        "shr %[i], %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        : [out1] "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "g" (g), [h] "c" (h & 63),
          [i] "c" (i & 63)
        : "rax", "cc"
    );
    
    /* Another with different register constraints */
    asm volatile (
        "lea (%[a],%[b],8), %[out2]\n\t"
        "add %[c], %[out2]\n\t"
        "lea (%[out2],%[d],4), %[out2]\n\t"
        "imul %[e], %[out2]\n\t"
        "add %[f], %[out2]\n\t"
        "sub %[g], %[out2]\n\t"
        "and %[h], %[out2]\n\t"
        "or %[i], %[out2]\n\t"
        "xor %[j], %[out2]\n\t"
        : [out2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

#ifdef __AVX512F__
NOOPT __m512i avx512_gather_scatter(__m512i index, __m512i mask,
                                    __m512i src1, __m512i src2,
                                    __m512i src3, __m512i src4,
                                    __m512i src5, __m512i src6,
                                    int scale, int base) {
    /* AVX-512 gather/scatter with many parameters */
    __m512i result = _mm512_mask_i32gather_epi32(src1, mask, index, 
                                                 (const void*)base, scale);
    result = _mm512_add_epi32(result, src2);
    result = _mm512_add_epi32(result, src3);
    result = _mm512_add_epi32(result, src4);
    result = _mm512_add_epi32(result, src5);
    result = _mm512_add_epi32(result, src6);
    return result;
}
#endif

#elif defined(__aarch64__)
#include <arm_neon.h>

NOOPT uint64x2_t arm_neon_complex_op(uint64x2_t a, uint64x2_t b, uint64x2_t c,
                                     uint64x2_t d, uint64x2_t e, uint64x2_t f,
                                     uint64x2_t g, uint64x2_t h, uint64x2_t i,
                                     uint64x2_t j) {
    /* ARM NEON operations that might expand to many operands */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vaddq_u64(c, d);
    uint64x2_t t3 = vaddq_u64(e, f);
    uint64x2_t t4 = vaddq_u64(g, h);
    uint64x2_t t5 = vaddq_u64(i, j);
    
    uint64x2_t result = vaddq_u64(t1, t2);
    result = vaddq_u64(result, t3);
    result = vaddq_u64(result, t4);
    result = vaddq_u64(result, t5);
    
    /* Additional operations to increase operand count */
    result = vmulq_u64(result, a);
    result = vaddq_u64(result, b);
    result = vsubq_u64(result, c);
    
    return result;
}

/* ARM SVE might have multi-operand instructions */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

NOOPT svint64_t arm_sve_multi_op(svint64_t a, svint64_t b, svint64_t c,
                                 svint64_t d, svint64_t e, svint64_t f,
                                 svint64_t g, svint64_t h, svint64_t i,
                                 svint64_t j, svbool_t pg) {
    svint64_t result = svadd_m(pg, a, b);
    result = svadd_m(pg, result, c);
    result = svadd_m(pg, result, d);
    result = svadd_m(pg, result, e);
    result = svadd_m(pg, result, f);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    result = svadd_m(pg, result, j);
    return result;
}
#endif

#elif defined(__powerpc64__) || defined(__PPC64__)
#include <altivec.h>

NOOPT vector unsigned long long ppc_altivec_complex_op(
    vector unsigned long long a, vector unsigned long long b,
    vector unsigned long long c, vector unsigned long long d,
    vector unsigned long long e, vector unsigned long long f,
    vector unsigned long long g, vector unsigned long long h,
    vector unsigned long long i, vector unsigned long long j) {
    
    vector unsigned long long t1 = vec_add(a, b);
    vector unsigned long long t2 = vec_add(c, d);
    vector unsigned long long t3 = vec_add(e, f);
    vector unsigned long long t4 = vec_add(g, h);
    vector unsigned long long t5 = vec_add(i, j);
    
    vector unsigned long long result = vec_add(t1, t2);
    result = vec_add(result, t3);
    result = vec_add(result, t4);
    result = vec_add(result, t5);
    
    /* Additional operations */
    result = vec_mul(result, a);
    result = vec_add(result, b);
    result = vec_sub(result, c);
    
    return result;
}
#endif

/* Generic fallback for architectures without specific intrinsics */
NOOPT uint64_t generic_multi_op(uint64_t a, uint64_t b, uint64_t c,
                                uint64_t d, uint64_t e, uint64_t f,
                                uint64_t g, uint64_t h, uint64_t i,
                                uint64_t j, uint64_t k) {
    /* 11-operand expression */
    return ((a * b) + (c * d) + (e * f) + (g * h) + (i * j)) / k +
           (a & b & c & d & e & f & g & h & i & j & k) +
           (a | b | c | d | e | f | g | h | i | j | k);
}

/* Test function that tries multiple approaches */
NOOPT uint64_t test_multi_operand_expansion(int variant, uint64_t seed) {
    uint64_t result = seed;
    
    /* Create many variables to force register pressure */
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
    uint64_t k = seed + 11;
    
    switch (variant % 4) {
        case 0:
            result = complex_mul_highpart(a, b, c, d, e, f, g, h, i, j);
            break;
        case 1:
            result = generic_multi_op(a, b, c, d, e, f, g, h, i, j, k);
            break;
#ifdef __x86_64__
        case 2:
            result = x86_multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
            break;
#endif
        case 3: {
            uint64_t out[4];
            uint64_t arr1[4] = {a, b, c, d};
            uint64_t arr2[4] = {e, f, g, h};
            uint64_t flags;
            multi_precision_add(out, arr1, arr2, 4, i, &flags, j, k, a, b);
            result = out[0] + out[1] + out[2] + out[3] + flags;
            break;
        }
    }
    
    /* Mix in more operations to prevent optimization */
    result ^= (result << 13);
    result ^= (result >> 17);
    result ^= (result << 5);
    
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
    
    for (int i = 0; i < iterations; i++) {
        /* Test different variants to hit different code paths */
        for (int variant = 0; variant < 4; variant++) {
            uint64_t result = test_multi_operand_expansion(variant, i + variant);
            total += result;
            
            /* Use result to prevent dead code elimination */
            if (result == 0x12345678) {
                printf("Impossible!\n");
            }
        }
    }
    
    printf("Total checksum: %lu\n", (unsigned long)total);
    
    /* Also test vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    v4si vec5 = {17, 18, 19, 20};
    v4si mask = {3, 2, 1, 0};
    
    v4si vec_result = vector_permute_complex(vec1, vec2, mask, 
                                             vec3, vec4, vec5,
                                             vec1, vec2, vec3, vec4);
    
    /* Use vector result */
    int vec_sum = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    printf("Vector sum: %d\n", vec_sum);
    
    return (total > 0) ? 0 : 1;
}
