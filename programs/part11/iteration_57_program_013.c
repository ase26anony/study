/* Test program to trigger 10-11 operand RTL expansions in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[64] = {0};
volatile int checksum = 0;

/* ==================== GCC VECTOR EXTENSIONS (Portable) ==================== */

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
        indices[4], indices[5], indices[6], indices[7],
        indices[8], indices[9], indices[10], indices[11],
        indices[12], indices[13], indices[14], indices[15]);
    
    /* Store result to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Complex blend operation with multiple masks */
void test_gcc_complex_blend(v4di a, v4di b, v4di c, v4di d, 
                           v4di mask1, v4di mask2) {
    /* Complex expression that may require many operands */
    v4di result = (mask1 & a) | 
                  (~mask1 & mask2 & b) | 
                  (~mask1 & ~mask2 & c) | 
                  (mask1 & mask2 & d);
    
    /* Additional operations to increase complexity */
    result = result + (mask1 >> 2) + (mask2 << 1);
    
    memcpy((void*)&global_result[8], &result, sizeof(result));
}
#endif

/* ==================== x86_64 AVX-512 INTRINSICS ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask - takes many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has 5 explicit args but expands to many RTL operands */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Additional complex operation */
    result = _mm512_add_epi32(result, _mm512_slli_epi32(idx, 2));
    result = _mm512_xor_epi32(result, _mm512_set1_epi32(0x55555555));
    
    _mm512_storeu_si512((void*)&global_result[16], result);
}

/* AVX-512 ternary logic operation - can expand to many operands */
void test_avx512_ternary(__m512i a, __m512i b, __m512i c) {
    /* _mm512_ternarylogic_epi32 takes 4 operands but complex expansion */
    __m512i result = _mm512_ternarylogic_epi64(a, b, c, 0x96); /* (a ^ b) | (~a & c) */
    
    /* Chain multiple operations */
    result = _mm512_maskz_permutexvar_epi64(0xFF, result, _mm512_set_epi64(7,6,5,4,3,2,1,0));
    
    _mm512_storeu_si512((void*)&global_result[24], result);
}
#endif

/* ==================== ARM SVE INTRINSICS ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather operation with multiple vector arguments */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64 takes base, offsets, predicate - may expand to many operands */
    svint64_t result = svld1_gather_s64(pg, &global_result[32], offsets);
    
    /* Complex operation chain */
    result = svadd_x(pg, result, base);
    result = svmul_x(pg, result, svdup_s64(2));
    
    svst1_s64(pg, &global_result[32], result);
}

/* SVE table lookup/permute with multiple vectors */
void test_sve_tbl(svint64_t data, svint64_t indices, svint64_t fallback, svbool_t pg) {
    /* svtbl takes data and indices - complex expansion possible */
    svint64_t result = svtbl_s64(data, indices);
    
    /* Blend with fallback using predicate */
    result = svsel_s64(pg, result, fallback);
    
    svst1_s64(pg, &global_result[40], result);
}
#endif

/* ==================== POWERPC ALTIVEC/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* Altivec vec_perm with three vectors - may expand further */
void test_altivec_perm(vector signed long long a, 
                      vector signed long long b,
                      vector unsigned char perm) {
    /* vec_perm takes 3 vectors - complex RTL expansion possible */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Additional operations */
    result = vec_add(result, vec_sll(result, vec_splats((unsigned int)2)));
    
    vec_st(result, 0, (vector signed long long*)&global_result[48]);
}

/* Complex vector select with multiple conditions */
void test_altivec_select(vector signed long long a, vector signed long long b,
                        vector signed long long c, vector signed long long d,
                        vector bool long long cond1, vector bool long long cond2) {
    /* Nested selects can expand to many operands */
    vector signed long long temp = vec_sel(a, b, cond1);
    vector signed long long result = vec_sel(c, d, cond2);
    result = vec_sel(temp, result, cond1 & cond2);
    
    vec_st(result, 0, (vector signed long long*)&global_result[56]);
}
#endif

/* ==================== MAIN TEST DRIVER ==================== */

int main() {
    /* Initialize test data */
    int indices[16];
    for (int i = 0; i < 16; i++) {
        indices[i] = (i * 3) % 16; /* Non-trivial permutation pattern */
    }
    
    /* Test GCC vector extensions if available */
#ifdef __GNUC__
    {
        v8di a = {0,1,2,3,4,5,6,7};
        v8di b = {8,9,10,11,12,13,14,15};
        v4di c = {16,17,18,19};
        v4di d = {20,21,22,23};
        v4di mask1 = {0xFF, 0xFF, 0xFF, 0xFF};
        v4di mask2 = {0x00, 0xFF, 0x00, 0xFF};
        
        test_gcc_vector_shuffle(a, b, indices);
        test_gcc_complex_blend(c, d, c, d, mask1, mask2);
    }
#endif
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
        
        test_avx512_permute(a, b, idx, mask);
        test_avx512_ternary(a, b, idx);
    }
#endif
    
    /* Test ARM SVE if available */
#ifdef __ARM_FEATURE_SVE
    {
        svbool_t pg = svptrue_b64();
        svint64_t base = svdup_s64(100);
        svint64_t offsets = svindex_s64(0, 1);
        
        test_sve_gather(base, offsets, pg);
        
        svint64_t data = svdup_s64(50);
        svint64_t idx = svindex_s64(0, 2);
        svint64_t fallback = svdup_s64(0);
        
        test_sve_tbl(data, idx, fallback, pg);
    }
#endif
    
    /* Test PowerPC Altivec if available */
#ifdef __ALTIVEC__
    {
        vector signed long long a = {1,2};
        vector signed long long b = {3,4};
        vector signed long long c = {5,6};
        vector signed long long d = {7,8};
        vector unsigned char perm = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        vector bool long long cond1 = {0xFF, 0x00};
        vector bool long long cond2 = {0x00, 0xFF};
        
        test_altivec_perm(a, b, perm);
        test_altivec_select(a, b, c, d, cond1, cond2);
    }
#endif
    
    /* Compute checksum from results to ensure all operations executed */
    for (int i = 0; i < 64; i++) {
        checksum += global_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
