/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[64] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

/* Using GCC vector extensions for portable vector permutation */
typedef int64_t v4di __attribute__((vector_size(32)));  /* 256-bit vector */
typedef int64_t v8di __attribute__((vector_size(64)));  /* 512-bit vector */

/* Function using __builtin_shuffle with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, v8di c, v8di d, 
                             v8di mask1, v8di mask2, int *indices) {
    /* Complex expression that may expand to many operands */
    v8di temp1 = __builtin_shuffle(a, b, 
        (v8di){indices[0], indices[1], indices[2], indices[3],
               indices[4], indices[5], indices[6], indices[7]});
    
    v8di temp2 = __builtin_shuffle(c, d,
        (v8di){indices[8], indices[9], indices[10], indices[11],
               indices[12], indices[13], indices[14], indices[15]});
    
    /* Blend operation using masks - may create complex RTL */
    v8di result = (mask1 & temp1) | (~mask1 & temp2);
    result = (mask2 & result) | (~mask2 & (temp1 + temp2));
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with mask - can generate many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, 
                         __mmask16 mask, __m512i c, __m512i d) {
    /* _mm512_mask_permutex2var_epi32 has many operands when expanded:
       dest, mask, idx, a, b -> potentially 5 vector + mask = 6 operands
       Combined with other operations can reach 10-11 */
    
    __m512i temp1 = _mm512_mask_permutex2var_epi32(a, mask, idx, b, c);
    __m512i temp2 = _mm512_mask_permutex2var_epi64(d, 0xFF, idx, a, b);
    
    /* Complex blend with mask */
    __m512i result = _mm512_mask_blend_epi32(mask, temp1, temp2);
    
    /* Additional operation to potentially increase operand count */
    result = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    
    /* Store result */
    _mm512_storeu_si512((void*)global_result, result);
}

/* Another AVX-512 test with gather operation */
void test_avx512_gather(__m512i base, __m512i index, __mmask16 mask) {
    /* _mm512_mask_i32gather_epi32 has: src, mask, index, base, scale
       When expanded with all operands explicit, can have many */
    __m512i scale_vec = _mm512_set1_epi32(4);
    
    /* Complex gather expression */
    __m512i result = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // index
        (const void*)global_result, // base pointer
        4);                      // scale
    
    /* Store */
    _mm512_storeu_si512((void*)(global_result + 32), result);
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with predicate - can have many operands */
void test_sve_gather(int64_t *base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64 has: pg, base, offsets -> 3 operands
       Combined with other operations in expansion */
    svint64_t data = svld1_gather_s64(pg, base, offsets);
    
    /* Additional operations */
    svint64_t ones = svdup_n_s64(1);
    svint64_t result = svadd_s64_z(pg, data, ones);
    
    /* Store with scatter - another multi-operand operation */
    svst1_scatter_s64(pg, base + 32, offsets, result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* vec_perm with three vectors - may expand to many operands */
vector signed long long test_ppc_permute(vector signed long long a,
                                        vector signed long long b,
                                        vector unsigned char perm) {
    /* vec_perm(a, b, perm) takes 3 vectors */
    vector signed long long temp1 = vec_perm(a, b, perm);
    
    /* Combine with other operations */
    vector signed long long temp2 = vec_add(a, b);
    vector signed long long result = vec_sel(temp1, temp2, 
        (vector unsigned long long){0xFF00FF00FF00FF00ULL});
    
    return result;
}
#endif

/* ==================== Main Test Driver ==================== */

int main(int argc, char **argv) {
    /* Initialize test data */
    int indices[16];
    for (int i = 0; i < 16; i++) {
        indices[i] = (i * 3) % 16;
    }
    
    /* Initialize vectors for GCC extensions */
    v8di a = {0,1,2,3,4,5,6,7};
    v8di b = {8,9,10,11,12,13,14,15};
    v8di c = {16,17,18,19,20,21,22,23};
    v8di d = {24,25,26,27,28,29,30,31};
    v8di mask1 = {-1,0,-1,0,-1,0,-1,0};  /* Pattern of all 1s and 0s */
    v8di mask2 = {0,-1,0,-1,0,-1,0,-1};
    
    /* Test GCC vector shuffle (most portable) */
    test_gcc_vector_shuffle(a, b, c, d, mask1, mask2, indices);
    
    /* Update checksum from global_result */
    for (int i = 0; i < 8; i++) {
        checksum += global_result[i];
    }
    
#ifdef __AVX512F__
    /* Initialize AVX-512 test data */
    __m512i avx_a = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i avx_b = _mm512_set_epi32(16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
    __m512i idx = _mm512_set_epi32(0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 pattern */
    
    test_avx512_permute(avx_a, avx_b, idx, mask, avx_a, avx_b);
    
    /* Update checksum */
    for (int i = 0; i < 16; i++) {
        checksum += ((int32_t*)global_result)[i];
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    /* SVE test would require runtime length queries */
    /* This is a placeholder - actual SVE code needs length-agnostic approach */
#endif
    
#ifdef __ALTIVEC__
    /* PowerPC test */
    vector signed long long ppc_a = {0,1,2,3};
    vector signed long long ppc_b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    
    vector signed long long ppc_result = test_ppc_permute(ppc_a, ppc_b, perm);
    
    /* Store to global */
    memcpy((void*)(global_result + 40), &ppc_result, sizeof(ppc_result));
#endif
    
    /* Final checksum and output */
    printf("Test completed. Checksum: %d\n", checksum);
    
    return 0;
}
