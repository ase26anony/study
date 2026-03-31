/* Test program to trigger 10/11-operand RTL expansion in GCC's optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from eliminating our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

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
    return ((t1 + t2) * (t3 + t4)) ^ t5;
}

/* Vector operations that might expand to many operands */
NOINLINE
v4si vector_multi_ops(v4si a, v4si b, v4si c, v4si d,
                      v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = e * f;
    v4si t4 = g & a;
    v4si t5 = b | c;
    
    /* Mix operations - might expand to many operands */
    return (t1 * t2) + (t3 & t4) | t5;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE
__m128i x86_multi_operand_intrinsic(__m128i a, __m128i b, __m128i c,
                                    __m128i d, __m128i e, __m128i f,
                                    __m128i g, __m128i h) {
    /* Complex chain of SSE/AVX operations */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(c, d);
    __m128i t3 = _mm_mullo_epi32(e, f);
    __m128i t4 = _mm_and_si128(g, h);
    __m128i t5 = _mm_or_si128(t1, t2);
    
    /* More operations to increase operand count */
    __m128i t6 = _mm_xor_si128(t3, t4);
    __m128i t7 = _mm_slli_epi32(t5, 3);
    __m128i t8 = _mm_srli_epi32(t6, 2);
    
    return _mm_add_epi32(t7, t8);
}

/* Extended inline assembly with many operands */
NOINLINE
uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                               uint64_t d, uint64_t e, uint64_t f,
                               uint64_t g, uint64_t h, uint64_t i,
                               uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Inline assembly with many input/output/clobber operands */
    asm volatile (
        /* Complex multi-step operation using many registers */
        "mov %[a], %%rax\n\t"
        "mul %[b]\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %%rdx, %[r2]\n\t"
        "mov %[c], %%rax\n\t"
        "mul %[d]\n\t"
        "add %[r1], %%rax\n\t"
        "adc %[r2], %%rdx\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %[e], %%rax\n\t"
        "mul %[f]\n\t"
        "sub %[r1], %%rax\n\t"
        "sbb $0, %%rdx\n\t"
        "mov %%rax, %[r3]\n\t"
        "mov %[g], %%rax\n\t"
        "mul %[h]\n\t"
        "xor %[r3], %%rax\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %[i], %%rax\n\t"
        "mul %[j]\n\t"
        "or %[r1], %%rax\n\t"
        : [r1] "=&r" (result1),
          [r2] "=&r" (result2),
          [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "rax", "rdx", "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOINLINE
uint64x2_t arm_multi_operand_intrinsic(uint64x2_t a, uint64x2_t b,
                                       uint64x2_t c, uint64x2_t d,
                                       uint64x2_t e, uint64x2_t f,
                                       uint64x2_t g, uint64x2_t h) {
    /* Complex NEON operations chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vandq_u64(g, h);
    uint64x2_t t5 = vorrq_u64(t1, t2);
    uint64x2_t t6 = veorq_u64(t3, t4);
    uint64x2_t t7 = vshlq_u64(t5, vdupq_n_u64(3));
    uint64x2_t t8 = vshrq_n_u64(t6, 2);
    
    return vaddq_u64(t7, t8);
}

#elif defined(__powerpc64__) || defined(__PPC64__)
/* PowerPC specific multi-operand operations */
NOINLINE
unsigned long long ppc_multi_operand(unsigned long long a,
                                     unsigned long long b,
                                     unsigned long long c,
                                     unsigned long long d,
                                     unsigned long long e,
                                     unsigned long long f,
                                     unsigned long long g,
                                     unsigned long long h,
                                     unsigned long long i,
                                     unsigned long long j) {
    unsigned long long result;
    
    /* PowerPC extended inline assembly with many operands */
    asm volatile (
        "mulld %0, %1, %2\n\t"
        "mulhdu %3, %4, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "xor %0, %0, %8\n\t"
        "or %0, %0, %9\n\t"
        "and %0, %0, %10\n\t"
        : "=&r" (result)
        : "r" (a), "r" (b), "r" (c),
          "r" (d), "r" (e), "r" (f),
          "r" (g), "r" (h), "r" (i),
          "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Multi-precision arithmetic that might expand to many operands */
NOINLINE
void multi_precision_calc(uint64_t *result,
                          const uint64_t *a,
                          const uint64_t *b,
                          int size) {
    /* Manual multi-precision addition */
    uint64_t carry = 0;
    for (int i = 0; i < size; i++) {
        uint64_t sum = a[i] + b[i] + carry;
        carry = (sum < a[i]) || (carry && sum == a[i]);
        result[i] = sum;
    }
}

/* Test function that combines multiple expansion strategies */
NOINLINE
uint64_t test_multi_operand_expansion(int variant, uint64_t seed) {
    uint64_t result = seed;
    
    /* Initialize many variables to use as operands */
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
    uint64_t l = seed + 12;
    
    switch (variant % 4) {
        case 0:
            /* Complex arithmetic expression */
            result = multi_operand_arithmetic(a, b, c, d, e, f, g, h, i, j);
            break;
            
        case 1:
            /* Architecture-specific multi-operand operation */
#ifdef __x86_64__
            result = x86_multi_operand_asm(a, b, c, d, e, f, g, h, i, j);
#elif defined(__aarch64__) || defined(__powerpc64__) || defined(__PPC64__)
            /* Fallback for other architectures */
            result = ((a * b) + (c * d) + (e * f) + (g * h) + (i * j)) ^ k;
#endif
            break;
            
        case 2:
            /* Multi-precision arithmetic */
            {
                uint64_t mp_a[4] = {a, b, c, d};
                uint64_t mp_b[4] = {e, f, g, h};
                uint64_t mp_result[4];
                multi_precision_calc(mp_result, mp_a, mp_b, 4);
                result = mp_result[0] + mp_result[1] + mp_result[2] + mp_result[3];
            }
            break;
            
        case 3:
            /* Mixed operations to confuse the optimizer */
            result = (((a * b) >> (c & 31)) +
                     ((d * e) >> (f & 31)) +
                     ((g * h) >> (i & 31)) +
                     ((j * k) >> (l & 31)));
            break;
    }
    
    /* Add some trivial computation to prevent dead code elimination */
    return result ^ (result >> 32);
}

int main(int argc, char *argv[]) {
    uint64_t total = 0;
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Test with different variants and seeds */
    for (int iter = 0; iter < iterations; iter++) {
        for (int variant = 0; variant < 4; variant++) {
            uint64_t seed = (iter * 100) + variant;
            uint64_t result = test_multi_operand_expansion(variant, seed);
            
            /* Use result to prevent optimization */
            total += result;
            
            /* Add some branching to affect optimization decisions */
            if (result & 1) {
                total ^= result;
            } else {
                total += (result << 1);
            }
        }
    }
    
    /* Also test vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    v4si vec_e = {17, 18, 19, 20};
    v4si vec_f = {21, 22, 23, 24};
    v4si vec_g = {25, 26, 27, 28};
    
    v4si vec_result = vector_multi_ops(vec_a, vec_b, vec_c, vec_d,
                                       vec_e, vec_f, vec_g);
    
    /* Use vector result */
    for (int i = 0; i < 4; i++) {
        total += vec_result[i];
    }
    
    printf("Result: %lu\n", (unsigned long)total);
    return 0;
}
