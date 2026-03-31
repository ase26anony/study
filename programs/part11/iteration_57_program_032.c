/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[64] = {0};
volatile int global_checksum = 0;

/* ========== AVX-512 Implementation ========== */
#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with mask - may expand to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Create mask with alternating bits */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    /* This intrinsic takes 5 arguments but expands to many operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_epi32((void*)global_result, result);
    
    /* Compute simple checksum */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += global_result[i];
    }
    global_checksum += sum;
}

/* Test AVX-512 blend with multiple masks - creates complex expression */
void test_avx512_complex_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    __mmask16 m1 = 0x5555;  /* 0101010101010101 */
    __mmask16 m2 = 0x3333;  /* 0011001100110011 */
    
    /* Complex blend expression that may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_mask_blend_epi32(m1 & m2, t1, t2);
    
    _mm512_storeu_epi32((void*)(global_result + 16), result);
}

#endif /* __AVX512F__ */

/* ========== GCC Vector Extensions (Portable) ========== */
/* Use GCC's native vector extensions for vec_perm expansion */
typedef int32_t v16si __attribute__((vector_size(64)));

/* Global index array to prevent constant folding */
volatile int perm_indices[16] = {
    0, 16, 1, 17, 2, 18, 3, 19,
    4, 20, 5, 21, 6, 22, 7, 23
};

void test_gcc_vector_shuffle(void) {
    /* Initialize vectors */
    v16si va = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si vb = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Create index vector from volatile array */
    v16si idx;
    for (int i = 0; i < 16; i++) {
        idx[i] = perm_indices[i];
    }
    
    /* Use __builtin_shuffle with variable indices - forces vec_perm expansion */
    v16si result = __builtin_shuffle(va, vb, idx);
    
    /* Store result */
    memcpy((void*)(global_result + 32), &result, sizeof(result));
    
    /* Update checksum */
    for (int i = 0; i < 16; i++) {
        global_checksum += ((int32_t*)global_result)[32 + i];
    }
}

/* ========== ARM SVE Implementation ========== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    const int N = 16;
    int64_t base_array[64];
    int64_t offset_array[64];
    int64_t result_array[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        base_array[i] = i * 100;
        offset_array[i] = i * 8;
    }
    
    /* Create SVE vectors - actual implementation depends on SVE vector length */
    svbool_t pg = svwhilelt_b64(0, N);
    svint64_t base = svld1(pg, &base_array[0]);
    svint64_t offsets = svld1(pg, &offset_array[0]);
    
    /* Gather operation - expands to many operands */
    svint64_t gathered = svld1_gather_offset(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1(pg, &result_array[0], gathered);
    
    /* Copy to global */
    for (int i = 0; i < N; i++) {
        global_result[48 + i] = result_array[i];
    }
}
#endif /* __ARM_FEATURE_SVE */

/* ========== PowerPC Altivec ========== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    /* vec_perm with three vectors can expand to many operands */
    vector signed int va = {0,1,2,3};
    vector signed int vb = {4,5,6,7};
    vector unsigned char vc = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    
    /* Complex permutation expression */
    vector signed int result = vec_perm(va, vb, vc);
    
    /* Store result */
    memcpy((void*)(global_result + 56), &result, sizeof(result));
}
#endif /* __ALTIVEC__ */

/* ========== Main Function ========== */
int main(int argc, char **argv) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call target-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_complex_blend();
    printf("AVX-512 tests executed\n");
#endif
    
    test_gcc_vector_shuffle();
    printf("GCC vector shuffle test executed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE gather test executed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec permute test executed\n");
#endif
    
    /* Final checksum computation */
    int final_sum = global_checksum;
    for (int i = 0; i < 64; i++) {
        final_sum += global_result[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("Test completed successfully\n");
    
    return 0;
}
