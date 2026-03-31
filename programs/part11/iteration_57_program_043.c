/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int global_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 test using _mm512_mask_permutex2var_epi32 (11 operands when expanded) */
void test_avx512_permute(void) {
    /* Create source vectors */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    /* Index vector for permutation */
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Mask - alternating pattern */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic expands to many operands:
       dest, mask, idx, vec1, vec2 = 5 explicit operands
       But RTL expansion adds more for mask, addressing modes, etc. */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_epi32((void*)global_result, result);
    
    /* Also test the 64-bit version */
    __m512i vec1_64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i vec2_64 = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i idx_64 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __mmask8 mask_64 = 0xAA;
    
    __m512i result2 = _mm512_mask_permutex2var_epi64(vec1_64, mask_64, idx_64, vec2_64, vec1_64);
    _mm512_storeu_epi64((void*)(global_result + 8), result2);
}

/* AVX-512 blend with multiple sources - complex expression */
void test_avx512_complex_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    __mmask16 m1 = 0x5555; /* 0101... */
    __mmask16 m2 = 0x3333; /* 0011... */
    
    /* Complex expression that may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_add_epi32(t1, t2);
    
    /* Force use of result */
    _mm512_storeu_epi32((void*)global_result, result);
}
#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));
typedef int64_t v4di __attribute__((vector_size(32)));

/* This should trigger vec_perm expansion with many operands */
void test_gcc_vector_shuffle(void) {
    v8si a = {0,1,2,3,4,5,6,7};
    v8si b = {8,9,10,11,12,13,14,15};
    
    /* Variable indices to prevent constant folding */
    volatile int idx0 = 0, idx1 = 9, idx2 = 2, idx3 = 11;
    volatile int idx4 = 4, idx5 = 13, idx6 = 6, idx7 = 15;
    
    /* Use __builtin_shuffle with variable indices
       This expands to a vec_perm with potentially many operands */
    v8si result = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    
    /* Store result to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
    
    /* Test with 64-bit vectors as well */
    v4di a64 = {0,1,2,3};
    v4di b64 = {4,5,6,7};
    volatile int idx64_0 = 0, idx64_1 = 5, idx64_2 = 2, idx64_3 = 7;
    
    v4di result64 = __builtin_shufflevector(a64, b64,
        idx64_0, idx64_1, idx64_2, idx64_3);
    
    memcpy((void*)(global_result + 4), &result64, sizeof(result64));
}

/* Complex vector expression that might expand to many operands */
void test_gcc_complex_expr(void) {
    v8si v1 = {1,2,3,4,5,6,7,8};
    v8si v2 = {9,10,11,12,13,14,15,16};
    v8si v3 = {17,18,19,20,21,22,23,24};
    v8si v4 = {25,26,27,28,29,30,31,32};
    
    /* Create masks using comparisons */
    v8si mask1 = v1 < v2;
    v8si mask2 = v3 > v4;
    
    /* Complex blend-like expression using conditional operator emulation
       This creates a complex RTL expression */
    v8si temp1 = mask1 ? v1 : v2;
    v8si temp2 = mask2 ? v3 : v4;
    v8si result = temp1 + temp2;
    
    /* Mix in another operation */
    result = result * (v1 - v2);
    
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather operation with multiple vector arguments */
void test_sve_gather(void) {
    /* These intrinsics typically expand to many operands */
    int64_t base_array[16] = {0};
    int64_t offset_array[16] = {0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120};
    int64_t data_array[16] = {0};
    
    /* Create SVE vectors */
    svint64_t base = svld1_s64(svptrue_b64(), base_array);
    svint64_t offsets = svld1_s64(svptrue_b64(), offset_array);
    
    /* Gather operation - expands to many operands in RTL */
    svint64_t gathered = svld1_gather_s64(svptrue_b64(), base, offsets);
    
    /* Store result */
    svst1_s64(svptrue_b64(), data_array, gathered);
    
    /* Copy to global */
    memcpy((void*)global_result, data_array, sizeof(data_array));
}
#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vector arguments */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_st(result, 0, (vector signed int*)global_result);
}
#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call all available test functions */
    
    /* GCC vector extensions (always available) */
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    
#ifdef __AVX512F__
    printf("AVX-512 tests...\n");
    test_avx512_permute();
    test_avx512_complex_blend();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("SVE tests...\n");
    test_sve_gather();
#endif
    
#ifdef __ALTIVEC__
    printf("Altivec tests...\n");
    test_altivec_permute();
#endif
    
    /* Compute checksum from results to ensure code isn't optimized away */
    for (int i = 0; i < 16; i++) {
        global_checksum += (int)global_result[i];
    }
    
    printf("Checksum: %d\n", global_checksum);
    
    /* Also print a few values to ensure computation happened */
    printf("Sample values: %ld, %ld, %ld\n", 
           (long)global_result[0], 
           (long)global_result[1], 
           (long)global_result[2]);
    
    return 0;
}
