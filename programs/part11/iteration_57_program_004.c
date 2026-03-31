/* Test program to cover 10-11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[64] = {0};
volatile int global_checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

#ifdef __GNUC__
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, int *indices) {
    /* Create a complex permutation pattern */
    v8di result;
    
    /* Use __builtin_shuffle with variable indices - forces runtime permutation */
    /* This may expand to a multi-operand vec_perm RTL */
    for (int i = 0; i < 8; i++) {
        int idx = indices[i];
        if (idx < 8) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 8];
        }
    }
    
    /* Store to volatile global to prevent optimization */
    for (int i = 0; i < 8; i++) {
        global_result[i] = result[i];
    }
}

/* Complex blend operation with multiple vectors */
void test_gcc_complex_blend(v4di a, v4di b, v4di c, v4di d, v4di mask1, v4di mask2) {
    /* Complex expression that may require many operands */
    v4di temp1 = (mask1 & a) | (~mask1 & b);
    v4di temp2 = (mask2 & c) | (~mask2 & d);
    v4di result = temp1 + temp2;
    
    /* Mix with another operation */
    result = result * a - b / (c + 1);
    
    for (int i = 0; i < 4; i++) {
        global_result[8 + i] = result[i];
    }
}
#endif

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask - takes many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has 5 explicit args but expands to many RTL operands */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Additional complex operation to ensure expansion */
    result = _mm512_add_epi32(result, _mm512_slli_epi32(b, 2));
    result = _mm512_and_si512(result, _mm512_set1_epi32(0xFFFFFFFF));
    
    /* Store to prevent optimization */
    _mm512_store_epi64((void*)&global_result[16], result);
}

/* AVX-512 ternary logic with three vectors and immediate */
void test_avx512_ternary(__m512i a, __m512i b, __m512i c) {
    /* _mm512_ternarylogic_epi64 takes 4 args but expands with mask */
    __m512i result = _mm512_ternarylogic_epi64(a, b, c, 0xE8); /* (A & B) | (C & ~(A | B)) */
    
    /* Combine with another operation */
    result = _mm512_maskz_add_epi64(0xAA, result, a);
    
    _mm512_store_epi64((void*)&global_result[24], result);
}

/* Complex gather operation - may expand to many operands */
void test_avx512_gather(__m512i base, __m512i index, __mmask16 mask) {
    __m512i scale = _mm512_set1_epi64(8);
    __m512i result = _mm512_mask_i64gather_epi64(_mm512_setzero_si512(), 
                                                mask, 
                                                index, 
                                                (const void*)&global_result[0],
                                                scale);
    
    _mm512_store_epi64((void*)&global_result[32], result);
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64_index takes base, offsets, pg - may expand to many operands */
    svint64_t result = svld1_gather_s64_index(pg, &global_result[0], base, offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)&global_result[40], result);
}

/* SVE complex permute (tbl) */
void test_sve_permute(svint64_t a, svint64_t b, svint64_t indices) {
    /* Create table from two vectors */
    svint64_t table = svzip1_s64(a, b);
    
    /* Table lookup with indices - may require many operands */
    svint64_t result = svtbl_s64(table, indices);
    
    svbool_t pg = svptrue_b64();
    svst1_s64(pg, (int64_t*)&global_result[48], result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* Complex vector permute with three vectors */
void test_altivec_permute(vector signed long long a, 
                         vector signed long long b,
                         vector unsigned char perm) {
    /* vec_perm with three vectors - may expand to many operands */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Additional operation */
    result = vec_add(result, vec_sld(a, b, 8));
    
    /* Store result */
    vec_st(result, 0, (vector signed long long*)&global_result[56]);
}
#endif

/* ==================== Main Test Driver ==================== */

int main(int argc, char **argv) {
    /* Initialize test data */
    int64_t test_data[64];
    for (int i = 0; i < 64; i++) {
        test_data[i] = i * 3 + 1;
    }
    
    /* Copy to global to prevent constant folding */
    memcpy((void*)global_result, test_data, sizeof(test_data));
    
    /* Test GCC vector extensions if available */
#ifdef __GNUC__
    {
        v8di vec_a = {0,1,2,3,4,5,6,7};
        v8di vec_b = {8,9,10,11,12,13,14,15};
        int indices[8] = {0,9,2,11,4,13,6,15}; /* Mix from both vectors */
        
        test_gcc_vector_shuffle(vec_a, vec_b, indices);
        
        v4di v1 = {1,2,3,4};
        v4di v2 = {5,6,7,8};
        v4di v3 = {9,10,11,12};
        v4di v4 = {13,14,15,16};
        v4di m1 = {0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF, 0};
        v4di m2 = {0, 0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF};
        
        test_gcc_complex_blend(v1, v2, v3, v4, m1, m2);
    }
#endif
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512i vec_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i vec_b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
        __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __mmask16 mask = 0xAAAA;
        
        test_avx512_permute(vec_a, vec_b, idx, mask);
        
        __m512i vec_c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
        test_avx512_ternary(vec_a, vec_b, vec_c);
        
        __m512i base_vec = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i index_vec = _mm512_set_epi64(0,2,4,6,8,10,12,14);
        test_avx512_gather(base_vec, index_vec, 0xFF);
    }
#endif
    
    /* Test ARM SVE if available */
#ifdef __ARM_FEATURE_SVE
    {
        svbool_t pg = svptrue_b64();
        svint64_t base = svld1_s64(pg, (const int64_t*)&global_result[0]);
        svint64_t offsets = svindex_s64(0, 2);
        
        test_sve_gather(base, offsets, pg);
        
        svint64_t vec_a = svld1_s64(pg, (const int64_t*)&global_result[8]);
        svint64_t vec_b = svld1_s64(pg, (const int64_t*)&global_result[16]);
        svint64_t indices = svindex_s64(0, 1);
        
        test_sve_permute(vec_a, vec_b, indices);
    }
#endif
    
    /* Test PowerPC Altivec if available */
#ifdef __ALTIVEC__
    {
        vector signed long long vec_a = (vector signed long long){0,1,2,3};
        vector signed long long vec_b = (vector signed long long){4,5,6,7};
        vector unsigned char perm = (vector unsigned char){
            0,1,2,3,4,5,6,7,
            16,17,18,19,20,21,22,23
        };
        
        test_altivec_permute(vec_a, vec_b, perm);
    }
#endif
    
    /* Compute checksum from results */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (int)(global_result[i] & 0xFF);
    }
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 1 : 0;
}
