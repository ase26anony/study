/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with many operands */
void test_avx512_permute(void) {
    /* Create 10+ operands for permutation */
    __m512i src1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i src2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* Alternating bits */
    
    /* This intrinsic expands to many operands:
       dest, mask, idx, src1, src2 = 5 explicit operands
       But RTL expansion adds more for mask, index, etc. */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src1, src2);
    
    /* Another complex permutation with blending */
    __m512i src3 = _mm512_set_epi32(63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48);
    __m512i idx2 = _mm512_set_epi32(16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
    __mmask16 mask2 = 0x5555;
    
    /* Chain operations to create complex expression */
    __m512i temp = _mm512_mask_permutex2var_epi32(src2, mask2, idx2, src3, src1);
    result = _mm512_mask_add_epi32(result, mask, result, temp);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_si512((void*)global_result, result);
    
    /* Compute checksum */
    for (int i = 0; i < 16; i++) {
        checksum += global_result[i];
    }
}

/* AVX-512 gather with many operands */
void test_avx512_gather(void) {
#ifdef __AVX512VL__
    int base[64];
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    __m512i vindex = _mm512_set_epi32(0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60);
    __mmask16 mask = 0xFFFF;
    __m512i src = _mm512_set1_epi32(999);
    int scale = 4;
    
    /* Gather with mask, base, vindex, scale, src - expands to many operands */
    __m512i result = _mm512_mask_i32gather_epi32(src, mask, vindex, base, scale);
    
    _mm512_storeu_si512((void*)(global_result + 8), result);
    
    for (int i = 0; i < 16; i++) {
        checksum += global_result[8 + i];
    }
#endif
}

#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector permutation using GCC extensions */
void test_gcc_vector_shuffle(void) {
    /* Define 256-bit vector type (8x int32) */
    typedef int v8si __attribute__((vector_size(32)));
    
    /* Initialize vectors */
    v8si a = {0,1,2,3,4,5,6,7};
    v8si b = {8,9,10,11,12,13,14,15};
    v8si c = {16,17,18,19,20,21,22,23};
    
    /* Variable indices to prevent constant folding */
    volatile int idx_array[16] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15};
    
    /* Complex permutation expression that may expand to many operands */
    v8si result;
    
    /* Manual shuffle with variable indices - forces general permute expansion */
    for (int i = 0; i < 8; i++) {
        int idx = idx_array[i];
        if (idx < 8) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 8];
        }
    }
    
    /* Blend operation that creates complex RTL */
    v8si mask = {0, -1, 0, -1, 0, -1, 0, -1}; /* Alternating select */
    v8si blended = (mask & result) | (~mask & c);
    
    /* Store result */
    memcpy((void*)global_result, &blended, sizeof(blended));
    
    for (int i = 0; i < 8; i++) {
        checksum += global_result[i];
    }
}

/* More complex vector expression */
void test_gcc_complex_expr(void) {
    typedef float v8sf __attribute__((vector_size(32)));
    typedef int v8si __attribute__((vector_size(32)));
    
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    v8sf v3 = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f};
    v8sf v4 = {9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f};
    
    /* Complex fused multiply-add like expression */
    v8sf result = v1 * v2 + v3 * v4;
    
    /* Add conditional blending */
    v8si mask = v1 > v2;
    v8sf blended = __builtin_shuffle(result, v4, 
        (v8si){0,9,2,11,4,13,6,15});  /* Interleave */
    
    memcpy((void*)(global_result + 8), &blended, sizeof(blended));
    
    for (int i = 0; i < 8; i++) {
        checksum += (int)global_result[8 + i];
    }
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather with predicate, base, offset - many operands */
    int64_t base_array[100];
    for (int i = 0; i < 100; i++) base_array[i] = i * 3;
    
    svbool_t pg = svptrue_b64();
    svint64_t offsets = svindex_s64(0, 4);  /* 0, 4, 8, ... */
    
    /* Gather operation with multiple vector operands */
    svint64_t gathered = svld1_gather_s64index_s64(pg, base_array, offsets);
    
    /* Store to global */
    svst1_s64(pg, (int64_t*)global_result, gathered);
    
    /* Compute checksum */
    for (int i = 0; i < svcntd(); i++) {
        checksum += global_result[i];
    }
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    /* vec_perm takes 3 vectors and returns permuted result */
    vector signed int v1 = {1,2,3,4};
    vector signed int v2 = {5,6,7,8};
    vector signed char perm = {0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
    
    /* Multiple vec_perm operations in complex expression */
    vector signed int result = vec_perm(v1, v2, perm);
    
    /* Blend with another operation */
    vector signed int v3 = {9,10,11,12};
    vector signed int mask = vec_cmpeq(v1, v2);  /* All false */
    vector signed int blended = vec_sel(result, v3, mask);
    
    /* Store result */
    memcpy((void*)global_result, &blended, sizeof(blended));
    
    for (int i = 0; i < 4; i++) {
        checksum += global_result[i];
    }
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(int argc, char **argv) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Reset checksum */
    checksum = 0;
    
    /* Test GCC vector extensions (most portable) */
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    
#ifdef __AVX512F__
    printf("Testing AVX-512 permutations...\n");
    test_avx512_permute();
#ifdef __AVX512VL__
    test_avx512_gather();
#endif
#endif

#ifdef __ARM_FEATURE_SVE
    printf("Testing SVE gather...\n");
    test_sve_gather();
#endif

#ifdef __ALTIVEC__
    printf("Testing Altivec permute...\n");
    test_altivec_permute();
#endif

    printf("Final checksum: %d\n", checksum);
    
    /* Use result to prevent optimization */
    if (checksum > 1000000) {
        printf("Unexpected large checksum\n");
    }
    
    return 0;
}
