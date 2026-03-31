/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization from removing our test code */
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
    /* Complex expression that might require many temporary operands */
    long long t1 = (a * b) >> 32;
    long long t2 = (c * d) >> 32;
    long long t3 = (e * f) >> 32;
    long long t4 = (g * h) >> 32;
    long long t5 = (i * j) >> 32;
    
    /* Mix them in a way that prevents optimization */
    return t1 + t2 - t3 * t4 / (t5 + 1);
}

/* Vector operations that might expand to many operands */
NOINLINE v4si test_vector_ops(v4si a, v4si b, v4si c, v4si d,
                              v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = g * h;
    v4si t4 = (t1 & t2) | t3;
    v4si t5 = t1 ^ t2 ^ t3;
    
    return t4 + t5;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE __m256i test_avx2_multi_operand(__m256i a, __m256i b, __m256i c,
                                         __m256i d, __m256i e, __m256i f,
                                         __m256i g, __m256i h, __m256i i) {
    /* Complex AVX2 expression that might require many operands */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_sub_epi32(c, d);
    __m256i t3 = _mm256_mullo_epi32(e, f);
    __m256i t4 = _mm256_and_si256(g, h);
    __m256i t5 = _mm256_or_si256(t1, t2);
    __m256i t6 = _mm256_xor_si256(t3, t4);
    
    return _mm256_add_epi32(t5, t6);
}

/* Extended inline assembly with many operands */
NOINLINE long long test_x86_asm_10ops(long long a, long long b, long long c,
                                      long long d, long long e, long long f,
                                      long long g, long long h, long long i,
                                      long long j) {
    long long result1, result2, result3;
    
    /* Complex inline assembly that might expand to many operands */
    asm volatile (
        "imulq %[b], %[a]\n\t"
        "imulq %[d], %[c]\n\t"
        "imulq %[f], %[e]\n\t"
        "imulq %[h], %[g]\n\t"
        "imulq %[j], %[i]\n\t"
        "addq %[c], %[a]\n\t"
        "addq %[e], %[a]\n\t"
        "addq %[g], %[a]\n\t"
        "addq %[i], %[a]\n\t"
        "movq %[a], %[out1]\n\t"
        "lea (%[b],%[c],2), %[out2]\n\t"
        "lea (%[d],%[e],4), %[out3]"
        : [out1] "=r" (result1), [out2] "=r" (result2), [out3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

NOINLINE int64x2_t test_neon_multi_operand(int64x2_t a, int64x2_t b, int64x2_t c,
                                           int64x2_t d, int64x2_t e, int64x2_t f,
                                           int64x2_t g, int64x2_t h, int64x2_t i) {
    /* Complex NEON expression */
    int64x2_t t1 = vaddq_s64(a, b);
    int64x2_t t2 = vsubq_s64(c, d);
    int64x2_t t3 = vmulq_s64(e, f);
    int64x2_t t4 = vandq_s64(g, h);
    int64x2_t t5 = vorrq_s64(t1, t2);
    int64x2_t t6 = veorq_s64(t3, t4);
    
    return vaddq_s64(t5, t6);
}

/* ARM inline assembly with many operands */
NOINLINE long long test_arm_asm_10ops(long long a, long long b, long long c,
                                      long long d, long long e, long long f,
                                      long long g, long long h, long long i,
                                      long long j) {
    long long result1, result2, result3;
    
    asm volatile (
        "mul %[a], %[a], %[b]\n\t"
        "mul %[c], %[c], %[d]\n\t"
        "mul %[e], %[e], %[f]\n\t"
        "mul %[g], %[g], %[h]\n\t"
        "mul %[i], %[i], %[j]\n\t"
        "add %[a], %[a], %[c]\n\t"
        "add %[a], %[a], %[e]\n\t"
        "add %[a], %[a], %[g]\n\t"
        "add %[a], %[a], %[i]\n\t"
        "mov %[out1], %[a]\n\t"
        "madd %[out2], %[b], %[c], %[d]\n\t"
        "madd %[out3], %[e], %[f], %[g]"
        : [out1] "=r" (result1), [out2] "=r" (result2), [out3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

#elif defined(__powerpc__) || defined(__PPC__)
/* PowerPC specific multi-operand operations */
NOINLINE long long test_ppc_multi_operand(long long a, long long b, long long c,
                                          long long d, long long e, long long f,
                                          long long g, long long h, long long i,
                                          long long j) {
    long long result;
    
    /* PowerPC inline assembly with many operands */
    asm volatile (
        "mulld %[res], %[a], %[b]\n\t"
        "mulld %[c], %[c], %[d]\n\t"
        "mulld %[e], %[e], %[f]\n\t"
        "mulld %[g], %[g], %[h]\n\t"
        "mulld %[i], %[i], %[j]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[i]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}
#endif

/* Test function that combines multiple approaches */
NOINLINE long long test_combined_multi_operand(int argc, char **argv) {
    long long result = 0;
    
    /* Initialize many variables to use as operands */
    long long vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (long long)(argc + i) * (i + 1);
    }
    
    /* Test different code paths based on argc */
    if (argc > 10) {
        /* Path 1: Complex arithmetic with many operands */
        result = test_multi_operand_arithmetic(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
    } else if (argc > 5) {
        /* Path 2: Target-specific inline assembly */
#ifdef __x86_64__
        result = test_x86_asm_10ops(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
#elif defined(__aarch64__)
        result = test_arm_asm_10ops(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
#elif defined(__powerpc__) || defined(__PPC__)
        result = test_ppc_multi_operand(
            vars[0], vars[1], vars[2], vars[3], vars[4],
            vars[5], vars[6], vars[7], vars[8], vars[9]);
#endif
    } else {
        /* Path 3: Vector operations */
        v4si vec_vars[8];
        for (int i = 0; i < 8; i++) {
            vec_vars[i] = (v4si){vars[i], vars[i+1], vars[i+2], vars[i+3]};
        }
        
        v4si vec_result = test_vector_ops(
            vec_vars[0], vec_vars[1], vec_vars[2], vec_vars[3],
            vec_vars[4], vec_vars[5], vec_vars[6], vec_vars[7]);
        
        result = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    }
    
    /* Mix in more operations to prevent dead code elimination */
    for (int i = 0; i < argc % 10; i++) {
        result += vars[i] * (i + 1);
    }
    
    return result;
}

int main(int argc, char **argv) {
    long long final_result = 0;
    
    /* Call the test function multiple times with different parameters */
    for (int iteration = 0; iteration < (argc > 1 ? atoi(argv[1]) % 10 : 3); iteration++) {
        /* Modify argc to trigger different code paths */
        int modified_argc = argc + iteration;
        
        /* Call the combined test function */
        long long iter_result = test_combined_multi_operand(modified_argc, argv);
        
        /* Use the result to prevent optimization */
        final_result += iter_result * (iteration + 1);
        
        /* Additional complex computation that might expand to many operands */
        if (iteration % 2 == 0) {
            /* Multi-precision arithmetic that might require many temporary registers */
            long long a = iter_result;
            long long b = final_result;
            long long c = a * b;
            long long d = (c >> 32) | (c << 32);
            long long e = d * a / (b + 1);
            long long f = e ^ d ^ c;
            long long g = (f * 0x5DEECE66DLL + 0xBLL) & 0xFFFFFFFFFFFFLL;
            
            final_result = g;
        }
    }
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %lld\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
