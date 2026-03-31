/* Test program to trigger 10/11-operand RTL expansion in GCC's optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization from eliminating our test cases */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansion */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

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
    
    /* Multi-precision arithmetic */
    uint64_t result = t1 + t2;
    result = (result * t3) >> 32;
    result += t4;
    result = (result * t5) >> 32;
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_multi_op(v4si a, v4si b, v4si c, v4si d,
                              v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e + f;
    v4si t3 = t1 * t2 + g;
    v4si t4 = t3 >> 2;
    v4si t5 = t4 * a + b;
    
    return t5;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE __m256i x86_multi_operand_intrinsic(__m256i a, __m256i b, __m256i c,
                                             __m256i d, __m256i e, __m256i f) {
    /* Complex AVX2/AVX-512 operation chain */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_mullo_epi32(c, d);
    __m256i t3 = _mm256_slli_epi32(e, 3);
    __m256i t4 = _mm256_add_epi32(t1, t2);
    __m256i t5 = _mm256_sub_epi32(t4, t3);
    __m256i t6 = _mm256_and_si256(t5, f);
    
    return _mm256_add_epi32(t6, a);
}

/* Extended inline assembly with many operands */
NOINLINE uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Complex inline assembly that might expand to many operands */
    asm volatile (
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
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rdx", "cc"
    );
    
    result3 = result1 + result2;
    return result3;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOINLINE uint64x2_t arm_multi_operand_intrinsic(uint64x2_t a, uint64x2_t b,
                                                uint64x2_t c, uint64x2_t d,
                                                uint64x2_t e, uint64x2_t f) {
    /* Complex NEON operation chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vmulq_u64(c, d);
    uint64x2_t t3 = vshlq_u64(e, vdupq_n_u64(3));
    uint64x2_t t4 = vaddq_u64(t1, t2);
    uint64x2_t t5 = vsubq_u64(t4, t3);
    uint64x2_t t6 = vandq_u64(t5, f);
    
    return vaddq_u64(t6, a);
}

/* ARM-specific multi-operand inline assembly */
NOINLINE uint64_t arm_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result;
    
    asm volatile (
        "add %[a], %[b], %[tmp1]\n\t"
        "add %[c], %[d], %[tmp2]\n\t"
        "mul %[tmp1], %[tmp2], %[tmp3]\n\t"
        "add %[e], %[f], %[tmp4]\n\t"
        "add %[g], %[h], %[tmp5]\n\t"
        "mul %[tmp4], %[tmp5], %[tmp6]\n\t"
        "add %[tmp3], %[tmp6], %[tmp7]\n\t"
        "add %[i], %[j], %[tmp8]\n\t"
        "mul %[tmp7], %[tmp8], %[result]\n\t"
        : [result] "=r" (result),
          [tmp1] "=&r" (a), [tmp2] "=&r" (b), [tmp3] "=&r" (c),
          [tmp4] "=&r" (d), [tmp5] "=&r" (e), [tmp6] "=&r" (f),
          [tmp7] "=&r" (g), [tmp8] "=&r" (h)
        : [a] "1" (a), [b] "2" (b), [c] "3" (c), [d] "4" (d),
          [e] "5" (e), [f] "6" (f), [g] "7" (g), [h] "8" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
/* PowerPC-specific implementations */
NOINLINE uint64_t ppc_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                        uint64_t d, uint64_t e, uint64_t f,
                                        uint64_t g, uint64_t h, uint64_t i,
                                        uint64_t j) {
    uint64_t result;
    
    asm volatile (
        "mulld %[tmp1], %[a], %[b]\n\t"
        "mulld %[tmp2], %[c], %[d]\n\t"
        "add %[tmp3], %[tmp1], %[tmp2]\n\t"
        "mulld %[tmp4], %[e], %[f]\n\t"
        "mulld %[tmp5], %[g], %[h]\n\t"
        "add %[tmp6], %[tmp4], %[tmp5]\n\t"
        "mulld %[tmp7], %[i], %[j]\n\t"
        "add %[tmp8], %[tmp3], %[tmp6]\n\t"
        "add %[result], %[tmp8], %[tmp7]\n\t"
        : [result] "=r" (result),
          [tmp1] "=&r" (a), [tmp2] "=&r" (b), [tmp3] "=&r" (c),
          [tmp4] "=&r" (d), [tmp5] "=&r" (e), [tmp6] "=&r" (f),
          [tmp7] "=&r" (g), [tmp8] "=&r" (h)
        : [a] "1" (a), [b] "2" (b), [c] "3" (c), [d] "4" (d),
          [e] "5" (e), [f] "6" (f), [g] "7" (g), [h] "8" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Function that combines multiple expansion strategies */
NOINLINE uint64_t combined_multi_operand_test(int argc, char **argv) {
    uint64_t result = 0;
    
    /* Initialize many variables to use as operands */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (uint64_t)(i + argc);
    }
    
    /* Test 1: Complex arithmetic expression */
    result += multi_operand_arith(vars[0], vars[1], vars[2], vars[3],
                                  vars[4], vars[5], vars[6], vars[7],
                                  vars[8], vars[9]);
    
    /* Test 2: Vector operations */
    v4si vec_a = {vars[0], vars[1], vars[2], vars[3]};
    v4si vec_b = {vars[4], vars[5], vars[6], vars[7]};
    v4si vec_c = {vars[8], vars[9], vars[10], vars[11]};
    v4si vec_d = {vars[12], vars[13], vars[14], vars[15]};
    v4si vec_e = {vars[16], vars[17], vars[18], vars[19]};
    v4si vec_f = {1, 2, 3, 4};
    v4si vec_g = {5, 6, 7, 8};
    
    v4si vec_result = vector_multi_op(vec_a, vec_b, vec_c, vec_d,
                                      vec_e, vec_f, vec_g);
    result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    if (argc > 1) {
        /* Test with AVX intrinsics */
        __m256i avx_a = _mm256_set_epi64x(vars[0], vars[1], vars[2], vars[3]);
        __m256i avx_b = _mm256_set_epi64x(vars[4], vars[5], vars[6], vars[7]);
        __m256i avx_c = _mm256_set_epi64x(vars[8], vars[9], vars[10], vars[11]);
        __m256i avx_d = _mm256_set_epi64x(vars[12], vars[13], vars[14], vars[15]);
        __m256i avx_e = _mm256_set_epi64x(vars[16], vars[17], vars[18], vars[19]);
        __m256i avx_f = _mm256_set_epi64x(1, 2, 3, 4);
        
        __m256i avx_result = x86_multi_operand_intrinsic(avx_a, avx_b, avx_c,
                                                         avx_d, avx_e, avx_f);
        uint64_t *avx_res = (uint64_t*)&avx_result;
        result += avx_res[0] + avx_res[1] + avx_res[2] + avx_res[3];
    } else {
        /* Test with extended inline assembly */
        result += x86_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                        vars[4], vars[5], vars[6], vars[7],
                                        vars[8], vars[9]);
    }
#elif defined(__aarch64__)
    if (argc > 1) {
        /* Test with NEON intrinsics */
        uint64x2_t neon_a = {vars[0], vars[1]};
        uint64x2_t neon_b = {vars[2], vars[3]};
        uint64x2_t neon_c = {vars[4], vars[5]};
        uint64x2_t neon_d = {vars[6], vars[7]};
        uint64x2_t neon_e = {vars[8], vars[9]};
        uint64x2_t neon_f = {vars[10], vars[11]};
        
        uint64x2_t neon_result = arm_multi_operand_intrinsic(neon_a, neon_b,
                                                             neon_c, neon_d,
                                                             neon_e, neon_f);
        result += vgetq_lane_u64(neon_result, 0) + 
                  vgetq_lane_u64(neon_result, 1);
    } else {
        /* Test with ARM inline assembly */
        result += arm_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                        vars[4], vars[5], vars[6], vars[7],
                                        vars[8], vars[9]);
    }
#elif defined(__powerpc64__) || defined(__PPC64__)
    /* PowerPC inline assembly test */
    result += ppc_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                    vars[4], vars[5], vars[6], vars[7],
                                    vars[8], vars[9]);
#endif
    
    return result;
}

int main(int argc, char **argv) {
    uint64_t total_result = 0;
    
    /* Run multiple iterations with different inputs */
    for (int iter = 0; iter < (argc > 2 ? atoi(argv[2]) : 3); iter++) {
        /* Modify argc slightly each iteration to vary code paths */
        int modified_argc = argc + iter;
        
        /* Call the combined test function */
        uint64_t iter_result = combined_multi_operand_test(modified_argc, argv);
        
        /* Use the result to prevent dead code elimination */
        total_result ^= iter_result;
        
        /* Print progress to ensure side effects */
        if (argc > 3) {
            printf("Iteration %d: result = %lu\n", iter, iter_result);
        }
    }
    
    /* Final output to prevent optimization */
    printf("Final result: %lu\n", total_result);
    
    return (int)(total_result & 0x7FFFFFFF);
}
