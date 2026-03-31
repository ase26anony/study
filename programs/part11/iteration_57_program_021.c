/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[64] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - likely expands to many operands */
void test_avx512_permute(void) {
    /* Create 10+ operands for permutation */
    __m512i src1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i src2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* Alternating bits */
    
    /* This intrinsic takes 5 explicit arguments but expands to many more RTL operands */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Use volatile store to prevent optimization */
    _mm512_storeu_epi32((void*)global_result, result);
    
    /* Complex expression that might expand further */
    __m512i blend = _mm512_mask_blend_epi32(0x5555, result, src2);
    _mm512_storeu_epi32((void*)(global_result + 16), blend);
}

/* AVX-512 ternary logic with 3 source vectors and immediate */
void test_avx512_ternary(void) {
    __m512i a = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i b = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i c = _mm512_set1_epi32(0x55555555);
    
    /* vpternlogd with 3 vectors + immediate = many operands when expanded */
    __m512i res = _mm512_ternarylogic_epi32(a, b, c, 0x96);  /* XOR pattern */
    _mm512_storeu_epi32((void*)(global_result + 32), res);
}

#endif  /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to vec_perm with many ops */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable indices prevent constant folding */
    volatile int idx0 = 0, idx1 = 5, idx2 = 10, idx3 = 15;
    volatile int idx4 = 16, idx5 = 21, idx6 = 26, idx7 = 31;
    volatile int idx8 = 3, idx9 = 7, idx10 = 11, idx11 = 15;
    volatile int idx12 = 19, idx13 = 23, idx14 = 27, idx15 = 31;
    
    /* Complex shuffle expression - GCC may expand this to many operands */
    v16si result = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx8, idx9, idx10, idx11, idx12, idx13, idx14, idx15);
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Complex vector expression that might require multi-operand expansion */
void test_gcc_complex_expr(void) {
    v8di v1 = {0,1,2,3,4,5,6,7};
    v8di v2 = {8,9,10,11,12,13,14,15};
    v8di v3 = {16,17,18,19,20,21,22,23};
    v8di v4 = {24,25,26,27,28,29,30,31};
    v8di mask1 = {0, -1, 0, -1, 0, -1, 0, -1};
    v8di mask2 = {-1, 0, -1, 0, -1, 0, -1, 0};
    
    /* Complex blend expression - may expand to many RTL operands */
    v8di temp1 = v1 & mask1;
    v8di temp2 = v2 & ~mask1 & mask2;
    v8di temp3 = v3 & ~mask1 & ~mask2;
    v8di temp4 = v4 & mask1 & mask2;
    
    v8di result = temp1 | temp2 | temp3 | temp4;
    
    memcpy((void*)(global_result + 32), &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather with multiple vector arguments */
    int64_t base_array[100];
    for (int i = 0; i < 100; i++) base_array[i] = i * 2;
    
    svint64_t offsets = svindex_s64(0, 1);
    svbool_t pg = svptrue_b64();
    
    /* Gather with base + offsets + predicate = many operands */
    svint64_t gathered = svld1_gather_s64index_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)global_result, gathered);
}

#endif  /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with 3 vectors - may expand to many RTL operands */
    vector signed int result = vec_perm(a, b, perm);
    
    memcpy((void*)global_result, &result, sizeof(result));
}

#endif  /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansions...\n");
    
    /* Call all applicable test functions */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_ternary();
#endif
    
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
#endif
    
    /* Compute checksum to ensure code isn't optimized away */
    for (int i = 0; i < 64; i++) {
        checksum += (int)global_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
