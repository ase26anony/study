/* Test program to cover 10/11 operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[8] = {0};
volatile int g_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with mask - may expand to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential values */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    
    /* Index vector - select alternating elements */
    __m512i idx = _mm512_set_epi32(30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0);
    
    /* Mask - alternating bits */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic takes 5 arguments but may expand to more operands */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_si512((void*)g_result, result);
    
    /* Compute simple checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += g_result[i];
    }
    g_checksum += sum;
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    __m512i d = _mm512_set_epi64(31,30,29,28,27,26,25,24);
    
    /* Complex blend expression that may require many operands */
    __mmask8 mask1 = 0xAA; /* 0b10101010 */
    __mmask8 mask2 = 0x33; /* 0b00110011 */
    
    /* Multi-step blend that could be combined */
    __m512i temp1 = _mm512_mask_blend_epi64(mask1, a, b);
    __m512i temp2 = _mm512_mask_blend_epi64(mask2, c, d);
    __m512i result = _mm512_mask_blend_epi64(0x0F, temp1, temp2);
    
    _mm512_storeu_si512((void*)g_result, result);
}
#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector extensions for general coverage */
typedef int32_t v16si __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[16] = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    /* Create index vector from array */
    v16si indices;
    for (int i = 0; i < 16; i++) {
        indices[i] = idx_array[i];
    }
    
    /* Complex expression that may require many operands during expansion */
    v16si result;
    
    /* Manual permutation using conditional selects - creates complex RTL */
    for (int i = 0; i < 16; i++) {
        int idx = indices[i];
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Store result */
    memcpy((void*)g_result, &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather operation with multiple vector arguments */
void test_sve_gather(void) {
    /* Base array */
    int64_t base_array[100];
    for (int i = 0; i < 100; i++) {
        base_array[i] = i * 10;
    }
    
    /* Create offset vector */
    svint64_t offsets = svindex_s64(0, 2); /* 0, 2, 4, ... */
    
    /* All-true predicate */
    svbool_t pg = svptrue_b64();
    
    /* Gather with base + offsets - may expand to many operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)g_result, gathered);
}
#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    vector unsigned int a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned int b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vectors - may expand further */
    vector unsigned int result = vec_perm(a, b, perm);
    
    vec_st(result, 0, (vector unsigned int*)g_result);
}
#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call target-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests completed\n");
#endif
    
    test_gcc_vector_shuffle();
    printf("GCC vector shuffle test completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE gather test completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec permute test completed\n");
#endif
    
    /* Compute final checksum */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        final_sum += g_result[i];
    }
    
    printf("Final checksum: %d\n", final_sum + g_checksum);
    
    return 0;
}
