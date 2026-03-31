/* Test program to trigger 10-11 operand RTL expansions in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

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
       dest, mask, idx, src1, src2 = 5 explicit operands
       But RTL expansion adds more for mask, memory, etc. */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Force use of result to prevent optimization */
    _mm512_storeu_si512((void*)global_result, result);
    
    /* Complex expression that might expand further */
    __m512i blend1 = _mm512_mask_blend_epi32(0x5555, src1, src2);
    __m512i blend2 = _mm512_mask_blend_epi32(0xAAAA, src2, src1);
    __m512i final = _mm512_add_epi32(blend1, blend2);
    _mm512_storeu_si512((void*)(global_result + 8), final);
}

/* AVX-512 with 3-source blend - potentially many operands */
__attribute__((noinline))
void test_avx512_ternlog(void) {
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i c = _mm512_set_epi64(23,22,21,20,19,18,17,16);
    
    /* Ternary logic with immediate - expands to many operands */
    __m512i res = _mm512_ternarylogic_epi64(a, b, c, 0xE8); /* (A & B) | (C & ~(A | B)) */
    
    /* Chain multiple operations */
    res = _mm512_add_epi64(res, a);
    res = _mm512_sub_epi64(res, b);
    
    _mm512_storeu_si512((void*)(global_result + 16), res);
}
#endif

/* ========== GCC Vector Extensions (Portable) ========== */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - forces vec_perm expansion */
__attribute__((noinline))
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array prevents constant folding */
    volatile int indices[16] = {0,16,2,18,4,20,6,22,8,24,10,26,12,28,14,30};
    
    /* Create permutation - this should expand to vec_perm with many operands */
    v16si result;
    for (int i = 0; i < 16; i++) {
        int idx = indices[i];
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
    
    /* Another complex expression */
    v16si mask = {0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1};
    v16si blended = (mask & a) | (~mask & b);
    memcpy((void*)(global_result + 8), &blended, sizeof(blended));
}

/* ========== ARM SVE Implementation ========== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

__attribute__((noinline))
void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    int64_t base_array[32];
    for (int i = 0; i < 32; i++) base_array[i] = i * 100;
    
    svint64_t offsets = svindex_s64(0, 1);
    svbool_t pg = svptrue_b64();
    
    /* Gather with base + offsets - expands to many operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, &base_array[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)(global_result + 24), gathered);
    
    /* Complex predicate operation */
    svint64_t data1 = svdup_s64(42);
    svint64_t data2 = svdup_s64(99);
    svbool_t pred1 = svcmplt_s64(pg, offsets, svdup_s64(8));
    svbool_t pred2 = svcmpgt_s64(pg, offsets, svdup_s64(4));
    
    /* Blend based on two predicates - potentially many operands */
    svint64_t blended = svsel_s64(pred1, data1, 
                         svsel_s64(pred2, data2, gathered));
    
    svst1_s64(pg, (int64_t*)(global_result + 28), blended);
}
#endif

/* ========== PowerPC Altivec/VSX ========== */
#ifdef __ALTIVEC__
#include <altivec.h>

__attribute__((noinline))
void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,20,21,22,23};
    
    /* vec_perm with three vectors - may expand to many operands */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Complex expression */
    vector signed int mask = {0,-1,0,-1};
    vector signed int blended = vec_sel(a, b, (vector unsigned int)mask);
    vector signed int final = vec_add(result, blended);
    
    memcpy((void*)global_result, &final, sizeof(final));
}
#endif

/* ========== Main Test Driver ========== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call all target-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_ternlog();
    printf("AVX-512 tests executed\n");
#endif
    
    test_gcc_vector_shuffle();
    printf("GCC vector tests executed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE tests executed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec tests executed\n");
#endif
    
    /* Compute checksum to ensure all operations executed */
    for (int i = 0; i < 32; i++) {
        checksum += (int)global_result[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
