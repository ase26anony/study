/* Test program to trigger 10-11 operand RTL expansions in GCC optabs.cc */
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
    /* Initialize vectors with test data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
    
    /* This intrinsic typically expands to many operands:
       dest, mask, idx, vec1, vec2 = 5 vector/mask operands
       Each vector is 512-bit = potentially multiple registers */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Force use of result to prevent dead code elimination */
    _mm512_storeu_si512((void*)g_result, result);
    
    /* Also test the 64-bit version which might have different expansion */
    __m512i vec1_64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i vec2_64 = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i idx_64  = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __mmask8 mask8 = 0xAA;
    
    __m512i result2 = _mm512_mask_permutex2var_epi64(vec1_64, mask8, idx_64, vec2_64, vec1_64);
    
    /* Mix results to create dependency chain */
    result = _mm512_add_epi32(result, result2);
    _mm512_storeu_si512((void*)(g_result + 8), result);
}

/* Test AVX-512 ternary logic - another candidate for many operands */
void test_avx512_ternlog(void) {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0xCCCCCCCC);
    __m512i c = _mm512_set1_epi32(0xF0F0F0F0);
    
    /* Ternary logic with immediate - may expand to many operands */
    __m512i result = _mm512_ternarylogic_epi32(a, b, c, 0x96); /* (a ^ b) & ~c */
    
    /* Store to prevent optimization */
    _mm512_storeu_si512((void*)(g_result + 16), result);
}

#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle operation that may require many operands */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable indices to prevent constant folding */
    volatile int idx0 = 0, idx1 = 5, idx2 = 10, idx3 = 15;
    volatile int idx4 = 4, idx5 = 9, idx6 = 14, idx7 = 3;
    volatile int idx8 = 8, idx9 = 13, idx10 = 2, idx11 = 7;
    volatile int idx12 = 12, idx13 = 1, idx14 = 6, idx15 = 11;
    
    /* Complex shuffle with many indices - may expand to vec_perm with many operands */
    v16si c = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx8, idx9, idx10, idx11, idx12, idx13, idx14, idx15);
    
    /* Use result to prevent optimization */
    memcpy((void*)g_result, &c, sizeof(c));
    
    /* Another complex expression: blend of three vectors */
    v16si mask1 = {0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1};
    v16si mask2 = {-1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0};
    
    v16si vec1 = a;
    v16si vec2 = b;
    v16si vec3 = c;
    
    /* Complex blend expression that might expand to many operands */
    v16si blend = (mask1 & vec1) | (~mask1 & mask2 & vec2) | (~mask1 & ~mask2 & vec3);
    
    memcpy((void*)(g_result + 32), &blend, sizeof(blend));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands:
       data, predicate, base, offset, scale */
    int64_t base_array[100];
    int64_t offset_array[100];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        base_array[i] = i * 100;
        offset_array[i] = i * 8;
    }
    
    svbool_t pg = svwhilelt_b64(0, svcntd());
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* Gather with multiple vector arguments */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, base_array, offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)g_result, gathered);
    
    /* Test scatter as well */
    svst1_scatter_s64offset_s64(pg, (int64_t*)g_result, offsets, gathered);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector signed int b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vector arguments */
    vector signed int c = vec_perm(a, b, perm);
    
    /* Complex expression with multiple vec_perm calls */
    vector signed int d = vec_add(vec_perm(a, c, perm), vec_perm(b, c, perm));
    
    /* Store results */
    memcpy((void*)g_result, &c, sizeof(c));
    memcpy((void*)(g_result + 16), &d, sizeof(d));
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(int argc, char **argv) {
    /* Initialize result array */
    memset((void*)g_result, 0, sizeof(g_result));
    
    /* Call architecture-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_ternlog();
#endif
    
    /* Always test GCC vector extensions */
    test_gcc_vector_shuffle();
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
#endif
    
    /* Compute checksum to ensure all operations are executed */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (int)g_result[i];
    }
    
    g_checksum = checksum;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
