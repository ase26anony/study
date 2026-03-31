/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[16] = {0};
volatile int g_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

void test_avx512_permute(void) {
    /* Create 10+ operand permutation operation */
    __m512i src1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i src2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx = _mm512_set_epi32(0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 pattern */
    
    /* This intrinsic expands to many operands:
     * dest, mask, idx, src1, src2 = 5 explicit operands
     * But RTL expansion adds more for mask, immediate constants, etc.
     */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Force computation and store to volatile global */
    _mm512_storeu_si512((void*)g_result, result);
    
    /* Complex expression with multiple vectors to encourage more operands */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0x5555;
    __mmask16 m2 = 0x3333;
    
    /* Multi-operation expression that might expand to many RTL operands */
    __m512i tmp = _mm512_mask_add_epi32(a, m1, b, c);
    tmp = _mm512_mask_sub_epi32(tmp, m2, tmp, d);
    tmp = _mm512_mask_mullo_epi32(tmp, mask, tmp, idx);
    
    _mm512_storeu_si512((void*)(g_result + 8), tmp);
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector implementation using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

void test_gcc_vector_shuffle(void) {
    /* Create vectors with volatile elements to prevent constant folding */
    volatile int32_t data1[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    volatile int32_t data2[16] = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    volatile int indices[16] = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    v16si a, b, idx_vec;
    
    /* Load with memory barriers to prevent optimization */
    for (int i = 0; i < 16; i++) {
        ((int32_t*)&a)[i] = data1[i];
        ((int32_t*)&b)[i] = data2[i];
        ((int32_t*)&idx_vec)[i] = indices[i];
    }
    
    /* Complex permutation using __builtin_shuffle 
     * This often expands to vec_perm with many operands */
    v16si perm_result = __builtin_shuffle(a, b, idx_vec);
    
    /* Store result to prevent elimination */
    memcpy((void*)g_result, &perm_result, sizeof(perm_result));
    
    /* Additional complex expression with multiple vector operations */
    v16si c = a + b;
    v16si d = a * b;
    v16si e = c - d;
    v16si f = e * idx_vec;
    
    /* Nested shuffle operation */
    v16si final = __builtin_shuffle(f, e, idx_vec);
    memcpy((void*)(g_result + 8), &final, sizeof(final));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    const int N = 16;
    int64_t base_array[N] = {0};
    int64_t offset_array[N] = {0};
    int64_t data_array[N] = {0};
    
    for (int i = 0; i < N; i++) {
        base_array[i] = i * 8;
        offset_array[i] = i * 4;
        data_array[i] = i * 2;
    }
    
    /* Create SVE vectors - actual intrinsic usage depends on SVE width */
    svbool_t pg = svwhilelt_b64(0, N);
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* Gather operation with multiple vector operands */
    svint64_t gathered = svld1_gather_s64(pg, base_array, offsets);
    
    /* Store to prevent optimization */
    svst1_s64(pg, (int64_t*)g_result, gathered);
    
    /* Complex SVE expression with multiple operations */
    svint64_t a = svadd_s64_z(pg, base, offsets);
    svint64_t b = svmul_s64_z(pg, a, gathered);
    svint64_t c = svsub_s64_z(pg, b, offsets);
    
    svst1_s64(pg, (int64_t*)(g_result + 8), c);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    /* vec_perm with three vector arguments */
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
    
    /* vec_perm typically expands to RTL with multiple operands */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_st(result, 0, (vector signed int*)g_result);
    
    /* Multiple vec_perm operations in complex expression */
    vector signed int c = vec_add(a, b);
    vector signed int d = vec_sub(b, a);
    vector unsigned char perm2 = {24,25,26,27,8,9,10,11,28,29,30,31,12,13,14,15};
    
    vector signed int result2 = vec_perm(c, d, perm2);
    vector signed int final = vec_add(result, result2);
    
    vec_st(final, 16, (vector signed int*)g_result);
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call architecture-specific tests */
#ifdef __AVX512F__
    printf("Running AVX-512 test...\n");
    test_avx512_permute();
#endif
    
    printf("Running GCC vector extensions test...\n");
    test_gcc_vector_shuffle();
    
#ifdef __ARM_FEATURE_SVE
    printf("Running ARM SVE test...\n");
    test_sve_gather();
#endif
    
#ifdef __ALTIVEC__
    printf("Running PowerPC Altivec test...\n");
    test_altivec_permute();
#endif
    
    /* Compute checksum from results to ensure computation happens */
    for (int i = 0; i < 16; i++) {
        g_checksum += (int)g_result[i];
    }
    
    printf("Checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
