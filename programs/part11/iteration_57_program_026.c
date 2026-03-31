/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[8] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

void test_avx512_permute(void) {
    /* Create 10 operands for _mm512_mask_permutex2var_epi32:
       1. Destination (a)
       2. Mask
       3. Index vector
       4. Source 1 (b)
       5. Source 2 (c)
       Plus 5 more vector operands for complex expression */
    
    __m512i a = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __m512i b = _mm512_set_epi32(16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);
    __m512i c = _mm512_set_epi32(32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47);
    __m512i idx = _mm512_set_epi32(0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
    
    /* This intrinsic expands to many operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, c);
    
    /* Complex expression with multiple vectors to encourage high operand count */
    __m512i d = _mm512_set1_epi32(100);
    __m512i e = _mm512_set1_epi32(200);
    __m512i f = _mm512_set1_epi32(300);
    
    /* Create a complex blend operation that may expand to many operands */
    __m512i temp1 = _mm512_add_epi32(result, d);
    __m512i temp2 = _mm512_sub_epi32(temp1, e);
    __m512i temp3 = _mm512_mullo_epi32(temp2, f);
    
    /* Use permute again with different sources */
    __m512i idx2 = _mm512_set_epi32(31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16);
    __m512i final_result = _mm512_permutex2var_epi64(temp3, idx2, b, c);
    
    /* Store to volatile global to prevent dead code elimination */
    _mm512_storeu_si512((void*)global_result, final_result);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)global_result[i];
    }
    checksum += sum;
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));

void test_gcc_vector_shuffle(void) {
    /* Create vectors with different patterns */
    v16si v1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    v16si v2 = {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    v16si v3 = {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
    
    /* Variable index array - prevents constant folding */
    volatile int indices[16] = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};
    
    /* Complex permutation expression that may expand to many operands */
    v16si result;
    
    /* Manual shuffle using GCC vector extensions - this creates complex RTL */
    for (int i = 0; i < 16; i++) {
        int idx = indices[i];
        if (idx < 16) {
            result[i] = v1[idx];
        } else if (idx < 32) {
            result[i] = v2[idx - 16];
        } else {
            result[i] = v3[idx - 32];
        }
    }
    
    /* Additional complex operations to increase operand count */
    v16si mask1 = {0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1};
    v16si mask2 = {-1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0};
    
    /* Complex blend operation: (mask1 & v1) | (mask2 & v2) | (~mask1 & ~mask2 & v3) */
    v16si blend1 = mask1 & v1;
    v16si blend2 = mask2 & v2;
    v16si blend3 = (~mask1) & (~mask2) & v3;
    v16si final_blend = blend1 | blend2 | blend3;
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &final_blend, sizeof(final_blend));
    
    /* Update checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)global_result[i];
    }
    checksum += sum;
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    const int N = 16;
    int64_t base_array[N] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150};
    int64_t offset_array[N] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int64_t data_array[N] = {1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 
                             1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015};
    
    svbool_t pg = svwhilelt_b64(0, N);
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    svint64_t data = svld1_s64(pg, data_array);
    
    /* Gather operation with multiple vector operands */
    svint64_t gathered = svld1_gather_s64(pg, base_array, offsets);
    
    /* Complex scatter operation */
    svst1_scatter_s64(pg, base_array, offsets, data);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)global_result, gathered);
    
    /* Update checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)global_result[i];
    }
    checksum += sum;
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    /* vec_perm takes 3 vectors and returns a permuted vector */
    vector signed int v1 = {0, 1, 2, 3};
    vector signed int v2 = {4, 5, 6, 7};
    vector signed int v3 = {8, 9, 10, 11};
    vector unsigned char perm1 = {0,1,2,3, 16,17,18,19, 4,5,6,7, 20,21,22,23};
    vector unsigned char perm2 = {8,9,10,11, 24,25,26,27, 12,13,14,15, 28,29,30,31};
    
    /* Multiple vec_perm operations in complex expression */
    vector signed int result1 = vec_perm(v1, v2, perm1);
    vector signed int result2 = vec_perm(v2, v3, perm2);
    vector signed int result3 = vec_perm(v1, v3, perm1);
    
    /* Complex blend */
    vector signed int final_result = vec_add(vec_add(result1, result2), result3);
    
    /* Store */
    memcpy((void*)global_result, &final_result, sizeof(final_result));
    
    /* Update checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)global_result[i];
    }
    checksum += sum;
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing high-operand-count RTL expansions...\n");
    
    /* Call target-specific tests */
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
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
