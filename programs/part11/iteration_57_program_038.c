/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[16] = {0};
volatile int g_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test function using _mm512_mask_permutex2var_epi32 - expands to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential values */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    /* Index vector - controls which elements to select */
    __m512i idx = _mm512_set_epi32(
        0, 16, 1, 17, 2, 18, 3, 19,
        4, 20, 5, 21, 6, 22, 7, 23
    );
    
    /* Mask - controls which elements to write */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic expands to many operands in RTL:
     * dest, mask, idx, vec1, vec2 = 5 explicit operands
     * But during RTL expansion, each vector register becomes multiple operands
     * Total often exceeds 10 operands */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_epi32((void*)g_result, result);
    
    /* Also test the 64-bit version */
    __m512i vec1_64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i vec2_64 = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i idx_64 = _mm512_set_epi64(0,8,1,9,2,10,3,11);
    __mmask8 mask_64 = 0xAA;
    
    __m512i result2 = _mm512_mask_permutex2var_epi64(vec1_64, mask_64, idx_64, vec2_64, vec1_64);
    _mm512_storeu_epi64((void*)(g_result + 8), result2);
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    /* Complex blend expression that may expand to many operands */
    __mmask16 m1 = 0x5555;
    __mmask16 m2 = 0x3333;
    
    /* This should create a complex RTL expression */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_add_epi32(t1, t2);
    
    _mm512_storeu_epi32((void*)g_result, result);
}

#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation using GCC extensions */

/* Define vector types */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Test function using GCC vector shuffle with variable indices */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array - prevents constant folding */
    static volatile int indices[16] = {
        0,16,1,17,2,18,3,19,
        4,20,5,21,6,22,7,23
    };
    
    /* Create index vector from array */
    v16si idx = {
        indices[0], indices[1], indices[2], indices[3],
        indices[4], indices[5], indices[6], indices[7],
        indices[8], indices[9], indices[10], indices[11],
        indices[12], indices[13], indices[14], indices[15]
    };
    
    /* This shuffle operation with variable indices may expand to 
     * a vec_perm RTL pattern with many operands */
    v16si result;
    
    /* Manual permutation - GCC may optimize this to vec_perm */
    for (int i = 0; i < 16; i++) {
        int index = idx[i];
        if (index < 16) {
            result[i] = a[index];
        } else {
            result[i] = b[index - 16];
        }
    }
    
    /* Store result */
    memcpy((void*)g_result, &result, sizeof(result));
}

/* Another GCC vector test with complex expression */
void test_gcc_complex_expr(void) {
    v8di v1 = {0,1,2,3,4,5,6,7};
    v8di v2 = {8,9,10,11,12,13,14,15};
    v8di v3 = {16,17,18,19,20,21,22,23};
    v8di v4 = {24,25,26,27,28,29,30,31};
    
    /* Complex expression that may generate many operands */
    v8di mask1 = {0, -1, 0, -1, 0, -1, 0, -1};
    v8di mask2 = {0, 0, -1, -1, 0, 0, -1, -1};
    
    /* This complex blend expression could expand to many RTL operands:
     * res = (mask1 & v1) | (~mask1 & mask2 & v2) | 
     *       (~mask1 & ~mask2 & v3) | (mask1 & mask2 & v4) */
    v8di not_mask1 = ~mask1;
    v8di not_mask2 = ~mask2;
    
    v8di t1 = mask1 & v1;
    v8di t2 = not_mask1 & mask2 & v2;
    v8di t3 = not_mask1 & not_mask2 & v3;
    v8di t4 = mask1 & mask2 & v4;
    
    v8di result = t1 | t2 | t3 | t4;
    
    memcpy((void*)g_result, &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    int64_t base_array[100];
    for (int i = 0; i < 100; i++) {
        base_array[i] = i;
    }
    
    svbool_t pg = svptrue_b64();
    svint64_t offsets = svindex_s64(0, 1);
    
    /* Gather with base + offsets - expands to many operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)g_result, gathered);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Run all applicable tests */
#ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    test_avx512_permute();
    test_avx512_blend();
#endif
    
    printf("Running GCC vector tests...\n");
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    
#ifdef __ARM_FEATURE_SVE
    printf("Running ARM SVE tests...\n");
    test_sve_gather();
#endif
    
    /* Compute checksum to ensure code isn't optimized away */
    for (int i = 0; i < 16; i++) {
        g_checksum += (int)g_result[i];
    }
    
    printf("Checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
