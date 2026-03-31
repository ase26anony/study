/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - potentially expands to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Create mask - alternating pattern */
    __mmask16 mask = 0xAAAA;  /* 0b1010101010101010 */
    
    /* This intrinsic takes 5 arguments but expands to many operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_si512((void*)global_result, result);
    
    /* Compute simple checksum */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += global_result[i];
    }
    checksum += sum;
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    __m512i d = _mm512_set_epi64(31,30,29,28,27,26,25,24);
    
    /* Complex expression that may expand to many operands */
    __m512i temp1 = _mm512_add_epi64(a, b);
    __m512i temp2 = _mm512_sub_epi64(c, d);
    __m512i mask = _mm512_set1_epi64(1);
    
    /* Conditional blend - may generate complex RTL */
    __m512i result = _mm512_mask_blend_epi64(0xAA, temp1, temp2);
    
    _mm512_storeu_si512((void*)global_result, result);
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    int64_t base_array[32];
    int64_t offset_array[32];
    
    for (int i = 0; i < 32; i++) {
        base_array[i] = i * 100;
        offset_array[i] = i * 8;
    }
    
    svbool_t pg = svptrue_b64();
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* Gather operation - potentially expands to many operands */
    svint64_t result = svld1_gather_s64(pg, base_array, offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)global_result, result);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int64_t v4di __attribute__((vector_size(32)));  /* 256-bit vector */
typedef int64_t v2di __attribute__((vector_size(16)));  /* 128-bit vector */

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v4di a = {0, 1, 2, 3};
    v4di b = {4, 5, 6, 7};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[8] = {0, 5, 2, 7, 1, 4, 3, 6};
    
    /* Create index vector from array */
    v4di indices = {
        idx_array[0], idx_array[1], idx_array[2], idx_array[3]
    };
    
    /* Complex expression combining multiple operations */
    v4di temp1 = a + b;
    v4di temp2 = a - b;
    v4di temp3 = a * b;
    
    /* This complex expression may expand to many RTL operands */
    v4di result = (temp1 & temp2) | (temp3 & ~temp1);
    
    /* Store to global */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Test with __builtin_shufflevector */
void test_builtin_shuffle(void) {
    v2di v1 = {0, 1};
    v2di v2 = {2, 3};
    v2di v3 = {4, 5};
    v2di v4 = {6, 7};
    
    /* Use volatile to prevent constant folding */
    volatile int i0 = 0, i1 = 2, i2 = 4, i3 = 6;
    
    /* Complex shuffle expression - may require many operands */
    v2di shuffled1 = __builtin_shufflevector(v1, v2, i0, i1);
    v2di shuffled2 = __builtin_shufflevector(v3, v4, i2, i3);
    
    v2di result = shuffled1 + shuffled2;
    
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0, 1, 2, 3};
    vector signed int b = {4, 5, 6, 7};
    vector unsigned char perm = {0,1,2,3, 16,17,18,19, 4,5,6,7, 20,21,22,23};
    
    /* vec_perm with three vectors may expand to many operands */
    vector signed int result = vec_perm(a, b, perm);
    
    vec_st(result, 0, (vector signed int*)global_result);
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Initialize global array */
    for (int i = 0; i < 16; i++) {
        global_result[i] = 0;
    }
    
    /* Call architecture-specific tests */
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
    
    /* Always run GCC vector tests */
    printf("Running GCC vector tests...\n");
    test_gcc_vector_shuffle();
    test_builtin_shuffle();
    
    /* Compute final checksum */
    int final_checksum = 0;
    for (int i = 0; i < 16; i++) {
        final_checksum += global_result[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Test completed.\n");
    
    return 0;
}
