#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

/* Simple PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t fast_rand(void) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

/* Initialize arrays with pseudo-random data */
static void init_data(__m256i* arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = _mm256_set_epi32(
            fast_rand(), fast_rand(), fast_rand(), fast_rand(),
            fast_rand(), fast_rand(), fast_rand(), fast_rand()
        );
    }
}

/* Complex expression with many temporaries - forces expander to handle many operands */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(__m256i* restrict src1, __m256i* restrict src2,
                          __m256i* restrict src3, __m256i* restrict src4,
                          __m256i* restrict dst, size_t n) {
    
    /* Volatile counter to prevent loop unrolling/simplification */
    volatile size_t volatile_counter = 0;
    
    for (size_t i = 0; i < n; i += 4) {
        /* Prevent CSE and constant propagation */
        asm volatile("" : "+r"(i) : : "memory");
        
        /* Load multiple vectors - creates many SSA values */
        __m256i v0 = src1[i];
        __m256i v1 = src1[i + 1];
        __m256i v2 = src2[i];
        __m256i v3 = src2[i + 1];
        __m256i v4 = src3[i];
        __m256i v5 = src3[i + 1];
        __m256i v6 = src4[i];
        __m256i v7 = src4[i + 1];
        
        /* Complex multi-statement expression with many temporaries */
        __m256i t0 = _mm256_add_epi32(v0, v1);
        __m256i t1 = _mm256_sub_epi32(v2, v3);
        __m256i t2 = _mm256_mullo_epi32(v4, v5);
        __m256i t3 = _mm256_and_si256(v6, v7);
        __m256i t4 = _mm256_or_si256(t0, t1);
        __m256i t5 = _mm256_xor_si256(t2, t3);
        
        /* Create many immediate constants for shuffle operations */
        int imm0 = (i & 7) << 2;
        int imm1 = ((i + 1) & 7) << 2;
        int imm2 = ((i + 2) & 7) << 2;
        int imm3 = ((i + 3) & 7) << 2;
        int imm4 = ((i + 4) & 7) << 2;
        int imm5 = ((i + 5) & 7) << 2;
        
        /* Use inline asm with 11 operands - triggers optab expansion */
        __m256i result;
        asm volatile (
            "vpblendd %[imm0], %[v0], %[v1], %[t0]\n\t"
            "vpblendd %[imm1], %[t0], %[v2], %[t1]\n\t"
            "vpblendd %[imm2], %[t1], %[v3], %[t2]\n\t"
            "vpblendd %[imm3], %[t2], %[v4], %[t3]\n\t"
            "vpblendd %[imm4], %[t3], %[v5], %[t4]\n\t"
            "vpblendd %[imm5], %[t4], %[v6], %[r]"
            : [r] "=x" (result)
            : [v0] "x" (v0), [v1] "x" (v1), [v2] "x" (v2),
              [v3] "x" (v3), [v4] "x" (v4), [v5] "x" (v5),
              [v6] "x" (v6), [imm0] "i" (imm0), [imm1] "i" (imm1),
              [imm2] "i" (imm2), [imm3] "i" (imm3), [imm4] "i" (imm4),
              [imm5] "i" (imm5)
            : "memory"
        );
        
        /* Another complex expression chain */
        __m256i final_result = _mm256_add_epi32(result, t5);
        final_result = _mm256_sub_epi32(final_result, t4);
        final_result = _mm256_mullo_epi32(final_result, t3);
        
        /* Store with memory constraint */
        asm volatile (
            "vmovdqa %[val], %[mem]"
            : [mem] "=m" (dst[i])
            : [val] "x" (final_result)
            : "memory"
        );
        
        volatile_counter++;
    }
}

/* Alternative using GCC vector builtins with many arguments */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
static void test_vector_shuffle(__m512i* restrict src, __m512i* restrict dst, size_t n) {
    for (size_t i = 0; i < n; i++) {
        /* Complex shuffle with many arguments - may trigger 10+ argument optab */
        __m512i v0 = src[i];
        __m512i v1 = src[(i + 1) % n];
        
        /* Create a shuffle mask with many immediate values */
        __m512i result = __builtin_shuffle(v0, v1, 
            (__v64qi){0, 63, 1, 62, 2, 61, 3, 60, 4, 59, 5, 58, 6, 57, 7, 56,
                      8, 55, 9, 54, 10, 53, 11, 52, 12, 51, 13, 50, 14, 49, 15, 48,
                      16, 47, 17, 46, 18, 45, 19, 44, 20, 43, 21, 42, 22, 41, 23, 40,
                      24, 39, 25, 38, 26, 37, 27, 36, 28, 35, 29, 34, 30, 33, 31, 32});
        
        dst[i] = result;
    }
}
#endif

/* Function with many scalar arguments - forces expander to handle many operands */
__attribute__((noinline))
static int64_t complex_scalar_expr(
    int64_t a, int64_t b, int64_t c, int64_t d,
    int64_t e, int64_t f, int64_t g, int64_t h,
    int64_t i, int64_t j, int64_t k) {
    
    /* Complex expression tree with many operations */
    int64_t t0 = (a * b) + (c ^ d);
    int64_t t1 = (e << 3) | (f >> 2);
    int64_t t2 = (g & h) * (i | j);
    int64_t t3 = (t0 - t1) ^ (t2 + k);
    int64_t t4 = (a ^ c) & (e ^ g);
    int64_t t5 = (b | d) + (f & h);
    int64_t t6 = (i * j) - (k << 1);
    
    return ((t3 * t4) + (t5 ^ t6)) & 0xFFFFFFFF;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_SIZE = ARRAY_SIZE / 8; /* 8 ints per __m256i */
    
    /* Allocate aligned memory for vector arrays */
    __m256i* src1 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* src2 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* src3 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* src4 = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    __m256i* dst = aligned_alloc(32, VEC_SIZE * sizeof(__m256i));
    
    if (!src1 || !src2 || !src3 || !src4 || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_data(src1, VEC_SIZE);
    init_data(src2, VEC_SIZE);
    init_data(src3, VEC_SIZE);
    init_data(src4, VEC_SIZE);
    memset(dst, 0, VEC_SIZE * sizeof(__m256i));
    
    /* Test the many-argument vector function */
    test_many_args(src1, src2, src3, src4, dst, VEC_SIZE);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    int32_t* dst_int = (int32_t*)dst;
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst_int[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Test scalar many-argument function */
    int64_t scalar_result = complex_scalar_expr(
        fast_rand(), fast_rand(), fast_rand(), fast_rand(),
        fast_rand(), fast_rand(), fast_rand(), fast_rand(),
        fast_rand(), fast_rand(), fast_rand());
    printf("Scalar result: %ld\n", scalar_result);
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(src3);
    free(src4);
    free(dst);
    
    return 0;
}
