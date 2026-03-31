/* Test program to cover 10-11 operand RTL expansion cases in optabs.cc */
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
    /* Initialize vectors with test data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Use volatile to prevent constant folding */
    volatile __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
    
    /* This intrinsic takes: dest, mask, idx, vec1, vec2 = 5 explicit args
       but expands to many more operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store result to prevent dead code elimination */
    _mm512_storeu_si512((void*)&global_results[result_index], result);
    result_index += 8; /* 512 bits = 8 int64_t */
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    __m512i d = _mm512_set_epi64(31,30,29,28,27,26,25,24);
    
    volatile __mmask8 mask1 = 0xAA; /* 10101010 */
    volatile __mmask8 mask2 = 0xCC; /* 11001100 */
    
    /* Complex blend operation that may expand to many operands */
    __m512i ab = _mm512_mask_blend_epi64(mask1, a, b);
    __m512i cd = _mm512_mask_blend_epi64(mask2, c, d);
    __m512i result = _mm512_mask_blend_epi64(0xF0, ab, cd);
    
    _mm512_storeu_si512((void*)&global_results[result_index], result);
    result_index += 8;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* SVE gather operation with many operands */
void test_sve_gather(void) {
    /* Use variable length to prevent optimization */
    volatile int n = 256;
    int64_t base_array[256];
    int64_t offset_array[256];
    int64_t result_array[256];
    
    for (int i = 0; i < 256; i++) {
        base_array[i] = i * 8;
        offset_array[i] = (i % 16) * 8;
    }
    
    /* SVE gather with predicate - expands to many operands */
    svbool_t pg = svwhilelt_b64(0, n);
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* This gather operation takes base, offsets, predicate = 3 args
       but expands to many more in RTL */
    svint64_t gathered = svld1_gather_s64(pg, base_array, offsets);
    
    svst1_s64(pg, result_array, gathered);
    
    /* Store to global */
    for (int i = 0; i < 16 && result_index < 240; i++) {
        global_results[result_index++] = result_array[i];
    }
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */

/* Portable GCC vector extensions for vec_perm operation */
typedef int64_t v4di __attribute__((vector_size(32)));

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v4di a = {0, 1, 2, 3};
    v4di b = {4, 5, 6, 7};
    
    /* Variable indices to prevent constant folding */
    volatile int idx0 = 0, idx1 = 5, idx2 = 2, idx3 = 7;
    
    /* Use __builtin_shuffle with variable indices */
    v4di result = __builtin_shufflevector(a, b, idx0, idx1, idx2, idx3);
    
    /* Store result */
    for (int i = 0; i < 4 && result_index < 252; i++) {
        global_results[result_index++] = result[i];
    }
}

/* More complex permutation with three input vectors */
void test_gcc_complex_permute(void) {
    typedef int32_t v8si __attribute__((vector_size(32)));
    
    v8si v1 = {0, 1, 2, 3, 4, 5, 6, 7};
    v8si v2 = {8, 9, 10, 11, 12, 13, 14, 15};
    v8si v3 = {16, 17, 18, 19, 20, 21, 22, 23};
    
    /* Create a complex expression that might expand to many operands */
    v8si temp = __builtin_shufflevector(v1, v2, 0, 9, 2, 11, 4, 13, 6, 15);
    v8si result = __builtin_shufflevector(temp, v3, 0, 1, 10, 11, 4, 5, 14, 15);
    
    /* Store as int64_t for consistency */
    int64_t* res_ptr = (int64_t*)&result;
    for (int i = 0; i < 4 && result_index < 248; i++) {
        global_results[result_index++] = res_ptr[i];
    }
}

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0, 1, 2, 3};
    vector signed int b = {4, 5, 6, 7};
    vector unsigned char perm = {0,1,2,3, 16,17,18,19, 8,9,10,11, 24,25,26,27};
    
    /* vec_perm with three vectors expands to many operands */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Store result */
    int64_t* res_ptr = (int64_t*)&result;
    for (int i = 0; i < 2 && result_index < 254; i++) {
        global_results[result_index++] = res_ptr[i];
    }
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Reset global index */
    result_index = 0;
    
    /* Call architecture-specific tests if supported */
#ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    test_avx512_permute();
    test_avx512_blend();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Running ARM SVE tests...\n");
    test_sve_gather();
#endif
    
#ifdef __ALTIVEC__
    printf("Running PowerPC Altivec tests...\n");
    test_altivec_permute();
#endif
    
    /* Always run GCC vector tests (portable) */
    printf("Running GCC vector tests...\n");
    test_gcc_vector_shuffle();
    test_gcc_complex_permute();
    
    /* Compute checksum to ensure all operations were executed */
    int64_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_results[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    printf("Result index: %d\n", result_index);
    
    return 0;
}
