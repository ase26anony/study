/* Test program to trigger 10-11 operand RTL expansions in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

#ifdef __GNUC__
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, int *indices) {
    /* Create a complex permutation pattern */
    v8di result;
    
    /* Use __builtin_shuffle with variable indices - forces runtime permutation */
    /* This may expand to a vec_perm RTL with many operands */
    result = __builtin_shufflevector(a, b, 
        indices[0], indices[1], indices[2], indices[3],
        indices[4], indices[5], indices[6], indices[7]);
    
    /* Store to volatile to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Complex blend operation with multiple vectors */
void test_gcc_complex_blend(v4di a, v4di b, v4di c, v4di d, v4di mask1, v4di mask2) {
    /* Complex expression that may require many operands */
    v4di result = (mask1 & a) | (~mask1 & mask2 & b) | 
                  (~mask1 & ~mask2 & c) | (mask1 & mask2 & d);
    
    memcpy((void*)global_result, &result, sizeof(result));
}
#endif

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permute with mask - can generate many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 takes 5 arguments but expands to many operands */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Complex chain of operations */
    __m512i temp = _mm512_add_epi32(result, b);
    temp = _mm512_maskz_permutexvar_epi32(mask, idx, temp);
    temp = _mm512_add_epi32(temp, a);
    
    _mm512_store_epi64((void*)global_result, temp);
}

/* AVX-512 blend with multiple vectors and masks */
void test_avx512_complex_blend(__m512i v1, __m512i v2, __m512i v3, __m512i v4,
                               __mmask16 m1, __mmask16 m2, __mmask16 m3) {
    /* Complex blend pattern that may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, v1, v2);
    __m512i t2 = _mm512_mask_blend_epi32(m2, v3, v4);
    __m512i result = _mm512_mask_blend_epi32(m3, t1, t2);
    
    /* Additional operations to prevent simplification */
    result = _mm512_add_epi64(result, v1);
    result = _mm512_xor_si512(result, v2);
    
    _mm512_store_epi64((void*)global_result, result);
}
#endif

/* ==================== AArch64 SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments - can have many operands */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64 takes base, offsets, predicate - expands to many operands */
    svint64_t result = svld1_gather_s64(pg, (const int64_t*)global_result, offsets);
    
    /* Store with scatter - another multi-operand operation */
    svst1_scatter_s64(pg, (int64_t*)global_result, offsets, result);
    
    /* Complex expression with gather and arithmetic */
    svint64_t scaled = svmul_s64_z(pg, result, base);
    svst1_scatter_s64(pg, (int64_t*)global_result + 8, offsets, scaled);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* PowerPC vec_perm with three vectors - may expand further */
void test_powerpc_vec_perm(vector signed long long a,
                          vector signed long long b,
                          vector unsigned char perm) {
    /* vec_perm with three vectors */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Complex permutation chain */
    vector signed long long temp = vec_add(result, a);
    temp = vec_perm(temp, b, perm);
    
    vec_st(temp, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main() {
    int test_indices[8] = {0, 8, 1, 9, 2, 10, 3, 11};  /* Cross-lane indices */
    
    /* Initialize some test data */
    int64_t data_a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int64_t data_b[8] = {8, 9, 10, 11, 12, 13, 14, 15};
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    
    /* Test GCC vector extensions (always available with GCC) */
#ifdef __GNUC__
    {
        v8di vec_a, vec_b;
        memcpy(&vec_a, data_a, sizeof(vec_a));
        memcpy(&vec_b, data_b, sizeof(vec_b));
        
        test_gcc_vector_shuffle(vec_a, vec_b, test_indices);
        
        /* Compute checksum from result */
        for (int i = 0; i < 8; i++) {
            checksum += global_result[i];
        }
        
        printf("GCC vector shuffle test completed\n");
    }
#endif
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512i avx_a = _mm512_loadu_epi64(data_a);
        __m512i avx_b = _mm512_loadu_epi64(data_b);
        __m512i idx = _mm512_set_epi32(7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8);
        __mmask16 mask = 0xAAAA;  /* Alternating bits */
        
        test_avx512_permute(avx_a, avx_b, idx, mask);
        
        for (int i = 0; i < 8; i++) {
            checksum += global_result[i];
        }
        
        printf("AVX-512 permute test completed\n");
    }
#endif
    
    /* Test SVE if available */
#ifdef __ARM_FEATURE_SVE
    {
        /* Note: SVE vectors are variable-length, so we use generic patterns */
        svint64_t sve_base = svld1_s64(svptrue_b64(), data_a);
        svint64_t offsets = svld1_s64(svptrue_b64(), data_b);
        svbool_t pg = svptrue_b64();
        
        test_sve_gather(sve_base, offsets, pg);
        
        printf("SVE gather test completed\n");
    }
#endif
    
    /* Test PowerPC if available */
#ifdef __ALTIVEC__
    {
        vector signed long long ppc_a = vec_ld(0, (vector signed long long*)data_a);
        vector signed long long ppc_b = vec_ld(0, (vector signed long long*)data_b);
        vector unsigned char perm = {0,1,2,3,4,5,6,7, 16,17,18,19,20,21,22,23};
        
        test_powerpc_vec_perm(ppc_a, ppc_b, perm);
        
        printf("PowerPC vec_perm test completed\n");
    }
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully!\n");
    
    return 0;
}
