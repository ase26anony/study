/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[8] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with mask - may expand to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential values */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Create mask - alternating pattern */
    __mmask16 mask = 0xAAAA; /* 0b1010101010101010 */
    
    /* This intrinsic has many operands when expanded:
       dest, mask, idx, vec1, vec2 = 5 operands at C level,
       but RTL expansion may break this into more */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Force use of result to prevent optimization */
    _mm512_storeu_si512((void*)global_result, result);
    
    /* Complex expression that might expand further */
    __m512i temp1 = _mm512_add_epi32(result, vec1);
    __m512i temp2 = _mm512_sub_epi32(temp1, vec2);
    __m512i temp3 = _mm512_and_si512(temp2, idx);
    __m512i temp4 = _mm512_or_si512(temp3, result);
    
    /* Another permutation with different mask */
    __mmask16 mask2 = 0x5555;
    __m512i final_result = _mm512_mask_permutex2var_epi32(
        temp4, mask2, idx, vec2, temp1);
    
    /* Store to volatile to ensure computation */
    _mm512_storeu_si512((void*)(global_result + 8), final_result);
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector implementation using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle operation that may require many operands */
void test_gcc_vector_shuffle(v16si a, v16si b, v16si c, v16si d) {
    /* Variable indices - prevent constant folding */
    volatile int idx0 = 0, idx1 = 5, idx2 = 10, idx3 = 15;
    volatile int idx4 = 4, idx5 = 9, idx6 = 14, idx7 = 3;
    volatile int idx8 = 8, idx9 = 13, idx10 = 2, idx11 = 7;
    volatile int idx12 = 12, idx13 = 1, idx14 = 6, idx15 = 11;
    
    /* Complex expression combining multiple shuffles and operations */
    v16si temp1 = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx8, idx9, idx10, idx11, idx12, idx13, idx14, idx15);
    
    v16si temp2 = __builtin_shufflevector(c, d,
        idx15, idx14, idx13, idx12, idx11, idx10, idx9, idx8,
        idx7, idx6, idx5, idx4, idx3, idx2, idx1, idx0);
    
    /* Blend operation using conditional operator on vectors */
    v16si mask = a > b;
    v16si result = (mask & temp1) | (~mask & temp2);
    
    /* Additional complex operation */
    v16si temp3 = __builtin_shufflevector(result, temp1,
        idx0+1, idx1+1, idx2+1, idx3+1, idx4+1, idx5+1, idx6+1, idx7+1,
        idx8+1, idx9+1, idx10+1, idx11+1, idx12+1, idx13+1, idx14+1, idx15+1);
    
    /* Store result */
    memcpy((void*)global_result, &temp3, sizeof(temp3));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    const int64_t base_array[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    volatile int64_t offsets[8] = {0,2,4,6,8,10,12,14};
    
    svint64_t base = svld1_s64(svptrue_b64(), base_array);
    svint64_t offset_vec = svld1_s64(svptrue_b64(), offsets);
    
    /* Create predicate - alternating pattern */
    svbool_t pg = svptrue_pat_b64(SV_VL8);
    
    /* Gather operation with predicate, base, and offsets */
    svint64_t gathered = svld1_gather_s64(pg, base_array, offset_vec);
    
    /* Additional SVE operation with multiple vector arguments */
    svint64_t added = svadd_s64_z(pg, gathered, base);
    svint64_t multiplied = svmul_s64_z(pg, added, offset_vec);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)global_result, multiplied);
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    /* Create vectors */
    vector signed int v1 = {0,1,2,3};
    vector signed int v2 = {4,5,6,7};
    vector signed int v3 = {8,9,10,11};
    vector signed int v4 = {12,13,14,15};
    
    /* Permute control vector */
    vector unsigned char perm_ctrl = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vectors (expands to multiple operations) */
    vector signed int temp1 = vec_perm(v1, v2, perm_ctrl);
    vector signed int temp2 = vec_perm(v3, v4, perm_ctrl);
    
    /* Complex blend */
    vector signed int mask = vec_cmpgt(v1, v2);
    vector signed int result = vec_sel(temp2, temp1, mask);
    
    /* Store result */
    vec_st(result, 0, (vector signed int*)global_result);
}
#endif

/* ==================== Main Function ==================== */
int main(int argc, char *argv[]) {
    /* Initialize some vector data for GCC vector test */
    v16si vec_a, vec_b, vec_c, vec_d;
    for (int i = 0; i < 16; i++) {
        vec_a[i] = i;
        vec_b[i] = i + 16;
        vec_c[i] = i + 32;
        vec_d[i] = i + 48;
    }
    
    /* Run architecture-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    printf("AVX-512 test completed\n");
#endif
    
    /* GCC vector test (always compiled) */
    test_gcc_vector_shuffle(vec_a, vec_b, vec_c, vec_d);
    printf("GCC vector test completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("ARM SVE test completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("PowerPC Altivec test completed\n");
#endif
    
    /* Compute checksum from results to ensure computation */
    for (int i = 0; i < 16; i++) {
        checksum += global_result[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
