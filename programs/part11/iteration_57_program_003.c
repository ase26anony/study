/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_results[64] = {0};
volatile int global_index = 0;

/* ========== AVX-512 Implementation ========== */
#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noinline))
void test_avx512_permute(void) {
    /* Create 10+ operand permutation operation */
    __m512i src1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i src2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* Alternating bits pattern */
    
    /* This intrinsic expands to many operands:
     * dest, mask, idx, src1, src2 = 5 explicit operands
     * But RTL expansion adds more for mask, etc.
     */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Store to prevent optimization */
    _mm512_storeu_si512((void*)&global_results[global_index], result);
    global_index += 8;
}

/* Another AVX-512 test with blend operation */
__attribute__((noinline))
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    __m512i d = _mm512_set_epi64(31,30,29,28,27,26,25,24);
    __mmask8 mask1 = 0xAA;
    __mmask8 mask2 = 0x55;
    
    /* Complex blend that might expand to many operands */
    __m512i ab = _mm512_mask_blend_epi64(mask1, a, b);
    __m512i cd = _mm512_mask_blend_epi64(mask2, c, d);
    __m512i result = _mm512_mask_blend_epi64(mask1 ^ mask2, ab, cd);
    
    _mm512_storeu_si512((void*)&global_results[global_index], result);
    global_index += 8;
}
#endif

/* ========== ARM SVE Implementation ========== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

__attribute__((noinline))
void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    svint64_t base = svdup_s64(100);
    svint64_t offsets = svindex_s64(0, 1);
    svbool_t pg = svptrue_b64();
    
    /* Gather operation with multiple vector operands */
    svint64_t data = svld1_gather_s64index_s64(pg, (const int64_t*)global_results, offsets);
    
    /* Store result */
    svst1_s64(pg, &global_results[global_index], data);
    global_index += svcntd();
}

__attribute__((noinline))
void test_sve_permute(void) {
    svint64_t a = svdup_s64(1);
    svint64_t b = svdup_s64(2);
    svint64_t c = svdup_s64(3);
    svbool_t pg = svptrue_b64();
    
    /* Complex permutation pattern */
    svint64_t tmp1 = svsel_s64(pg, a, b);
    svint64_t tmp2 = svsel_s64(pg, b, c);
    svint64_t result = svsel_s64(pg, tmp1, tmp2);
    
    svst1_s64(pg, &global_results[global_index], result);
    global_index += svcntd();
}
#endif

/* ========== GCC Vector Extensions (Portable) ========== */
/* Using GCC's native vector extensions for vec_perm operation */
typedef int64_t v4di __attribute__((vector_size(32)));

__attribute__((noinline))
v4di test_gcc_vector_shuffle(v4di a, v4di b, v4di mask) {
    /* This should expand to vec_perm with many operands */
    v4di result;
    
    /* Complex shuffle using GCC builtin */
    int64_t idx_array[8] = {0, 4, 1, 5, 2, 6, 3, 7};
    
    /* Force variable indices to prevent constant folding */
    volatile int start = 0;
    for (int i = 0; i < 4; i++) {
        idx_array[i] = start + i;
        idx_array[i + 4] = start + i + 4;
    }
    
    /* Create a permutation using __builtin_shuffle */
    result = __builtin_shuffle(a, b, (v4di){idx_array[0], idx_array[1], 
                                            idx_array[2], idx_array[3]});
    
    /* Additional operation to increase complexity */
    result = result + mask;
    
    return result;
}

/* ========== PowerPC Altivec/VSX ========== */
#ifdef __ALTIVEC__
#include <altivec.h>

__attribute__((noinline))
void test_altivec_permute(void) {
    vector signed long long a = {1, 2, 3, 4};
    vector signed long long b = {5, 6, 7, 8};
    vector unsigned char perm = {0,1,2,3, 16,17,18,19, 4,5,6,7, 20,21,22,23};
    
    /* vec_perm with 3 vector arguments */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_st(result, 0, (vector signed long long*)&global_results[global_index]);
    global_index += 4;
}
#endif

/* ========== Main Test Driver ========== */
int main(void) {
    int checksum = 0;
    
    /* Initialize global results array */
    for (int i = 0; i < 64; i++) {
        global_results[i] = i;
    }
    
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    test_avx512_permute();
    test_avx512_blend();
#endif
    
    /* Test ARM SVE if available */
#ifdef __ARM_FEATURE_SVE
    printf("Running ARM SVE tests...\n");
    test_sve_gather();
    test_sve_permute();
#endif
    
    /* Test GCC vector extensions */
    printf("Running GCC vector extension tests...\n");
    v4di vec_a = {1, 2, 3, 4};
    v4di vec_b = {5, 6, 7, 8};
    v4di vec_mask = {0, 1, 0, 1};
    v4di vec_result = test_gcc_vector_shuffle(vec_a, vec_b, vec_mask);
    
    /* Store GCC vector result */
    memcpy((void*)&global_results[global_index], &vec_result, sizeof(vec_result));
    global_index += 4;
    
    /* Test PowerPC Altivec if available */
#ifdef __ALTIVEC__
    printf("Running PowerPC Altivec tests...\n");
    test_altivec_permute();
#endif
    
    /* Compute checksum of all results */
    for (int i = 0; i < 64; i++) {
        checksum += (int)global_results[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
