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
NOINLINE
long long test_multi_operand_arithmetic(long long a, long long b, 
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
NOINLINE
v4si test_vector_ops(v4si a, v4si b, v4si c, v4si d,
                     v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = e * f;
    v4si t4 = g / (h + (v4si){1,1,1,1});
    
    return (t1 & t2) | (t3 ^ t4);
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE
__m256i test_avx2_multi_operand(__m256i a, __m256i b, __m256i c,
                                __m256i d, __m256i e, __m256i f,
                                __m256i g, __m256i h, __m256i i) {
    /* Complex AVX2 operation chain */
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_sub_epi32(c, d);
    __m256i t3 = _mm256_mullo_epi32(e, f);
    __m256i t4 = _mm256_slli_epi32(g, 4);
    __m256i t5 = _mm256_srli_epi32(h, 2);
    
    /* Blend with mask - might generate many operands */
    return _mm256_blendv_epi8(t1, t2, 
           _mm256_blendv_epi8(t3, t4,
           _mm256_blendv_epi8(t5, i, a)));
}

/* Inline assembly with many operands */
NOINLINE
long long test_x86_asm_10ops(long long a, long long b, long long c,
                             long long d, long long e, long long f,
                             long long g, long long h, long long i) {
    long long result1, result2;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        "imulq %[b], %[a]\n\t"
        "imulq %[d], %[c]\n\t"
        "imulq %[f], %[e]\n\t"
        "imulq %[h], %[g]\n\t"
        "addq %[c], %[a]\n\t"
        "addq %[g], %[e]\n\t"
        "subq %[e], %[a]\n\t"
        "movq %[a], %[r1]\n\t"
        "movq %[i], %[r2]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    return result1 + result2;
}

#endif

#ifdef __aarch64__
#include <arm_neon.h>

NOINLINE
int32x4_t test_neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                  int32x4_t d, int32x4_t e, int32x4_t f,
                                  int32x4_t g, int32x4_t h) {
    /* Complex NEON operation chain */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vsubq_s32(c, d);
    int32x4_t t3 = vmulq_s32(e, f);
    int32x4_t t4 = vshlq_s32(g, vdupq_n_s32(2));
    
    /* Multiple lane operations */
    int32x4_t t5 = vmlaq_s32(t1, t2, t3);
    return vaddq_s32(t4, t5);
}

#endif

#ifdef __powerpc64__
#include <altivec.h>

NOINLINE
vector signed int test_altivec_multi_operand(vector signed int a,
                                             vector signed int b,
                                             vector signed int c,
                                             vector signed int d,
                                             vector signed int e,
                                             vector signed int f,
                                             vector signed int g,
                                             vector signed int h) {
    /* AltiVec operations that might expand to many operands */
    vector signed int t1 = vec_add(a, b);
    vector signed int t2 = vec_sub(c, d);
    vector signed int t3 = vec_mul(e, f);
    vector signed int t4 = vec_sl(g, (vector unsigned int){2,2,2,2});
    
    /* Complex permutation */
    vector signed int t5 = vec_perm(t1, t2, (vector unsigned char){
        0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15});
    
    return vec_add(t3, vec_add(t4, t5));
}

#endif

/* Test function that tries multiple approaches */
NOINLINE
long long test_mixed_operations(int argc, char **argv) {
    long long result = 0;
    
    /* Initialize many variables to use as operands */
    long long vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (long long)argc + i * 3;
    }
    
    /* Try different code paths based on argc */
    if (argc > 10) {
        /* Path 1: Complex arithmetic with many operands */
        result = test_multi_operand_arithmetic(
            vars[0], vars[1], vars[2], vars[3],
            vars[4], vars[5], vars[6], vars[7],
            vars[8], vars[9]);
    } else if (argc > 5) {
        /* Path 2: Vector operations */
        v4si vec_result = test_vector_ops(
            (v4si){vars[0], vars[1], vars[2], vars[3]},
            (v4si){vars[4], vars[5], vars[6], vars[7]},
            (v4si){vars[8], vars[9], vars[10], vars[11]},
            (v4si){vars[12], vars[13], vars[14], vars[15]},
            (v4si){vars[0], vars[1], vars[2], vars[3]},
            (v4si){vars[4], vars[5], vars[6], vars[7]},
            (v4si){vars[8], vars[9], vars[10], vars[11]},
            (v4si){vars[12], vars[13], vars[14], vars[15]});
        
        /* Extract result */
        int *ptr = (int*)&vec_result;
        result = ptr[0] + ptr[1] + ptr[2] + ptr[3];
    }
#ifdef __x86_64__
    else if (argc > 3) {
        /* Path 3: AVX2 operations */
        __m256i avx_result = test_avx2_multi_operand(
            _mm256_set_epi32(vars[0], vars[1], vars[2], vars[3],
                            vars[4], vars[5], vars[6], vars[7]),
            _mm256_set_epi32(vars[8], vars[9], vars[10], vars[11],
                            vars[12], vars[13], vars[14], vars[15]),
            _mm256_set_epi32(vars[0], vars[1], vars[2], vars[3],
                            vars[4], vars[5], vars[6], vars[7]),
            _mm256_set_epi32(vars[8], vars[9], vars[10], vars[11],
                            vars[12], vars[13], vars[14], vars[15]),
            _mm256_set_epi32(vars[0], vars[1], vars[2], vars[3],
                            vars[4], vars[5], vars[6], vars[7]),
            _mm256_set_epi32(vars[8], vars[9], vars[10], vars[11],
                            vars[12], vars[13], vars[14], vars[15]),
            _mm256_set_epi32(vars[0], vars[1], vars[2], vars[3],
                            vars[4], vars[5], vars[6], vars[7]),
            _mm256_set_epi32(vars[8], vars[9], vars[10], vars[11],
                            vars[12], vars[13], vars[14], vars[15]),
            _mm256_set_epi32(vars[0], vars[1], vars[2], vars[3],
                            vars[4], vars[5], vars[6], vars[7]));
        
        /* Extract result */
        int avx_vals[8];
        _mm256_storeu_si256((__m256i*)avx_vals, avx_result);
        for (int i = 0; i < 8; i++) result += avx_vals[i];
    } else if (argc > 1) {
        /* Path 4: Inline assembly with many operands */
        result = test_x86_asm_10ops(
            vars[0], vars[1], vars[2], vars[3],
            vars[4], vars[5], vars[6], vars[7],
            vars[8]);
    }
#endif
    
    /* Mix in some additional operations to prevent dead code elimination */
    for (int i = 0; i < argc % 10; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

int main(int argc, char **argv) {
    long long total = 0;
    
    /* Call test function multiple times with different parameters */
    for (int i = 0; i < (argc % 5) + 1; i++) {
        total += test_mixed_operations(argc + i, argv);
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", total);
    
    return (int)(total % 256);
}
