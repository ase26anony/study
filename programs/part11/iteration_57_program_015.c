/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_results[1024];
volatile int result_index = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permutex2var with mask - can generate many operands */
__attribute__((noinline))
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    /* Create index vector - alternating between vec1 and vec2 */
    __m512i idx = _mm512_set_epi32(0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23);
    
    /* Create mask - alternating pattern */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic can expand to many operands:
       dest, mask, idx, vec1, vec2 = 5 vector operands
       When expanded to RTL, each vector becomes multiple operands */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Force usage and prevent optimization */
    _mm512_store_epi32((void*)&global_results[result_index], result);
    result_index += 16;
}

/* Another AVX-512 test with blend operations */
__attribute__((noinline))
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    __m512i d = _mm512_set_epi64(31,30,29,28,27,26,25,24);
    
    /* Complex blend expression that might expand to many operands */
    __m512i mask1 = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFF);
    __m512i mask2 = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAA);
    
    /* This complex expression may require many operands when expanded */
    __m512i temp1 = _mm512_and_si512(mask1, a);
    __m512i temp2 = _mm512_andnot_si512(mask1, b);
    __m512i temp3 = _mm512_and_si512(mask2, c);
    __m512i temp4 = _mm512_andnot_si512(mask2, d);
    
    __m512i result = _mm512_or_si512(
        _mm512_or_si512(temp1, temp2),
        _mm512_or_si512(temp3, temp4)
    );
    
    _mm512_store_epi64((void*)&global_results[result_index], result);
    result_index += 8;
}

#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector implementation using GCC extensions */

typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
__attribute__((noinline))
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[16] = {
        0,16,1,17,2,18,3,19,
        4,20,5,21,6,22,7,23
    };
    
    /* Build shuffle result manually - GCC may optimize to vec_perm */
    v16si result;
    for (int i = 0; i < 16; i++) {
        int idx = idx_array[i];
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Store to prevent optimization */
    memcpy((void*)&global_results[result_index], &result, sizeof(result));
    result_index += 16;
}

/* More complex vector expression */
__attribute__((noinline))
void test_gcc_complex_expr(void) {
    v8di v1 = {0,1,2,3,4,5,6,7};
    v8di v2 = {8,9,10,11,12,13,14,15};
    v8di v3 = {16,17,18,19,20,21,22,23};
    v8di v4 = {24,25,26,27,28,29,30,31};
    
    /* Complex expression that might expand to many operands */
    v8di mask1 = (v8di){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    v8di mask2 = (v8di){0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
    
    v8di result = (mask1 & v1) | (~mask1 & mask2 & v2) | 
                  (~mask1 & ~mask2 & v3) | (mask1 & mask2 & v4);
    
    memcpy((void*)&global_results[result_index], &result, sizeof(result));
    result_index += 8;
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* SVE gather operation with multiple vectors */
__attribute__((noinline))
void test_sve_gather(void) {
    /* Create base array */
    int64_t base_array[100];
    for (int i = 0; i < 100; i++) {
        base_array[i] = i * 2;
    }
    
    /* Create offset vector */
    svint64_t offsets = svindex_s64(0, 1);
    
    /* Create predicate */
    svbool_t pg = svptrue_b64();
    
    /* Gather operation - may expand to many operands */
    svint64_t result = svld1_gather_s64index_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)&global_results[result_index], result);
    result_index += svcntd();
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand instruction expansion...\n");
    
    /* Call all available test functions */
#ifdef __AVX512F__
    printf("Testing AVX-512 permutations...\n");
    test_avx512_permute();
    test_avx512_blend();
#endif
    
    printf("Testing GCC vector extensions...\n");
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE gather...\n");
    test_sve_gather();
#endif
    
    /* Compute checksum to ensure all operations were performed */
    int64_t checksum = 0;
    for (int i = 0; i < result_index && i < 1024; i++) {
        checksum += global_results[i];
    }
    
    printf("Result checksum: %ld\n", (long)checksum);
    printf("Total elements processed: %d\n", result_index);
    
    return 0;
}
