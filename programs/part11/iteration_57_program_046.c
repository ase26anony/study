/* Test program to trigger 10- and 11-operand RTL expansions in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_results[256] = {0};
volatile int result_index = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - likely to generate many operands */
void test_avx512_permute(void) {
    /* Create source vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    /* Index vector for permutation */
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Mask - alternating pattern */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
    
    /* This intrinsic takes 5 arguments but expands to many RTL operands */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store results to prevent optimization */
    _mm512_storeu_si512((void*)&global_results[result_index], result);
    result_index += 8;  /* 512-bit = 8x 64-bit */
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    __m512i d = _mm512_set_epi64(31,30,29,28,27,26,25,24);
    
    /* Complex blend operation that may expand to many operands */
    __mmask8 mask1 = 0xAA;  /* 10101010 */
    __mmask8 mask2 = 0x33;  /* 00110011 */
    
    /* Multi-operand blend pattern */
    __m512i ab = _mm512_mask_blend_epi64(mask1, a, b);
    __m512i cd = _mm512_mask_blend_epi64(mask2, c, d);
    __m512i result = _mm512_mask_blend_epi64(0x0F, ab, cd);
    
    _mm512_storeu_si512((void*)&global_results[result_index], result);
    result_index += 8;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* SVE gather operation with multiple operands */
void test_sve_gather(void) {
    /* Base array */
    int64_t base_array[32];
    for (int i = 0; i < 32; i++) base_array[i] = i * 100;
    
    /* Offset vector */
    svint64_t offsets = svindex_s64(0, 1);
    
    /* Predicate - all true */
    svbool_t pg = svptrue_b64();
    
    /* Gather operation - expands to many RTL operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, &base_array[0], offsets);
    
    /* Store results */
    svst1_s64(pg, &global_results[result_index], gathered);
    result_index += svcntd();  /* Number of 64-bit elements */
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int64_t v4di __attribute__((vector_size(32)));  /* 256-bit vector */
typedef int64_t v2di __attribute__((vector_size(16)));  /* 128-bit vector */

/* Complex shuffle operation with variable indices */
void test_gcc_vector_shuffle(void) {
    /* Source vectors */
    v4di a = {0, 1, 2, 3};
    v4di b = {4, 5, 6, 7};
    
    /* Variable index array - prevents constant folding */
    volatile int idx_array[8] = {0, 5, 2, 7, 1, 4, 3, 6};
    
    /* Create index vector from array */
    v4di idx = {idx_array[0], idx_array[1], idx_array[2], idx_array[3]};
    
    /* Complex expression that may require many operands */
    v4di mask1 = {0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00};
    v4di mask2 = {0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF};
    
    /* This complex expression may expand to many RTL operands */
    v4di shuffled = __builtin_shufflevector(a, b, 
        idx_array[0] & 7, idx_array[1] & 7, idx_array[2] & 7, idx_array[3] & 7);
    
    /* Store results */
    memcpy((void*)&global_results[result_index], &shuffled, sizeof(shuffled));
    result_index += 4;
}

/* Test with even more operands using nested shuffles */
void test_gcc_complex_shuffle(void) {
    v4di v1 = {10, 11, 12, 13};
    v4di v2 = {20, 21, 22, 23};
    v4di v3 = {30, 31, 32, 33};
    v4di v4 = {40, 41, 42, 43};
    
    /* Variable indices from global array */
    volatile int indices[16] = {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7};
    
    /* Complex multi-vector shuffle expression */
    v4di shuffle1 = __builtin_shufflevector(v1, v2, 
        indices[0], indices[1], indices[2], indices[3]);
    v4di shuffle2 = __builtin_shufflevector(v3, v4, 
        indices[4], indices[5], indices[6], indices[7]);
    
    /* Final shuffle combining results */
    v4di result = __builtin_shufflevector(shuffle1, shuffle2,
        indices[8], indices[9], indices[10], indices[11]);
    
    memcpy((void*)&global_results[result_index], &result, sizeof(result));
    result_index += 4;
}

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    /* Create vectors */
    vector signed int v1 = {1, 2, 3, 4};
    vector signed int v2 = {5, 6, 7, 8};
    vector unsigned char perm = {0,1,2,3, 16,17,18,19, 8,9,10,11, 24,25,26,27};
    
    /* vec_perm takes 3 vectors but may expand further */
    vector signed int result = vec_perm(v1, v2, perm);
    
    /* Store result */
    memcpy((void*)&global_results[result_index], &result, sizeof(result));
    result_index += 2;  /* 128-bit = 2x 64-bit */
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call architecture-specific tests */
#ifdef __AVX512F__
    printf("Testing AVX-512...\n");
    test_avx512_permute();
    test_avx512_blend();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE...\n");
    test_sve_gather();
#endif
    
#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec...\n");
    test_altivec_permute();
#endif
    
    /* Always test GCC vector extensions */
    printf("Testing GCC vector extensions...\n");
    test_gcc_vector_shuffle();
    test_gcc_complex_shuffle();
    
    /* Compute checksum to ensure operations executed */
    int64_t checksum = 0;
    for (int i = 0; i < result_index && i < 256; i++) {
        checksum += global_results[i];
    }
    
    printf("Result checksum: %ld\n", (long)checksum);
    printf("Total elements stored: %d\n", result_index);
    
    return 0;
}
