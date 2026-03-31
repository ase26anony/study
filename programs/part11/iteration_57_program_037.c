/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[16] = {0};
volatile int g_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - likely to generate many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx = _mm512_set_epi32(30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0);
    
    /* Use a mask to control blending */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic takes 5 arguments but expands to many more operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_epi32((void*)g_result, result);
    
    /* Compute checksum */
    for (int i = 0; i < 16; i++) {
        g_checksum += g_result[i];
    }
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    __m512i d = _mm512_set_epi64(31,30,29,28,27,26,25,24);
    
    /* Complex blend operation that may expand to many operands */
    __m512i ab = _mm512_blend_epi32(a, b, 0xCC); /* 0b11001100 */
    __m512i cd = _mm512_blend_epi32(c, d, 0x33); /* 0b00110011 */
    __m512i result = _mm512_blend_epi32(ab, cd, 0xAA); /* 0b10101010 */
    
    _mm512_storeu_epi64((void*)g_result, result);
    
    for (int i = 0; i < 8; i++) {
        g_checksum += g_result[i];
    }
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* SVE gather operation with multiple vector operands */
void test_sve_gather(void) {
    /* Base array for gather */
    int64_t base_array[32];
    for (int i = 0; i < 32; i++) {
        base_array[i] = i * 2;
    }
    
    /* Create offset vector */
    svint64_t offsets = svindex_s64(0, 1);
    
    /* Create predicate */
    svbool_t pg = svptrue_b64();
    
    /* Gather operation - expands to many operands in RTL */
    svint64_t gathered = svld1_gather_s64index_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)g_result, gathered);
    
    for (int i = 0; i < 16; i++) {
        g_checksum += g_result[i];
    }
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));
typedef int64_t v4di __attribute__((vector_size(32)));

/* Test with GCC vector shuffle - may generate vec_perm with many operands */
void test_gcc_vector_shuffle(void) {
    v8si a = {0,1,2,3,4,5,6,7};
    v8si b = {8,9,10,11,12,13,14,15};
    
    /* Variable indices to prevent constant folding */
    volatile int idx0 = 0, idx1 = 9, idx2 = 2, idx3 = 11;
    volatile int idx4 = 4, idx5 = 13, idx6 = 6, idx7 = 15;
    
    /* Use __builtin_shufflevector with variable indices */
    v8si result = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    
    /* Store result */
    memcpy((void*)g_result, &result, sizeof(result));
    
    for (int i = 0; i < 8; i++) {
        g_checksum += g_result[i];
    }
}

/* Complex vector expression that may expand to many operands */
void test_gcc_complex_expr(void) {
    v4di v1 = {1, 2, 3, 4};
    v4di v2 = {5, 6, 7, 8};
    v4di v3 = {9, 10, 11, 12};
    v4di v4 = {13, 14, 15, 16};
    
    /* Complex expression that might generate many operands */
    v4di mask1 = (v1 > v2) ? v1 : v2;
    v4di mask2 = (v3 < v4) ? v3 : v4;
    
    /* Even more complex blend-like operation */
    v4di result = (mask1 & v1) | (~mask1 & v2) | (mask2 & v3) | (~mask2 & v4);
    
    memcpy((void*)g_result, &result, sizeof(result));
    
    for (int i = 0; i < 4; i++) {
        g_checksum += g_result[i];
    }
}

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector signed int c = {8,9,10,11};
    
    /* vec_perm with three vectors - may expand to many operands */
    vector signed char perm = {0,1,2,3, 16,17,18,19, 4,5,6,7, 20,21,22,23};
    
    /* Complex permutation */
    vector signed int ab = vec_perm(a, b, perm);
    vector signed int result = vec_add(ab, c);
    
    memcpy((void*)g_result, &result, sizeof(result));
    
    for (int i = 0; i < 4; i++) {
        g_checksum += g_result[i];
    }
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Reset checksum */
    g_checksum = 0;
    
    /* Call target-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests completed\n");
#endif
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE tests completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec tests completed\n");
#endif
    
    /* Always run GCC vector tests (most portable) */
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    printf("GCC vector tests completed\n");
    
    printf("Final checksum: %d\n", g_checksum);
    
    return 0;
}
