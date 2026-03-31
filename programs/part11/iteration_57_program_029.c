/* Test program to cover 10/11 operand RTL expansion cases in optabs.cc */
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
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* Alternating bits pattern */
    
    /* This intrinsic expands to many operands:
     * dest, mask, idx, src1, src2 = 5 explicit operands
     * But RTL expansion adds more for mask, addressing modes, etc.
     */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src1);
    
    /* Force computation and prevent dead code elimination */
    _mm512_storeu_si512((void*)g_result, result);
    
    /* Compute simple checksum */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += g_result[i];
    }
    g_checksum += sum;
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0x5555;
    __mmask16 m2 = 0xAAAA;
    
    /* Complex blend expression that may expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_add_epi32(t1, t2);
    
    _mm512_storeu_si512((void*)(g_result + 16), result);
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));

void test_gcc_vector_shuffle(void) {
    v8si a = {0,1,2,3,4,5,6,7};
    v8si b = {8,9,10,11,12,13,14,15};
    
    /* Variable indices force runtime permutation */
    volatile int idx_array[8] = {0,8,2,10,4,12,6,14};
    
    /* Create shuffle with many operands */
    v8si result = __builtin_shufflevector(a, b, 
        idx_array[0], idx_array[1], idx_array[2], idx_array[3],
        idx_array[4], idx_array[5], idx_array[6], idx_array[7]);
    
    /* Store result to prevent optimization */
    memcpy((void*)g_result, &result, sizeof(result));
    
    /* Another complex expression that may need many operands */
    v8si mask1 = {0,-1,0,-1,0,-1,0,-1};
    v8si mask2 = {-1,0,-1,0,-1,0,-1,0};
    
    v8si expr = (mask1 & a) | (mask2 & b) | (~mask1 & ~mask2 & (a + b));
    memcpy((void*)(g_result + 8), &expr, sizeof(expr));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    svbool_t pg = svptrue_b64();
    uint64_t base[8] = {0,8,16,24,32,40,48,56};
    int64_t offsets[8] = {0,1,2,3,4,5,6,7};
    int64_t data[8] = {100,200,300,400,500,600,700,800};
    
    svint64_t offset_vec = svld1_s64(pg, offsets);
    svint64_t result = svld1_gather_s64(pg, base, offset_vec);
    
    /* Store to prevent optimization */
    svst1_s64(pg, (int64_t*)g_result, result);
    
    /* Scatter with predicate, base, offset, and data vectors */
    svst1_scatter_s64(pg, base, offset_vec, result);
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vectors expands to multiple operands */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Complex expression with multiple vector operations */
    vector signed int mask = vec_cmpeq(a, a);
    vector signed int blend = vec_sel(a, b, mask);
    vector signed int final = vec_add(result, blend);
    
    memcpy((void*)g_result, &final, sizeof(final));
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call architecture-specific tests if supported */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests completed\n");
#endif
    
    test_gcc_vector_shuffle();
    printf("GCC vector tests completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE tests completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec tests completed\n");
#endif
    
    /* Final checksum to ensure all computations happened */
    int final_sum = 0;
    for (int i = 0; i < 32; i++) {
        final_sum += g_result[i];
    }
    g_checksum += final_sum;
    
    printf("Final checksum: %d\n", g_checksum);
    
    return 0;
}
