/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int global_checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

/* Generic vector types using GCC extensions */
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Test function using GCC vector extensions with complex shuffle */
void test_gcc_vector_shuffle(v8di a, v8di b, v8di mask) {
    /* Complex expression that may expand to many operands */
    v8di idx = {0, 9, 2, 11, 4, 13, 6, 15};
    
    /* Use __builtin_shuffle with variable indices - forces general permute */
    v8di result = __builtin_shuffle(a, b, idx);
    
    /* Another complex shuffle combining multiple operations */
    v8di temp1 = __builtin_shuffle(a, b, (v8di){8, 1, 10, 3, 12, 5, 14, 7});
    v8di temp2 = __builtin_shuffle(b, a, (v8di){7, 14, 5, 12, 3, 10, 1, 8});
    
    /* Blend operation using mask - creates complex RTL */
    v8di blended = (mask & result) | (~mask & temp1);
    
    /* Store to prevent optimization */
    for (int i = 0; i < 8; i++) {
        global_result[i] = blended[i];
    }
}

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has many operands:
       dest, mask, idx, a, b -> potentially expands to 10+ operands */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, a, b);
    
    /* Another complex operation with blend */
    __m512i shuffled = _mm512_permutex2var_epi64(a, idx, b);
    __m512i blended = _mm512_mask_blend_epi32(mask, a, shuffled);
    
    /* Store results */
    _mm512_storeu_si512((void*)global_result, blended);
}

/* Test AVX-512 gather with many operands */
void test_avx512_gather(void) {
    int64_t base[16] = {0};
    __m512i vindex = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_i64gather_epi64 has many operands:
       src, mask, vindex, base, scale, src2 */
    __m512i result = _mm512_mask_i64gather_epi64(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        vindex,                  // vindex
        (void*)base,             // base
        8                        // scale
    );
    
    _mm512_storeu_si512((void*)global_result, result);
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* Test SVE gather with many operands */
void test_sve_gather(void) {
    int64_t base[16] = {0};
    
    /* Create SVE vectors - these intrinsics expand to many RTL operands */
    svint64_t vindex = svdup_s64(0);
    svbool_t pg = svptrue_b64();
    
    /* svld1_gather_s64 has many operands: base, index, predicate */
    svint64_t result = svld1_gather_s64(pg, &base[0], vindex);
    
    /* Store results */
    svst1_s64(pg, (int64_t*)global_result, result);
}

/* Test SVE permute with multiple vector arguments */
void test_sve_permute(svint64_t a, svint64_t b, svint64_t idx) {
    /* Complex SVE operation that may expand to many operands */
    svint64_t temp1 = svtbl_s64(a, idx);
    svint64_t temp2 = svtbl_s64(b, idx);
    
    /* Blend operation */
    svbool_t pg = svptrue_b64();
    svint64_t result = svsel_s64(pg, temp1, temp2);
    
    /* Store */
    svst1_s64(pg, (int64_t*)global_result, result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* Test PowerPC vector permute with three vectors */
void test_ppc_permute(vector signed long long a, 
                      vector signed long long b,
                      vector unsigned char perm) {
    /* vec_perm with three vector arguments */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_st(result, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main(int argc, char **argv) {
    /* Initialize test data */
    v8di a = {0, 1, 2, 3, 4, 5, 6, 7};
    v8di b = {8, 9, 10, 11, 12, 13, 14, 15};
    v8di mask = {0, -1, 0, -1, 0, -1, 0, -1};
    
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Test GCC vector extensions (always available) */
    test_gcc_vector_shuffle(a, b, mask);
    
    /* Test architecture-specific intrinsics if available */
#ifdef __AVX512F__
    printf("Testing AVX-512...\n");
    __m512i avx_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i avx_b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
    __m512i avx_idx = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __mmask16 avx_mask = 0xAAAA;
    
    test_avx512_permute(avx_a, avx_b, avx_idx, avx_mask);
    test_avx512_gather();
#endif

#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE...\n");
    test_sve_gather();
    
    /* Create SVE vectors for permute test */
    svint64_t sve_a = svdup_s64(0);
    svint64_t sve_b = svdup_s64(1);
    svint64_t sve_idx = svdup_s64(0);
    test_sve_permute(sve_a, sve_b, sve_idx);
#endif

#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec...\n");
    vector signed long long ppc_a = {0, 1};
    vector signed long long ppc_b = {2, 3};
    vector unsigned char ppc_perm = {0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23};
    test_ppc_permute(ppc_a, ppc_b, ppc_perm);
#endif
    
    /* Compute checksum to ensure all operations executed */
    for (int i = 0; i < 16; i++) {
        global_checksum += (int)global_result[i];
    }
    
    printf("Checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
