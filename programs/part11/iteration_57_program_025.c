/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[8] = {0};
volatile int g_checksum = 0;

/* ========== AVX-512 Implementation ========== */
#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permutex2var intrinsic which expands to many operands */
void test_avx512_permute(void) {
    /* Create 10+ operands for the permutation */
    __m512i src1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i src2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx = _mm512_set_epi32(1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31);
    __mmask16 mask = 0xAAAA;  /* Alternating bits pattern */
    
    /* _mm512_mask_permutex2var_epi32 has 5 explicit args, but expands to many more:
       dest, src1, mask, idx, src2 -> potentially 10+ operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2);
    
    /* Store to volatile global to prevent dead code elimination */
    _mm512_storeu_si512((void*)g_result, result);
    
    /* Also test the 64-bit version */
    __m512i src1_64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i src2_64 = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    __m512i idx_64 = _mm512_set_epi64(1,3,5,7,9,11,13,15);
    __mmask8 mask_64 = 0xAA;
    
    __m512i result2 = _mm512_mask_permutex2var_epi64(src1_64, mask_64, idx_64, src2_64);
    
    /* Force computation */
    asm volatile("" : "+x"(result2));
}
#endif

/* ========== GCC Vector Extensions ========== */
/* Portable vector permutation using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle that may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v16si a, v16si b, v16si idx_vec) {
    /* Create a complex permutation expression.
       The __builtin_shufflevector with variable indices expands to 
       vec_perm RTL which can have many operands */
    v16si result;
    
    /* Use variable indices from idx_vec to prevent constant folding */
    int32_t idx_array[16];
    memcpy(idx_array, &idx_vec, sizeof(idx_array));
    
    /* Manually construct shuffle with many operands */
    for (int i = 0; i < 16; i++) {
        int idx = idx_array[i] & 31;  /* Ensure valid index 0-31 */
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Complex blend operation that might generate multi-operand RTL */
    v16si mask1 = a > b;
    v16si mask2 = a < (b << 1);
    v16si src1 = a * 2;
    v16si src2 = b * 3;
    v16si src3 = a + b;
    v16si src4 = a - b;
    
    /* This complex blend expression could expand to many operands:
       res = (mask1 & src1) | (~mask1 & mask2 & src2) | 
             (~mask1 & ~mask2 & src3) | (mask1 & mask2 & src4) */
    v16si blend_result = (mask1 & src1) | 
                         (~mask1 & mask2 & src2) |
                         (~mask1 & ~mask2 & src3) |
                         (mask1 & mask2 & src4);
    
    /* Store to prevent optimization */
    memcpy((void*)g_result, &blend_result, sizeof(blend_result));
}

/* ========== ARM SVE Implementation ========== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather operations can expand to many operands */
void test_sve_gather(void) {
    /* Simulate a gather with multiple vector arguments */
    int64_t base_array[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    int64_t offset_array[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int64_t data_array[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    
    svbool_t pg = svptrue_b64();
    svint64_t base = svld1(pg, base_array);
    svint64_t offsets = svld1(pg, offset_array);
    svint64_t data = svld1(pg, data_array);
    
    /* Complex SVE expression that may expand to many operands */
    svint64_t scaled_offsets = svmul_x(pg, offsets, svdup_s64(2));
    svint64_t gathered = svld1_gather_offset(pg, (const int64_t*)base_array, scaled_offsets);
    svint64_t result = svadd_x(pg, gathered, data);
    
    /* Store result */
    svst1(pg, (int64_t*)g_result, result);
}
#endif

/* ========== PowerPC Altivec ========== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {1,2,3,4};
    vector signed int b = {5,6,7,8};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
    
    /* vec_perm with three vectors expands to complex RTL */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Complex expression with multiple operations */
    vector signed int mask = vec_cmpgt(a, b);
    vector signed int blended = vec_sel(a, b, mask);
    vector signed int scaled = vec_madd(a, b, blended);
    
    memcpy((void*)g_result, &scaled, sizeof(scaled));
}
#endif

/* ========== Main Test Driver ========== */
int main(int argc, char *argv[]) {
    /* Initialize test data */
    v16si vec_a, vec_b, vec_idx;
    for (int i = 0; i < 16; i++) {
        vec_a[i] = i;
        vec_b[i] = i + 16;
        vec_idx[i] = (i * 3) % 32;  /* Non-linear pattern */
    }
    
    /* Run target-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    printf("AVX-512 test completed\n");
#endif
    
    test_gcc_vector_shuffle(vec_a, vec_b, vec_idx);
    printf("GCC vector shuffle test completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE test completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec test completed\n");
#endif
    
    /* Compute checksum from results to ensure computation */
    for (int i = 0; i < 8; i++) {
        g_checksum += (int)g_result[i];
    }
    
    printf("Final checksum: %d\n", g_checksum);
    
    return 0;
}
