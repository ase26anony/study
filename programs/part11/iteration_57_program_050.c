/* Test program to trigger 10/11 operand RTL expansions in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle that may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, v8di mask) {
    /* Create a complex permutation expression */
    v8di idx = {0, 9, 2, 11, 4, 13, 6, 15};
    v8di temp1 = __builtin_shufflevector(a, b, 0, 9, 2, 11, 4, 13, 6, 15);
    v8di temp2 = __builtin_shufflevector(b, a, 15, 6, 13, 4, 11, 2, 9, 0);
    
    /* Complex blend operation - may require many operands when expanded */
    v8di result = (mask & temp1) | (~mask & temp2);
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask - can expand to many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has 5 explicit args but expands to more */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Another complex permutation */
    __m512i result2 = _mm512_permutex2var_epi64(result, idx, b);
    
    /* Blend them based on mask */
    __m512i final = _mm512_mask_blend_epi32(mask, result, result2);
    
    /* Store to volatile global */
    _mm512_storeu_epi64((void*)global_result, final);
}

/* Test AVX-512 gather with many operands */
void test_avx512_gather(__m512i base, __m512i index, __mmask16 mask) {
    /* _mm512_mask_i64gather_epi64 expands to many operands */
    __m512i result = _mm512_mask_i64gather_epi64(_mm512_setzero_si512(), mask,
                                                index, (const void*)global_result, 8);
    
    /* Complex expression with multiple operations */
    __m512i scaled = _mm512_slli_epi64(result, 2);
    __m512i added = _mm512_add_epi64(scaled, base);
    
    _mm512_storeu_epi64((void*)(global_result + 8), added);
}
#endif

/* ==================== AArch64 SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with predicate, base, and offset - many operands */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64_index has 3 args but expands to many RTL operands */
    svint64_t result = svld1_gather_s64_index(pg, (const int64_t*)global_result,
                                             svreinterpret_u64(base), offsets);
    
    /* Store with scatter - also expands to many operands */
    svst1_scatter_s64_index(pg, (int64_t*)(global_result + 8), 
                           svreinterpret_u64(base), offsets, result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* vec_perm with three vectors - may expand further */
void test_altivec_perm(vector signed long long a,
                       vector signed long long b,
                       vector unsigned char perm) {
    /* vec_perm with variable permute control */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Complex expression */
    vector signed long long shifted = vec_sl(result, (vector unsigned long long){2,2});
    vector signed long long blended = vec_sel(a, shifted, (vector bool long long){-1,0});
    
    /* Store */
    vec_st(blended, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main(int argc, char *argv[]) {
    /* Initialize with some data */
    v8di vec_a = {0, 1, 2, 3, 4, 5, 6, 7};
    v8di vec_b = {8, 9, 10, 11, 12, 13, 14, 15};
    v8di mask = {-1, 0, -1, 0, -1, 0, -1, 0};
    
    /* Test portable GCC vector extensions */
    test_gcc_vector_shuffle(vec_a, vec_b, mask);
    
    /* Test architecture-specific intrinsics if available */
#ifdef __AVX512F__
    {
        __m512i avx_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i avx_b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
        __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __mmask16 m = 0xAAAA; /* 1010101010101010 */
        
        test_avx512_permute(avx_a, avx_b, idx, m);
        test_avx512_gather(avx_a, idx, m);
    }
#endif

#ifdef __ARM_FEATURE_SVE
    {
        /* SVE test would require runtime length detection */
        /* This is a placeholder - actual SVE code needs careful handling */
    }
#endif

#ifdef __ALTIVEC__
    {
        vector signed long long alti_a = {0, 1};
        vector signed long long alti_b = {2, 3};
        vector unsigned char perm = {0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23};
        
        test_altivec_perm(alti_a, alti_b, perm);
    }
#endif

    /* Compute checksum to ensure all operations executed */
    for (int i = 0; i < 16; i++) {
        checksum += (int)global_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
