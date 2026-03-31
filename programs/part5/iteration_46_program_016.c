/* Compile with: gcc -O3 -mavx2 -ftree-vectorize -fdump-rtl-expand -fdump-rtl-combine */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

/* Simple PRNG for deterministic testing */
static inline uint32_t lcg_parkmiller(uint32_t *state) {
    return *state = (uint64_t)*state * 48271 % 0x7fffffff;
}

/* Volatile counter to prevent loop unrolling */
static volatile int volatile_counter = 0;

/* Function with many arguments that can't be easily optimized */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(float *restrict out, 
                          const float *restrict in1,
                          const float *restrict in2,
                          const float *restrict in3,
                          const float *restrict in4,
                          int n) {
    /* Force dependency chain with volatile */
    int vc = volatile_counter;
    
    for (int i = 0; i < n; i += 8) {
        /* Load multiple vectors - creates many SSA values */
        __m256 v1 = _mm256_loadu_ps(&in1[i]);
        __m256 v2 = _mm256_loadu_ps(&in2[i]);
        __m256 v3 = _mm256_loadu_ps(&in3[i]);
        __m256 v4 = _mm256_loadu_ps(&in4[i]);
        
        /* Complex expression with many temporaries */
        __m256 t1 = _mm256_add_ps(v1, v2);
        __m256 t2 = _mm256_sub_ps(v3, v4);
        __m256 t3 = _mm256_mul_ps(t1, t2);
        __m256 t4 = _mm256_fmadd_ps(v1, v2, v3);
        
        /* Create many intermediate values to force expander to use many ops */
        __m256 mask1 = _mm256_cmp_ps(t1, t2, _CMP_LT_OS);
        __m256 mask2 = _mm256_cmp_ps(t3, t4, _CMP_GT_OS);
        
        /* Complex blend with many arguments - may expand to optab with many args */
        __m256 blended = _mm256_blendv_ps(
            _mm256_blendv_ps(t1, t2, mask1),
            _mm256_blendv_ps(t3, t4, mask2),
            _mm256_xor_ps(mask1, mask2)
        );
        
        /* Shuffle operation that could use many arguments */
        __m256 shuffled = _mm256_permutevar8x32_ps(blended, 
            _mm256_set_epi32(vc, 7, 6, 5, 4, 3, 2, 1));
        
        /* Store result */
        _mm256_storeu_ps(&out[i], shuffled);
        
        /* Update volatile counter to prevent CSE */
        vc = (vc + 1) & 7;
    }
    
    volatile_counter = vc;
}

/* Function using inline asm with many operands */
__attribute__((noinline, target("avx2")))
static void asm_many_operands(float *out, float *in1, float *in2, 
                             float *in3, float *in4, float *in5,
                             int imm1, int imm2, int imm3) {
    /* Extended asm with 11 operands */
    asm volatile (
        "vmovups (%1), %%ymm0\n\t"
        "vmovups (%2), %%ymm1\n\t"
        "vmovups (%3), %%ymm2\n\t"
        "vmovups (%4), %%ymm3\n\t"
        "vmovups (%5), %%ymm4\n\t"
        "vaddps %%ymm0, %%ymm1, %%ymm5\n\t"
        "vaddps %%ymm2, %%ymm3, %%ymm6\n\t"
        "vmulps %%ymm5, %%ymm6, %%ymm7\n\t"
        "vpermilps %6, %%ymm7, %%ymm8\n\t"
        "vpermilps %7, %%ymm8, %%ymm9\n\t"
        "vpermilps %8, %%ymm9, %%ymm10\n\t"
        "vmovups %%ymm10, (%0)"
        : 
        : "r"(out), "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "i"(imm1), "i"(imm2), "i"(imm3)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", 
          "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "memory"
    );
}

/* Complex expression with many temporaries */
__attribute__((noinline))
static long complex_many_arg_expr(char *cptr, short *sptr, int *iptr, 
                                 long *lptr, float *fptr, double *dptr,
                                 int idx1, int idx2, int idx3, int idx4) {
    /* Force many intermediate values */
    long val1 = lptr[idx1] + (long)iptr[idx2];
    long val2 = (long)(fptr[idx3] * 100.0f) + (long)(dptr[idx4] * 1000.0);
    long val3 = (long)cptr[idx1] << (sptr[idx2] & 0xF);
    long val4 = (val1 ^ val2) | (val3 & ~val1);
    
    /* Complex bitwise expression with many operands */
    long result = ((val1 + val2) * (val3 - val4)) |
                  ((val1 & val2) ^ (val3 | val4)) |
                  ((val1 << 3) + (val2 >> 2)) |
                  ((val3 & 0xFF) << 24) |
                  ((val4 & 0xFF00) << 16);
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* Vector permutation with many arguments */
#ifdef __AVX512F__
__attribute__((noinline, target("avx512f")))
static __m512i avx512_many_arg_shuffle(__m512i a, __m512i b, __m512i c,
                                      __m512i d, __m512i mask1, __m512i mask2,
                                      int imm1, int imm2, int imm3, int imm4) {
    /* Complex sequence that might require many-argument expansion */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_sub_epi32(c, d);
    __m512i t3 = _mm512_mullo_epi32(t1, t2);
    
    /* Permutation with many control values */
    __m512i permuted = _mm512_permutex2var_epi32(t1, mask1, t2);
    permuted = _mm512_permutex2var_epi32(permuted, mask2, t3);
    
    /* Blend with mask - many arguments */
    __mmask16 blend_mask = _mm512_cmp_epi32_mask(t1, t2, _MM_CMPINT_LT);
    __m512i blended = _mm512_mask_blend_epi32(blend_mask, permuted, a);
    
    /* Another permutation with immediate */
    blended = _mm512_permutexvar_epi32(_mm512_set_epi32(
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0), blended);
    
    return blended;
}
#endif

int main(void) {
    const int N = 1024;
    float *in1 = aligned_alloc(32, N * sizeof(float));
    float *in2 = aligned_alloc(32, N * sizeof(float));
    float *in3 = aligned_alloc(32, N * sizeof(float));
    float *in4 = aligned_alloc(32, N * sizeof(float));
    float *out = aligned_alloc(32, N * sizeof(float));
    
    /* Mixed type arrays for complex expression */
    char *carr = aligned_alloc(16, N * sizeof(char));
    short *sarr = aligned_alloc(16, N * sizeof(short));
    int *iarr = aligned_alloc(16, N * sizeof(int));
    long *larr = aligned_alloc(16, N * sizeof(long));
    float *farr = aligned_alloc(16, N * sizeof(float));
    double *darr = aligned_alloc(16, N * sizeof(double));
    
    /* Initialize with pseudo-random data */
    uint32_t seed = 123456789;
    for (int i = 0; i < N; i++) {
        in1[i] = (float)lcg_parkmiller(&seed) / 1000.0f;
        in2[i] = (float)lcg_parkmiller(&seed) / 1000.0f;
        in3[i] = (float)lcg_parkmiller(&seed) / 1000.0f;
        in4[i] = (float)lcg_parkmiller(&seed) / 1000.0f;
        
        carr[i] = (char)lcg_parkmiller(&seed);
        sarr[i] = (short)lcg_parkmiller(&seed);
        iarr[i] = (int)lcg_parkmiller(&seed);
        larr[i] = (long)lcg_parkmiller(&seed);
        farr[i] = (float)lcg_parkmiller(&seed) / 1000.0f;
        darr[i] = (double)lcg_parkmiller(&seed) / 1000.0;
    }
    
    /* Call function with many vector arguments */
    test_many_args(out, in1, in2, in3, in4, N);
    
    /* Call inline asm with many operands */
    asm_many_operands(out, in1, in2, in3, in4, out, 0x1B, 0x2D, 0x4E);
    
    /* Call complex expression function */
    long complex_result = 0;
    for (int i = 0; i < N - 10; i++) {
        complex_result += complex_many_arg_expr(carr, sarr, iarr, larr, 
                                               farr, darr, i, i+1, i+2, i+3);
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 many argument shuffle if available */
    __m512i avx512_a = _mm512_set1_epi32(1);
    __m512i avx512_b = _mm512_set1_epi32(2);
    __m512i avx512_c = _mm512_set1_epi32(3);
    __m512i avx512_d = _mm512_set1_epi32(4);
    __m512i mask1 = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i mask2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    __m512i avx512_result = avx512_many_arg_shuffle(avx512_a, avx512_b,
                                                   avx512_c, avx512_d,
                                                   mask1, mask2,
                                                   1, 2, 3, 4);
    
    /* Store to prevent optimization */
    _mm512_storeu_si512((__m512i*)out, avx512_result);
#endif
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += out[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Complex result: %ld\n", complex_result);
    
    /* Cleanup */
    free(in1);
    free(in2);
    free(in3);
    free(in4);
    free(out);
    free(carr);
    free(sarr);
    free(iarr);
    free(larr);
    free(farr);
    free(darr);
    
    return 0;
}
