/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test function using _mm512_mask_permutex2var_epi32 - expands to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Create a mask with alternating bits */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic typically expands to an RTL pattern with many operands:
     * dest, mask, idx, vec1, vec2
     * Each vector register counts as an operand in RTL expansion
     */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Store to volatile global to prevent optimization */
    _mm512_store_epi64((void*)global_result, result);
    
    /* Also test the 64-bit version which may have different operand count */
    __m512i idx64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i vec1_64 = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i vec2_64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __mmask8 mask64 = 0xAA;
    
    __m512i result2 = _mm512_mask_permutex2var_epi64(vec1_64, mask64, idx64, vec2_64, vec1_64);
    _mm512_store_epi64((void*)(global_result + 8), result2);
}

/* Another AVX-512 test with blend operation using multiple masks */
void test_avx512_complex_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    __mmask16 m1 = 0x5555;  /* 01010101... */
    __mmask16 m2 = 0x3333;  /* 00110011... */
    
    /* Complex expression that may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_add_epi32(t1, t2);
    
    /* Force use of result */
    _mm512_store_epi32((void*)global_result, result);
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands:
     * base, offset, scale, predicate, etc.
     */
    int64_t base_array[100];
    for (int i = 0; i < 100; i++) base_array[i] = i * 2;
    
    /* Create SVE vectors - exact API depends on SVE width */
    svint64_t offsets = svindex_s64(0, 1);
    svbool_t pg = svptrue_b64();
    
    /* Gather operation with multiple vector operands */
    svint64_t gathered = svld1_gather_s64index_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)global_result, gathered);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */

/* Portable test using GCC vector extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[16] = {0,17,2,19,4,21,6,23,8,25,10,27,12,29,14,31};
    
    /* Create index vector from array */
    v16si indices = {
        idx_array[0], idx_array[1], idx_array[2], idx_array[3],
        idx_array[4], idx_array[5], idx_array[6], idx_array[7],
        idx_array[8], idx_array[9], idx_array[10], idx_array[11],
        idx_array[12], idx_array[13], idx_array[14], idx_array[15]
    };
    
    /* Complex permutation expression */
    v16si mask = (indices < 16) ? a : b;
    v16si result = __builtin_shuffle(a, b, indices % 32);
    
    /* Store result to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Another GCC extension test with ternary operations */
void test_gcc_complex_ternary(void) {
    v8di v1 = {0,1,2,3,4,5,6,7};
    v8di v2 = {8,9,10,11,12,13,14,15};
    v8di v3 = {16,17,18,19,20,21,22,23};
    v8di v4 = {24,25,26,27,28,29,30,31};
    
    /* Create masks */
    v8di mask1 = {0, -1, 0, -1, 0, -1, 0, -1};  /* alternating */
    v8di mask2 = {0, 0, -1, -1, 0, 0, -1, -1};  /* groups of 2 */
    
    /* Complex expression that may expand to many operands */
    v8di result = (mask1 & v1) | (~mask1 & mask2 & v2) | 
                  (~mask1 & ~mask2 & v3) | (mask1 & mask2 & v4);
    
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,20,21,22,23};
    
    /* vec_perm with three vector arguments */
    vector signed int result = vec_perm(a, b, perm);
    
    vec_st(result, 0, (vector signed int*)global_result);
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call all applicable test functions */
    
    /* GCC vector extensions (always available) */
    test_gcc_vector_shuffle();
    test_gcc_complex_ternary();
    
#ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    test_avx512_permute();
    test_avx512_complex_blend();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Running SVE tests...\n");
    test_sve_gather();
#endif
    
#ifdef __ALTIVEC__
    printf("Running Altivec tests...\n");
    test_altivec_permute();
#endif
    
    /* Compute checksum from results to ensure evaluation */
    for (int i = 0; i < 16; i++) {
        checksum += (int)global_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
