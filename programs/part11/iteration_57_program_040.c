/* Test program to cover 10-11 operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[8] = {0};
volatile int g_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

void test_avx512_permute(void) {
    /* Create 10+ operand permutation operation */
    __m512i src1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i src2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* Alternating bits pattern */
    
    /* This intrinsic expands to many operands:
     * dest, mask, idx, src1, src2 = 5 explicit operands
     * But RTL expansion adds more for mask, immediate constants, etc.
     */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_si512((void*)g_result, result);
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0x5555;
    __mmask16 m2 = 0x3333;
    
    /* Complex blend expression that may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_add_epi32(t1, t2);
    
    _mm512_storeu_si512((void*)g_result, result);
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));

void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable indices to prevent constant folding */
    volatile int idx0 = 0, idx1 = 5, idx2 = 2, idx3 = 7;
    volatile int idx4 = 8, idx5 = 13, idx6 = 10, idx7 = 15;
    volatile int idx8 = 16, idx9 = 21, idx10 = 18, idx11 = 23;
    volatile int idx12 = 24, idx13 = 29, idx14 = 26, idx15 = 31;
    
    /* Complex shuffle with many indices - may expand to vec_perm with many operands */
    v16si c = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx8, idx9, idx10, idx11, idx12, idx13, idx14, idx15);
    
    /* Use result to prevent optimization */
    memcpy((void*)g_result, &c, sizeof(c));
}

/* Complex vector expression that may generate many RTL operands */
void test_gcc_complex_expr(void) {
    v16si v1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si v2 = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
    v16si v3 = {33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
    v16si v4 = {49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64};
    
    /* Complex expression that might expand to many operands */
    v16si mask1 = v1 > v2;
    v16si mask2 = v3 < v4;
    
    /* This complex blend-like operation may generate many RTL operands */
    v16si result = (mask1 & v1) | (~mask1 & mask2 & v2) | 
                   (~mask1 & ~mask2 & v3) | (mask1 & mask2 & v4);
    
    memcpy((void*)g_result, &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    int64_t base_array[100];
    for (int i = 0; i < 100; i++) base_array[i] = i * 10;
    
    svint64_t offsets = svindex_s64(0, 1);
    svbool_t pg = svptrue_b64();
    
    /* Gather with base, offsets, predicate - expands to many operands */
    svint64_t gathered = svld1_gather_s64index_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)g_result, gathered);
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {1,2,3,4};
    vector signed int b = {5,6,7,8};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vectors may expand further */
    vector signed int c = vec_perm(a, b, perm);
    
    /* Complex expression with multiple vec_perms */
    vector signed int d = vec_add(vec_perm(a, b, perm), 
                                  vec_perm(b, a, vec_sub((vector unsigned char)perm, 
                                                         (vector unsigned char){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1})));
    
    memcpy((void*)g_result, &d, sizeof(d));
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call target-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests executed\n");
#endif
    
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    printf("GCC vector tests executed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE tests executed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec tests executed\n");
#endif
    
    /* Compute checksum from results */
    for (int i = 0; i < 8; i++) {
        g_checksum += (int)g_result[i];
    }
    
    printf("Checksum: %d\n", g_checksum);
    return 0;
}
