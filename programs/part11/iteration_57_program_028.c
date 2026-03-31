/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
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
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx  = _mm512_set_epi32(1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31);
    
    /* Use a mask - this creates more operands */
    __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
    
    /* This intrinsic takes 5 arguments but may expand to more operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Another complex permutation with more operands */
    __m512i idx2 = _mm512_set_epi32(0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30);
    __mmask16 mask2 = 0x5555; /* 0101010101010101 pattern */
    
    /* Chain operations to create complex expressions */
    __m512i temp = _mm512_mask_permutex2var_epi32(vec2, mask2, idx2, vec2, vec1);
    
    /* Blend operation that might expand further */
    __m512i final = _mm512_mask_blend_epi32(0xFFFF, result, temp);
    
    /* Store to volatile global to prevent optimization */
    _mm512_store_epi64((void*)g_result, final);
    
    /* Compute simple checksum */
    for (int i = 0; i < 8; i++) {
        g_checksum += (int)g_result[i];
    }
}

/* AVX-512 gather with multiple operands */
void test_avx512_gather(void) {
    int64_t base_array[32];
    for (int i = 0; i < 32; i++) {
        base_array[i] = i * 2;
    }
    
    __m512i vindex = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __mmask8 mask = 0xFF;
    
    /* Gather with scale - may expand to many operands */
    __m512i gathered = _mm512_mask_i64gather_epi64(_mm512_setzero_si512(), mask, vindex,
                                                  (void*)base_array, 8);
    
    _mm512_store_epi64((void*)g_result, gathered);
    
    for (int i = 0; i < 8; i++) {
        g_checksum += (int)g_result[i];
    }
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector extensions that may trigger vec_perm expansion */

typedef int32_t v4si __attribute__((vector_size(16)));
typedef int64_t v2di __attribute__((vector_size(16)));

/* Complex shuffle with variable indices - may expand to many operands */
void test_gcc_vector_shuffle(void) {
    volatile v4si a = {1, 2, 3, 4};
    volatile v4si b = {5, 6, 7, 8};
    
    /* Variable index array prevents constant folding */
    int idx_array[4];
    for (int i = 0; i < 4; i++) {
        idx_array[i] = i * 2;
    }
    
    /* Create index vector from array */
    v4si idx = {idx_array[0], idx_array[1], idx_array[2], idx_array[3]};
    
    /* Use __builtin_shuffle with variable indices - may expand to vec_perm */
    v4si result = __builtin_shuffle(a, b, idx);
    
    /* Store result */
    int32_t* res_ptr = (int32_t*)&result;
    for (int i = 0; i < 4; i++) {
        g_checksum += res_ptr[i];
    }
}

/* More complex permutation with three input vectors */
void test_gcc_complex_permute(void) {
    typedef int32_t v8si __attribute__((vector_size(32)));
    
    volatile v8si v1 = {1,2,3,4,5,6,7,8};
    volatile v8si v2 = {9,10,11,12,13,14,15,16};
    volatile v8si v3 = {17,18,19,20,21,22,23,24};
    
    /* Complex expression that might expand to many operands */
    v8si temp1 = __builtin_shufflevector(v1, v2, 0,9,2,11,4,13,6,15);
    v8si temp2 = __builtin_shufflevector(v2, v3, 0,9,2,11,4,13,6,15);
    
    /* Blend-like operation */
    v8si mask = {0, -1, 0, -1, 0, -1, 0, -1};
    v8si result = (temp1 & ~mask) | (temp2 & mask);
    
    int32_t* res_ptr = (int32_t*)&result;
    for (int i = 0; i < 8; i++) {
        g_checksum += res_ptr[i];
    }
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
    
    /* Gather with multiple vector arguments */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, &base_array[0], offsets);
    
    /* Store results */
    int64_t temp[svcntd()];
    svst1_s64(pg, temp, gathered);
    
    for (size_t i = 0; i < svcntd(); i++) {
        g_checksum += (int)temp[i];
    }
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {1,2,3,4};
    vector signed int b = {5,6,7,8};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vectors - may expand to many operands */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Complex expression */
    vector signed int mask = {0, -1, 0, -1};
    vector signed int blended = vec_sel(a, result, mask);
    
    int* res_ptr = (int*)&blended;
    for (int i = 0; i < 4; i++) {
        g_checksum += res_ptr[i];
    }
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    g_checksum = 0;
    
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Test portable GCC vector extensions */
    test_gcc_vector_shuffle();
    test_gcc_complex_permute();
    
    /* Test architecture-specific implementations */
#ifdef __AVX512F__
    printf("Testing AVX-512...\n");
    test_avx512_permute();
    test_avx512_gather();
#endif

#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE...\n");
    test_sve_gather();
#endif

#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec...\n");
    test_altivec_permute();
#endif
    
    printf("Final checksum: %d\n", g_checksum);
    
    return 0;
}
