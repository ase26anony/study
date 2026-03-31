/* Test program to cover 10-11 operand RTL expansion in optabs.cc */
/* Compile with: -O2 -mavx512f -mavx512vl -ftree-vectorize for AVX-512 */
/* Or: -O2 -march=armv8-a+sve for AArch64 SVE */
/* Or: -O2 -ftree-vectorize for generic vector extensions */

#include <stdint.h>
#include <stdio.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[8] = {0};
volatile int global_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test function using _mm512_mask_permutex2var_epi32 - expands to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx  = _mm512_set_epi32(30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0);
    
    /* Create mask - alternating pattern */
    __mmask16 mask = 0xAAAA;  /* 0b1010101010101010 */
    
    /* This intrinsic should expand to RTL with many operands:
     * dest, mask, idx, vec1, vec2 = 5 explicit operands
     * But during RTL expansion, each may be broken into multiple parts
     */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Another complex permutation with more operands */
    __m512i idx2 = _mm512_set_epi32(1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31);
    __m512i result2 = _mm512_mask_permutex2var_epi32(result, ~mask, idx2, vec2, result);
    
    /* Store results to prevent optimization */
    _mm512_storeu_si512((void*)global_result, result2);
    
    /* Compute simple checksum */
    int32_t* ptr = (int32_t*)&result2;
    for (int i = 0; i < 16; i++) {
        global_checksum += ptr[i];
    }
}

/* AVX-512 blend with multiple masks - creates complex expression */
void test_avx512_complex_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    __mmask16 m1 = 0x5555;  /* 01010101... */
    __mmask16 m2 = 0x3333;  /* 00110011... */
    
    /* Complex blend expression that may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_mask_blend_epi32(m1 & m2, t1, t2);
    
    /* Force use of result */
    _mm512_storeu_si512((void*)(global_result + 4), result);
}

#endif /* __AVX512F__ */

/* ==================== AArch64 SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    const int N = 16;
    int64_t base_array[N] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int64_t offset_array[N] = {0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15};
    int64_t result_array[N] = {0};
    
    svbool_t pg = svwhilelt_b64(0, N);
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* Gather with predicate, base, and offsets - expands to many operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, base_array, offsets);
    
    /* Store result */
    svst1_s64(pg, result_array, gathered);
    
    /* Update checksum */
    for (int i = 0; i < N; i++) {
        global_checksum += result_array[i];
    }
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== Generic GCC Vector Extensions ==================== */

/* Using GCC vector extensions for portable testing */
typedef int32_t v8si __attribute__((vector_size(32)));
typedef int64_t v4di __attribute__((vector_size(32)));

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v8si a = {0,1,2,3,4,5,6,7};
    v8si b = {8,9,10,11,12,13,14,15};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[16] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15};
    
    /* Create complex permutation expression */
    v8si result;
    
    /* Manually construct shuffle with many operands */
    for (int i = 0; i < 8; i++) {
        int idx = idx_array[i];
        if (idx < 8) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 8];
        }
    }
    
    /* Another complex expression combining multiple shuffles */
    v8si shuffled1 = __builtin_shufflevector(a, b, 7,6,5,4,3,2,1,0);
    v8si shuffled2 = __builtin_shufflevector(b, a, 0,1,2,3,4,5,6,7);
    v8si mask = {0, -1, 0, -1, 0, -1, 0, -1};
    
    /* Complex blend operation */
    v8si final_result = (shuffled1 & mask) | (shuffled2 & ~mask);
    
    /* Store to global to prevent optimization */
    int32_t* ptr = (int32_t*)&final_result;
    for (int i = 0; i < 8; i++) {
        global_result[i % 4] += ptr[i];
    }
}

/* Test with 64-bit vectors for different permutation patterns */
void test_gcc_double_shuffle(void) {
    v4di v1 = {0,1,2,3};
    v4di v2 = {4,5,6,7};
    v4di v3 = {8,9,10,11};
    v4di v4 = {12,13,14,15};
    
    /* Complex expression with multiple vector operands */
    v4di t1 = __builtin_shufflevector(v1, v2, 0,2,4,6);
    v4di t2 = __builtin_shufflevector(v3, v4, 1,3,5,7);
    v4di mask = {0, -1, 0, -1};
    
    v4di result = (t1 & ~mask) | (t2 & mask);
    
    /* Use result */
    int64_t* ptr = (int64_t*)&result;
    for (int i = 0; i < 4; i++) {
        global_checksum += ptr[i];
    }
}

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int v1 = {0,1,2,3};
    vector signed int v2 = {4,5,6,7};
    vector signed char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vector arguments */
    vector signed int result = vec_perm(v1, v2, perm);
    
    /* Complex expression with multiple vec_perms */
    vector signed int v3 = {8,9,10,11};
    vector signed int v4 = {12,13,14,15};
    vector signed char perm2 = {4,5,6,7,20,21,22,23,12,13,14,15,28,29,30,31};
    
    vector signed int r1 = vec_perm(v1, v2, perm);
    vector signed int r2 = vec_perm(v3, v4, perm2);
    vector signed int mask_vec = {0, -1, 0, -1};
    
    vector signed int final = vec_sel(r1, r2, mask_vec);
    
    /* Store result */
    vec_st(final, 0, (vector signed int*)global_result);
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call all available test functions */
    
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
    
    /* Always run GCC vector extension tests */
    test_gcc_vector_shuffle();
    test_gcc_double_shuffle();
    printf("GCC vector extension tests executed\n");
    
    /* Final checksum to ensure all computations are used */
    int final_sum = global_checksum;
    for (int i = 0; i < 8; i++) {
        final_sum += global_result[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("Test completed.\n");
    
    return final_sum != 0 ? 0 : 1;
}
