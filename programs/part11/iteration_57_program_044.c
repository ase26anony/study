/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int global_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with many operands */
void test_avx512_permute(void) {
    /* Create 11 vector/mask operands to potentially trigger 11-operand case */
    __m512i src1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i src2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
    
    /* This intrinsic has 5 explicit operands but expands to more in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Use volatile operations to prevent optimization */
    _mm512_store_epi64((void*)global_result, result);
    
    /* Another test with permutex2var_epi64 which might use different operand count */
    __m512i idx2 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i src3 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i src4 = _mm512_set_epi64(8,9,10,11,12,13,14,15);
    __mmask8 mask2 = 0xF0;
    
    __m512i result2 = _mm512_mask_permutex2var_epi64(src3, mask2, idx2, src4, src3);
    
    /* Mix results to create complex expression */
    __m512i final = _mm512_add_epi64(result, result2);
    _mm512_store_epi64((void*)(global_result + 8), final);
}

/* Test blend with multiple sources - may expand to many operands */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0x5555;
    __mmask16 m2 = 0x3333;
    
    /* Complex blend expression that might expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_add_epi32(t1, t2);
    
    /* Force use of result */
    asm volatile("" : "+x"(result));
    _mm512_store_epi32((void*)global_result, result);
}

#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int32_t v8si __attribute__((vector_size(32)));
typedef int64_t v4di __attribute__((vector_size(32)));

/* Test with GCC vector shuffle - may trigger vec_perm with many operands */
void test_gcc_vector_shuffle(void) {
    /* Use volatile to prevent constant folding */
    volatile v8si a = {0,1,2,3,4,5,6,7};
    volatile v8si b = {8,9,10,11,12,13,14,15};
    
    /* Variable indices to prevent optimization */
    volatile int idx[16] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15};
    
    /* Complex shuffle expression */
    v8si result;
    for (int i = 0; i < 8; i++) {
        int index = idx[i];
        if (index < 8) {
            result[i] = a[index];
        } else {
            result[i] = b[index - 8];
        }
    }
    
    /* Another shuffle using builtin - may expand differently */
    v4si v1 = {0,1,2,3};
    v4si v2 = {4,5,6,7};
    v4si v3 = __builtin_shufflevector(v1, v2, 0,4,1,5);
    
    /* Store results to prevent elimination */
    memcpy((void*)global_result, &result, sizeof(result));
    memcpy((void*)(global_result + 8), &v3, sizeof(v3));
}

/* Test complex vector permutation */
void test_gcc_complex_permute(void) {
    v4di x = {0,1,2,3};
    v4di y = {4,5,6,7};
    v4di z = {8,9,10,11};
    
    /* Create a complex expression that might need many operands */
    v4di temp1 = x + y;
    v4di temp2 = y - z;
    v4di temp3 = x * z;
    
    /* Conditional blend-like operation */
    v4di mask = {0, -1, 0, -1};  /* true/false mask */
    v4di result = (mask & temp1) | (~mask & temp2);
    result = result + temp3;
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+x"(result));
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    volatile int64_t base[100];
    for (int i = 0; i < 100; i++) base[i] = i;
    
    svint64_t offsets = svindex_s64(0, 1);
    svbool_t pg = svptrue_b64();
    
    /* Gather with multiple vector arguments */
    svint64_t data = svld1_gather_s64index_s64(pg, &base[0], offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)global_result, data);
    
    /* Another test with scatter - also many operands */
    svint64_t data2 = svadd_s64_z(pg, data, svdup_s64(1));
    svst1_scatter_s64index_s64(pg, (int64_t*)(global_result + 8), offsets, data2);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
    
    /* vec_perm with three vectors - may expand to many operands */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Complex expression with multiple vec_perms */
    vector signed int c = {8,9,10,11};
    vector signed int d = {12,13,14,15};
    vector unsigned char perm2 = {24,25,26,27,28,29,30,31,8,9,10,11,12,13,14,15};
    
    vector signed int result2 = vec_perm(c, d, perm2);
    vector signed int final = vec_add(result, result2);
    
    /* Store to prevent elimination */
    vec_st(final, 0, (vector signed int*)global_result);
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    int checksum = 0;
    
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    printf("Testing AVX-512 permutations...\n");
    test_avx512_permute();
    test_avx512_blend();
#endif
    
    /* Test GCC vector extensions (always available) */
    printf("Testing GCC vector shuffles...\n");
    test_gcc_vector_shuffle();
    test_gcc_complex_permute();
    
    /* Test ARM SVE if available */
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE gather...\n");
    test_sve_gather();
#endif
    
    /* Test PowerPC Altivec if available */
#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec...\n");
    test_altivec_permute();
#endif
    
    /* Compute checksum from results */
    for (int i = 0; i < 16; i++) {
        checksum += (int)global_result[i];
    }
    
    global_checksum = checksum;
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
