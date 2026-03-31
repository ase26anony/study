/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

void test_avx512_permute(void) {
    /* Create 10+ operand permutation operation */
    __m512i src1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i src2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
    
    /* This intrinsic expands to many operands:
     * 1. Destination
     * 2. Mask
     * 3. Index vector
     * 4. Source 1
     * 5. Source 2
     * Plus additional implicit operands during RTL expansion
     */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Another variant with different sources */
    __m512i result2 = _mm512_mask2_permutex2var_epi32(src1, idx, mask, src2, src1);
    
    /* Store results to prevent optimization */
    _mm512_storeu_si512((void*)global_result, result);
    _mm512_storeu_si512((void*)(global_result + 8), result2);
    
    /* Complex blend operation with many operands */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0xCCCC;
    __mmask16 m2 = 0x3333;
    
    /* This should generate complex RTL with many operands */
    __m512i blend = _mm512_mask_blend_epi32(m1, 
                     _mm512_mask_blend_epi32(m2, a, b),
                     _mm512_mask_blend_epi32(m2, c, d));
    
    _mm512_storeu_si512((void*)(global_result + 16), blend);
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

void test_gcc_vector_shuffle(void) {
    /* Create vectors with sequential values */
    v16si va = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si vb = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array - prevents constant folding */
    volatile int indices[16] = {0,17,2,19,4,21,6,23,8,25,10,27,12,29,14,31};
    
    /* Complex permutation that may require many operands */
    v16si vc;
    for (int i = 0; i < 16; i++) {
        int idx = indices[i];
        if (idx < 16) {
            vc[i] = va[idx];
        } else {
            vc[i] = vb[idx - 16];
        }
    }
    
    /* Store result */
    memcpy((void*)global_result, &vc, sizeof(vc));
    
    /* Another test with __builtin_shufflevector */
    v8di v1 = {0,1,2,3,4,5,6,7};
    v8di v2 = {8,9,10,11,12,13,14,15};
    
    /* Shuffle with many indices - may expand to high operand count */
    v8di v3 = __builtin_shufflevector(v1, v2, 
                                      0,8,2,10,4,12,6,14,
                                      1,9,3,11,5,13,7,15);
    
    memcpy((void*)(global_result + 8), &v3, sizeof(v3));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    svbool_t pg = svptrue_b64();
    uint64_t base = 0x1000;
    svuint64_t offsets = svindex_u64(0, 1);
    
    /* Gather with multiple vector arguments */
    svuint64_t data = svld1_gather_u64index_u64(pg, (const uint64_t*)base, offsets);
    
    /* Store to prevent optimization */
    svst1_u64(pg, (uint64_t*)global_result, data);
    
    /* Complex scatter operation */
    svuint64_t values = svadd_u64_x(pg, data, svdup_u64(1));
    svst1_scatter_u64index_u64(pg, (uint64_t*)global_result, offsets, values);
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    /* vec_perm with three vectors can expand to many operands */
    vector signed int v1 = {0,1,2,3};
    vector signed int v2 = {4,5,6,7};
    vector signed int v3 = {8,9,10,11};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,32,33,34,35,48,49,50,51};
    
    /* Complex permutation sequence */
    vector signed int r1 = vec_perm(v1, v2, perm);
    vector signed int r2 = vec_perm(v2, v3, perm);
    vector signed int r3 = vec_perm(v1, r1, perm);
    vector signed int result = vec_add(r2, r3);
    
    /* Store result */
    vec_st(result, 0, (vector signed int*)global_result);
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call architecture-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    printf("AVX-512 test completed\n");
#endif
    
    test_gcc_vector_shuffle();
    printf("GCC vector shuffle test completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("ARM SVE test completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("PowerPC Altivec test completed\n");
#endif
    
    /* Compute checksum to ensure all operations executed */
    for (int i = 0; i < 32; i++) {
        checksum += (int)global_result[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
