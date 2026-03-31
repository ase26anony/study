/* test_optabs.c - Test program for 10/11 operand RTL expansion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimizations from eliminating our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE long long test_multi_operand_arithmetic(long long a, long long b, 
                                                 long long c, long long d,
                                                 long long e, long long f,
                                                 long long g, long long h,
                                                 long long i, long long j) {
    /* Complex expression that might require many temporary registers */
    long long t1 = (a * b) >> 32;
    long long t2 = (c * d) >> 32;
    long long t3 = (e * f) >> 32;
    long long t4 = (g * h) >> 32;
    long long t5 = (i * j) >> 32;
    
    /* Multi-step computation that might expand to many operands */
    long long result = t1 + t2 - t3 * t4 / t5;
    result = result ^ (t1 << 3) ^ (t2 << 2) ^ (t3 << 1);
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si test_vector_ops(v4si a, v4si b, v4si c, v4si d,
                              v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = g * h >> 1;
    v4si t4 = (a + b) & (c + d);
    v4si t5 = (e + f) | (g + h);
    
    v4si result = t1 * t2 + t3 / t4 - t5;
    result = result ^ t1 ^ t2 ^ t3 ^ t4 ^ t5;
    
    return result;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE __m256i test_avx2_multi_operand(__m256i a, __m256i b, __m256i c,
                                         __m256i d, __m256i e, __m256i f,
                                         __m256i g, __m256i h, __m256i i) {
    /* Complex AVX2 operations that might require many operands */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_sub_epi32(c, d);
    __m256i t3 = _mm256_mullo_epi32(e, f);
    __m256i t4 = _mm256_slli_epi32(g, 3);
    __m256i t5 = _mm256_srli_epi32(h, 2);
    __m256i t6 = _mm256_and_si256(i, t1);
    
    __m256i result = _mm256_add_epi32(t1, t2);
    result = _mm256_sub_epi32(result, t3);
    result = _mm256_add_epi32(result, t4);
    result = _mm256_sub_epi32(result, t5);
    result = _mm256_xor_si256(result, t6);
    
    return result;
}

/* Extended inline assembly with many operands */
NOINLINE unsigned long long test_x86_extended_asm(
    unsigned long long a, unsigned long long b,
    unsigned long long c, unsigned long long d,
    unsigned long long e, unsigned long long f,
    unsigned long long g, unsigned long long h,
    unsigned long long i, unsigned long long j) {
    
    unsigned long long result1, result2, result3;
    
    /* Complex inline assembly that might expand to many operands */
    asm volatile (
        /* Multiple operations chained together */
        "mov %[a], %%rax\n\t"
        "imul %[b], %%rax\n\t"
        "add %[c], %%rax\n\t"
        "sub %[d], %%rax\n\t"
        "mov %%rax, %[r1]\n\t"
        "mov %[e], %%rbx\n\t"
        "xor %[f], %%rbx\n\t"
        "or %[g], %%rbx\n\t"
        "and %[h], %%rbx\n\t"
        "mov %%rbx, %[r2]\n\t"
        "mov %[i], %%rcx\n\t"
        "shl $3, %%rcx\n\t"
        "mov %[j], %%rdx\n\t"
        "shr $2, %%rdx\n\t"
        "add %%rcx, %%rdx\n\t"
        "mov %%rdx, %[r3]\n\t"
        : [r1] "=r" (result1), [r2] "=r" (result2), [r3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOINLINE uint64x2_t test_neon_multi_operand(uint64x2_t a, uint64x2_t b,
                                            uint64x2_t c, uint64x2_t d,
                                            uint64x2_t e, uint64x2_t f,
                                            uint64x2_t g, uint64x2_t h) {
    /* Complex NEON operations */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = veorq_u64(e, f);
    uint64x2_t t4 = vorrq_u64(g, h);
    uint64x2_t t5 = vandq_u64(t1, t2);
    
    uint64x2_t result = vaddq_u64(t1, t2);
    result = vaddq_u64(result, t3);
    result = vaddq_u64(result, t4);
    result = vaddq_u64(result, t5);
    
    return result;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
#include <altivec.h>

NOINLINE vector unsigned long long test_altivec_multi_operand(
    vector unsigned long long a, vector unsigned long long b,
    vector unsigned long long c, vector unsigned long long d,
    vector unsigned long long e, vector unsigned long long f,
    vector unsigned long long g, vector unsigned long long h) {
    
    vector unsigned long long t1 = vec_add(a, b);
    vector unsigned long long t2 = vec_sub(c, d);
    vector unsigned long long t3 = vec_xor(e, f);
    vector unsigned long long t4 = vec_or(g, h);
    vector unsigned long long t5 = vec_and(t1, t2);
    
    vector unsigned long long result = vec_add(t1, t2);
    result = vec_add(result, t3);
    result = vec_add(result, t4);
    result = vec_add(result, t5);
    
    return result;
}
#endif

/* Multi-precision arithmetic that might expand to many operands */
NOINLINE unsigned long long test_multi_precision(
    unsigned long long a1, unsigned long long a0,
    unsigned long long b1, unsigned long long b0) {
    
    /* 128-bit multiplication using 64-bit parts */
    unsigned long long t1 = a0 * b0;
    unsigned long long t2 = a0 * b1;
    unsigned long long t3 = a1 * b0;
    unsigned long long t4 = a1 * b1;
    
    /* Complex carry propagation */
    unsigned long long carry = (t1 >> 32) + (t2 & 0xFFFFFFFF) + (t3 & 0xFFFFFFFF);
    unsigned long long result_low = (t1 & 0xFFFFFFFF) | ((carry & 0xFFFFFFFF) << 32);
    unsigned long long result_high = t4 + (t2 >> 32) + (t3 >> 32) + (carry >> 32);
    
    return result_low + result_high;
}

/* Function that tries multiple approaches */
NOINLINE unsigned long long test_combined_approach(int argc, char **argv) {
    unsigned long long result = 0;
    
    /* Initialize many variables to use as operands */
    long long vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (long long)(argc + i) * 123456789;
    }
    
    /* Test 1: Complex arithmetic expression */
    if (argc > 1) {
        result += test_multi_operand_arithmetic(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
    }
    
    /* Test 2: Multi-precision arithmetic */
    if (argc > 2) {
        result += test_multi_precision(vars[10], vars[11], vars[12], vars[13]);
    }
    
#ifdef __x86_64__
    /* Test 3: x86-specific extended inline assembly */
    if (argc > 3) {
        result += test_x86_extended_asm(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
    }
    
    /* Test 4: AVX2 operations */
    if (argc > 4) {
        __m256i avx_vars[9];
        for (int i = 0; i < 9; i++) {
            avx_vars[i] = _mm256_set_epi32(
                vars[i*4+3], vars[i*4+2], vars[i*4+1], vars[i*4+0],
                vars[i*4+7], vars[i*4+6], vars[i*4+5], vars[i*4+4]);
        }
        
        __m256i avx_result = test_avx2_multi_operand(
            avx_vars[0], avx_vars[1], avx_vars[2], avx_vars[3],
            avx_vars[4], avx_vars[5], avx_vars[6], avx_vars[7], avx_vars[8]);
        
        /* Extract and add to result */
        int avx_vals[8];
        _mm256_storeu_si256((__m256i*)avx_vals, avx_result);
        for (int i = 0; i < 8; i++) {
            result += avx_vals[i];
        }
    }
#endif
    
    /* Test 5: Vector operations */
    if (argc > 5) {
        v4si vec_a = {vars[0], vars[1], vars[2], vars[3]};
        v4si vec_b = {vars[4], vars[5], vars[6], vars[7]};
        v4si vec_c = {vars[8], vars[9], vars[10], vars[11]};
        v4si vec_d = {vars[12], vars[13], vars[14], vars[15]};
        v4si vec_e = {vars[16], vars[17], vars[18], vars[19]};
        v4si vec_f = {vars[0], vars[1], vars[2], vars[3]};
        v4si vec_g = {vars[4], vars[5], vars[6], vars[7]};
        v4si vec_h = {vars[8], vars[9], vars[10], vars[11]};
        
        v4si vec_result = test_vector_ops(
            vec_a, vec_b, vec_c, vec_d, vec_e, vec_f, vec_g, vec_h);
        
        for (int i = 0; i < 4; i++) {
            result += vec_result[i];
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    unsigned long long total_result = 0;
    
    /* Run multiple iterations with slightly different data */
    for (int iter = 0; iter < (argc > 0 ? argc : 3); iter++) {
        /* Modify argc for different code paths in each iteration */
        int modified_argc = argc + iter;
        
        /* Call the combined test function */
        unsigned long long iter_result = test_combined_approach(modified_argc, argv);
        
        /* Use the result to prevent dead code elimination */
        total_result ^= iter_result;
        
        /* Print progress to ensure code executes */
        if (iter % 100 == 0) {
            printf("Iteration %d: result = %llu\n", iter, iter_result);
        }
    }
    
    printf("Final result: %llu\n", total_result);
    return (int)(total_result & 0x7FFFFFFF);
}
