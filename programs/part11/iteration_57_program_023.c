/* Test program to trigger 10-11 operand RTL expansions in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[64] = {0};
volatile int global_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - potentially expands to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Create mask - alternating pattern */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic has many operands when expanded:
     * dest, mask, idx, vec1, vec2
     * Each vector is multiple RTL operands */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Another complex permutation with different sources */
    __m512i idx2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec3 = _mm512_set_epi32(63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48);
    
    /* Complex expression that might require many operands */
    __m512i result2 = _mm512_mask2_permutex2var_epi32(vec1, idx, mask, vec2, vec3);
    
    /* Store results to prevent optimization */
    _mm512_storeu_epi32((void*)global_result, result);
    _mm512_storeu_epi32((void*)(global_result + 8), result2);
}

/* AVX-512 blend with multiple masks and sources */
void test_avx512_complex_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    __mmask16 m1 = 0x5555; /* 01010101... */
    __mmask16 m2 = 0x3333; /* 00110011... */
    __mmask16 m3 = 0x0F0F; /* 00001111... */
    
    /* Complex blend chain - may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_mask_blend_epi32(m3, t1, t2);
    
    /* Add another operation to increase complexity */
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    result = _mm512_permutexvar_epi32(idx, result);
    
    _mm512_storeu_epi32((void*)(global_result + 16), result);
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    const int N = 16;
    int64_t base_array[N] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150};
    int64_t offset_array[N] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int64_t result_array[N] = {0};
    
    svbool_t pg = svwhilelt_b64(0, N);
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* svld1_gather_s64 has many operands: predicate, base, offsets */
    svint64_t gathered = svld1_gather_s64(pg, base_array, offsets);
    
    svst1_s64(pg, result_array, gathered);
    
    /* Copy to global */
    for (int i = 0; i < N; i++) {
        global_result[i + 24] = result_array[i];
    }
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[16];
    for (int i = 0; i < 16; i++) {
        idx_array[i] = (i * 3) % 32; /* Non-linear pattern */
    }
    
    /* Complex shuffle expression */
    v16si result;
    for (int i = 0; i < 16; i++) {
        int idx = idx_array[i];
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Another complex operation: blend based on condition */
    v16si mask = a > 7;
    v16si c = {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
    
    /* This complex expression may expand to many RTL operands */
    v16si blended;
    for (int i = 0; i < 16; i++) {
        blended[i] = mask[i] ? (a[i] + b[i]) : (c[i] - a[i]);
    }
    
    /* Store to global */
    for (int i = 0; i < 16; i++) {
        global_result[i + 40] = result[i];
        global_result[i + 56] = blended[i];
    }
}

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int v1 = {1,2,3,4};
    vector signed int v2 = {5,6,7,8};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
    
    /* vec_perm with three vector arguments */
    vector signed int result = vec_perm(v1, v2, perm);
    
    /* Complex expression with multiple operations */
    vector signed int v3 = {9,10,11,12};
    vector signed int v4 = {13,14,15,16};
    
    vector signed int t1 = vec_add(v1, v2);
    vector signed int t2 = vec_sub(v3, v4);
    vector signed int final_result = vec_perm(t1, t2, perm);
    
    /* Store results */
    vec_st(result, 0, (vector signed int*)&global_result[48]);
    vec_st(final_result, 0, (vector signed int*)&global_result[52]);
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansions...\n");
    
    /* Call architecture-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_complex_blend();
    printf("AVX-512 tests executed\n");
#endif
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE tests executed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec tests executed\n");
#endif
    
    /* Always run GCC vector extension test */
    test_gcc_vector_shuffle();
    printf("GCC vector tests executed\n");
    
    /* Compute checksum to ensure all operations are executed */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += global_result[i];
    }
    global_checksum = checksum;
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
