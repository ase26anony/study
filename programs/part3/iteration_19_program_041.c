/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Target-specific multi-operand operations */
#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* Complex inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                               uint64_t d, uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2, result3;
    
    /* 10-operand inline assembly (1 output + 9 inputs) */
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "subq %[d], %[b]\n\t"
        "xorq %[e], %[b]\n\t"
        "orq %[f], %[b]\n\t"
        "andq %[g], %[b]\n\t"
        "shlq $3, %[b]\n\t"
        "rorq $5, %[b]\n\t"
        "movq %[b], %[r1]\n\t"
        : [r1] "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    /* AVX-512 operation that might expand to many operands */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i vec1 = _mm512_set1_epi32(a);
        __m512i vec2 = _mm512_set1_epi32(b);
        __m512i vec3 = _mm512_set1_epi32(c);
        __m512i mask = _mm512_set1_epi32(0xFFFFFFFF);
        
        /* Complex AVX-512 operation */
        __m512i res = _mm512_mask_add_epi32(vec1, (__mmask16)mask, vec2, vec3);
        result2 = _mm512_reduce_add_epi32(res);
    } else {
        result2 = a + b + c;
    }
    
    return result1 + result2;
}

/* Multi-precision arithmetic that might expand to many operations */
NOINLINE static uint64_t x86_mult_highpart(uint64_t a, uint64_t b) {
    /* (a * b) >> 64 - might expand to multi-operand RTL */
    unsigned __int128 product = (unsigned __int128)a * (unsigned __int128)b;
    return (uint64_t)(product >> 64);
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOINLINE static uint64_t arm_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2;
    
    /* ARM inline assembly with many operands */
    asm volatile (
        "mul %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "sub %[r1], %[r1], %[d]\n\t"
        "eor %[r1], %[r1], %[e]\n\t"
        "orr %[r1], %[r1], %[f]\n\t"
        "and %[r1], %[r1], %[g]\n\t"
        "lsl %[r1], %[r1], #3\n\t"
        "ror %[r1], %[r1], #5\n\t"
        : [r1] "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    /* NEON operations that might expand to many operands */
    uint64x2_t vec1 = vdupq_n_u64(a);
    uint64x2_t vec2 = vdupq_n_u64(b);
    uint64x2_t vec3 = vdupq_n_u64(c);
    
    uint64x2_t res = vaddq_u64(vec1, vec2);
    res = vaddq_u64(res, vec3);
    
    result2 = vgetq_lane_u64(res, 0) + vgetq_lane_u64(res, 1);
    
    return result1 + result2;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
#include <altivec.h>

NOINLINE static uint64_t ppc_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2;
    
    /* PowerPC inline assembly with many operands */
    asm volatile (
        "mulld %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "subf %[r1], %[d], %[r1]\n\t"
        "xor %[r1], %[r1], %[e]\n\t"
        "or %[r1], %[r1], %[f]\n\t"
        "and %[r1], %[r1], %[g]\n\t"
        "sldi %[r1], %[r1], 3\n\t"
        "rotrdi %[r1], %[r1], 5\n\t"
        : [r1] "=r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    /* AltiVec/VMX operations */
    vector unsigned long long vec1 = (vector unsigned long long){a, b};
    vector unsigned long long vec2 = (vector unsigned long long){c, d};
    
    vector unsigned long long res = vec_add(vec1, vec2);
    result2 = res[0] + res[1];
    
    return result1 + result2;
}

#else
/* Generic fallback - still try to create multi-operand expressions */
NOINLINE static uint64_t generic_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                               uint64_t d, uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h, uint64_t i) {
    /* Complex expression that might expand to many RTL operands */
    uint64_t t1 = a * b;
    uint64_t t2 = c + d;
    uint64_t t3 = e ^ f;
    uint64_t t4 = g & h;
    uint64_t t5 = t1 + t2;
    uint64_t t6 = t3 | t4;
    uint64_t t7 = t5 - t6;
    uint64_t t8 = t7 << 3;
    uint64_t t9 = (t8 >> 5) | (t8 << (64 - 5));
    
    return t9 + i;
}
#endif

/* Vector operations that might trigger multi-operand expansion */
NOINLINE static v4si vector_multi_op(v4si a, v4si b, v4si c, v4si d) {
    /* Complex vector expression */
    v4si t1 = a * b;
    v4si t2 = c + d;
    v4si t3 = t1 & t2;
    v4si t4 = t1 | t2;
    v4si t5 = t3 ^ t4;
    v4si t6 = t5 << 2;
    v4si t7 = t6 >> 1;
    
    return t7 + a + b + c + d;
}

/* Function that uses many variables to force register pressure */
NOINLINE static uint64_t create_register_pressure(uint64_t a, uint64_t b, uint64_t c,
                                                  uint64_t d, uint64_t e, uint64_t f,
                                                  uint64_t g, uint64_t h, uint64_t i,
                                                  uint64_t j, uint64_t k, uint64_t l) {
    /* Use all variables in a complex expression */
    uint64_t t1 = a * b + c;
    uint64_t t2 = d ^ e ^ f;
    uint64_t t3 = g & h & i;
    uint64_t t4 = j | k | l;
    
    uint64_t t5 = (t1 + t2) * (t3 + t4);
    uint64_t t6 = (t1 - t2) / (t3 | 1);
    uint64_t t7 = (t4 << 3) ^ (t5 >> 2);
    
    return t5 + t6 + t7 + a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Main test function */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize many variables with different values */
    uint64_t vars[16];
    for (int i = 0; i < 16; i++) {
        vars[i] = (uint64_t)(argc + i * 123456789);
    }
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Target-specific multi-operand operations */
#ifdef __x86_64__
        result += x86_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7], vars[8]);
        result += x86_mult_highpart(vars[9], vars[10]);
#elif defined(__aarch64__)
        result += arm_multi_operand(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7], vars[8]);
#elif defined(__powerpc64__) || defined(__PPC64__)
        result += ppc_multi_operand(vars[0], vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7], vars[8]);
#else
        result += generic_multi_operand(vars[0], vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7], vars[8]);
#endif
    } else {
        /* Path 2: Vector operations and register pressure */
        v4si vec1 = {vars[0], vars[1], vars[2], vars[3]};
        v4si vec2 = {vars[4], vars[5], vars[6], vars[7]};
        v4si vec3 = {vars[8], vars[9], vars[10], vars[11]};
        v4si vec4 = {vars[12], vars[13], vars[14], vars[15]};
        
        v4si vec_result = vector_multi_op(vec1, vec2, vec3, vec4);
        result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    }
    
    /* Path 3: Always test register pressure function */
    result += create_register_pressure(vars[0], vars[1], vars[2], vars[3],
                                      vars[4], vars[5], vars[6], vars[7],
                                      vars[8], vars[9], vars[10], vars[11]);
    
    /* Loop with varying inputs to increase coverage */
    for (int i = 0; i < (argc % 10); i++) {
        vars[i % 16] += i;
        
#ifdef __x86_64__
        result ^= x86_multi_operand_asm(vars[0] + i, vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7], vars[8]);
#elif defined(__aarch64__)
        result ^= arm_multi_operand(vars[0] + i, vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7], vars[8]);
#elif defined(__powerpc64__) || defined(__PPC64__)
        result ^= ppc_multi_operand(vars[0] + i, vars[1], vars[2], vars[3],
                                   vars[4], vars[5], vars[6], vars[7], vars[8]);
#else
        result ^= generic_multi_operand(vars[0] + i, vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7], vars[8]);
#endif
    }
    
    printf("Result: %lu\n", (unsigned long)result);
    return (int)(result % 256);
}
