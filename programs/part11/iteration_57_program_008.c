/* Test program to cover 10-11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

/* Using GCC vector extensions for portable vector operations */
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, int *indices) {
    /* Create a complex permutation with variable indices */
    v8di result;
    
    /* This complex expression may require many operands during RTL expansion */
    result = __builtin_shufflevector(a, b, 
        indices[0], indices[1], indices[2], indices[3],
        indices[4], indices[5], indices[6], indices[7]);
    
    /* Store to volatile to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Complex blend operation with multiple vectors */
void test_gcc_complex_blend(v4di a, v4di b, v4di c, v4di d, v4di mask1, v4di mask2) {
    v4di result;
    
    /* Complex blend expression that may expand to many operands */
    result = (mask1 & a) | (~mask1 & mask2 & b) | 
             (~mask1 & ~mask2 & c) | (mask1 & mask2 & d);
    
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permutex2var intrinsic which takes many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    __m512i result;
    
    /* _mm512_mask_permutex2var_epi32 has many operands:
       result, a, mask, idx, b -> expands to 5 register operands plus constants */
    result = _mm512_mask_permutex2var_epi32(a, mask, idx, b);
    
    /* Another variant with more operands */
    result = _mm512_permutex2var_epi64(a, idx, b);
    
    /* Complex blend with mask - may expand further */
    __m512i c = _mm512_set1_epi32(0xFFFFFFFF);
    result = _mm512_mask_blend_epi32(mask, a, b);
    
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Test AVX-512 gather with complex addressing */
void test_avx512_gather(__m512i base, __m512i index, __mmask16 mask) {
    __m512i result;
    
    /* Gather operation with base, scale, index, mask, src - many operands */
    result = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), 
                                        mask, 
                                        index, 
                                        (const void*)global_result, 
                                        8);
    
    memcpy((void*)global_result, &result, sizeof(result));
}
#endif

/* ==================== AArch64 SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    svint64_t result;
    
    /* SVE gather typically expands to many operands */
    result = svld1_gather_s64_offset_s64(pg, (const int64_t*)global_result, offsets);
    
    /* Store result to prevent optimization */
    svst1_s64(pg, (int64_t*)global_result, result);
}

/* SVE complex arithmetic with multiple operands */
void test_sve_complex(svint64_t a, svint64_t b, svint64_t c, svint64_t d, svbool_t pg) {
    svint64_t result;
    
    /* Complex expression that may expand to many operands */
    result = svadd_s64_z(pg, 
                        svmul_s64_z(pg, a, b),
                        svmla_s64_z(pg, c, d, a));
    
    svst1_s64(pg, (int64_t*)global_result, result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* PowerPC vec_perm with three vectors */
void test_ppc_vec_perm(vector signed long long a, 
                       vector signed long long b, 
                       vector unsigned char perm) {
    vector signed long long result;
    
    /* vec_perm takes three vectors - may expand further during RTL */
    result = vec_perm(a, b, perm);
    
    vec_st(result, 0, (vector signed long long*)global_result);
}

/* Complex vector select with multiple conditions */
void test_ppc_complex_select(vector signed long long a, 
                             vector signed long long b,
                             vector signed long long c,
                             vector signed long long d,
                             vector bool long long mask1,
                             vector bool long long mask2) {
    vector signed long long result;
    
    /* Complex select expression */
    result = vec_sel(a, b, mask1);
    result = vec_sel(result, c, mask2);
    result = vec_sel(result, d, vec_and(mask1, mask2));
    
    vec_st(result, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main(int argc, char **argv) {
    /* Initialize test data */
    int indices[8] = {0, 8, 1, 9, 2, 10, 3, 11};
    
    /* Test GCC vector extensions (always available) */
    {
        v8di a = {0, 1, 2, 3, 4, 5, 6, 7};
        v8di b = {8, 9, 10, 11, 12, 13, 14, 15};
        test_gcc_vector_shuffle(a, b, indices);
        
        v4di a4 = {0, 1, 2, 3};
        v4di b4 = {4, 5, 6, 7};
        v4di c4 = {8, 9, 10, 11};
        v4di d4 = {12, 13, 14, 15};
        v4di mask1 = {-1, 0, -1, 0};
        v4di mask2 = {0, -1, 0, -1};
        test_gcc_complex_blend(a4, b4, c4, d4, mask1, mask2);
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    {
        __m512i a = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 
                                     7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi32(31, 30, 29, 28, 27, 26, 25, 24, 
                                     23, 22, 21, 20, 19, 18, 17, 16);
        __m512i idx = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 
                                       8, 9, 10, 11, 12, 13, 14, 15);
        __mmask16 mask = 0xAAAA;  /* Alternating bits */
        
        test_avx512_permute(a, b, idx, mask);
        
        __m512i base = _mm512_set1_epi32(0);
        __m512i index = _mm512_set_epi32(0, 8, 16, 24, 32, 40, 48, 56,
                                         64, 72, 80, 88, 96, 104, 112, 120);
        test_avx512_gather(base, index, mask);
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    /* Test SVE if available */
    {
        svbool_t pg = svptrue_b64();
        svint64_t base = svdup_s64(0);
        svint64_t offsets = svindex_s64(0, 8);
        
        test_sve_gather(base, offsets, pg);
        
        svint64_t a = svdup_s64(1);
        svint64_t b = svdup_s64(2);
        svint64_t c = svdup_s64(3);
        svint64_t d = svdup_s64(4);
        test_sve_complex(a, b, c, d, pg);
    }
#endif
    
#ifdef __ALTIVEC__
    /* Test PowerPC Altivec if available */
    {
        vector signed long long a = {0, 1};
        vector signed long long b = {2, 3};
        vector unsigned char perm = {0, 1, 2, 3, 4, 5, 6, 7,
                                     16, 17, 18, 19, 20, 21, 22, 23};
        
        test_ppc_vec_perm(a, b, perm);
        
        vector signed long long c = {4, 5};
        vector signed long long d = {6, 7};
        vector bool long long mask1 = {0, -1};
        vector bool long long mask2 = {-1, 0};
        
        test_ppc_complex_select(a, b, c, d, mask1, mask2);
    }
#endif
    
    /* Compute checksum from results to ensure code executes */
    for (int i = 0; i < 16; i++) {
        checksum += global_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
