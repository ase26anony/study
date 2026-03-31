/* Test program to cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test function using _mm512_mask_permutex2var_epi32 - 11 operands */
void test_avx512_permute(void) {
    /* Create 11 operands for the intrinsic */
    __m512i src1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i src2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
    
    /* This intrinsic expands to many operands:
       dest, mask, idx, src1, src2 = 5 explicit operands
       But RTL expansion adds more for mask, addressing modes, etc. */
    __m512i result = _mm512_mask_permutex2var_epi32(src1, mask, idx, src2, src2);
    
    /* Store to volatile global to prevent optimization */
    _mm512_storeu_epi32((void*)global_result, result);
    
    /* Complex expression that might expand further */
    __m512i temp = _mm512_add_epi32(result, src1);
    temp = _mm512_mask_add_epi32(temp, mask, temp, src2);
    temp = _mm512_permutexvar_epi32(idx, temp);
    
    _mm512_storeu_epi32((void*)(global_result + 8), temp);
}

/* Another test with blend operations - potentially 10+ operands */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0x5555; /* 0101010101010101 */
    __mmask16 m2 = 0x3333; /* 0011001100110011 */
    
    /* Complex blend expression that may expand to many operands */
    __m512i ab = _mm512_mask_blend_epi32(m1, a, b);
    __m512i cd = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_mask_blend_epi32(m1 | m2, ab, cd);
    
    /* Add more operations to increase operand count */
    result = _mm512_add_epi32(result, a);
    result = _mm512_sub_epi32(result, b);
    result = _mm512_mullo_epi32(result, c);
    
    _mm512_storeu_epi32((void*)(global_result + 16), result);
}

#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Test function using __builtin_shuffle with variable indices */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[16] = {0,17,2,19,4,21,6,23,8,25,10,27,12,29,14,31};
    
    /* Create a shuffle with many operands */
    v16si result;
    for (int i = 0; i < 16; i++) {
        int idx = idx_array[i];
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Store result */
    memcpy((void*)global_result, &result, sizeof(result));
    
    /* Complex permutation expression */
    v16si temp = result + a;
    temp = temp * b;
    
    /* Another shuffle with __builtin_shufflevector */
    v16si shuffle_result;
    /* This builtin may expand to vec_perm with many operands */
    shuffle_result = __builtin_shufflevector(a, b, 
        0,17,2,19,4,21,6,23,8,25,10,27,12,29,14,31);
    
    memcpy((void*)(global_result + 32), &shuffle_result, sizeof(shuffle_result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    int64_t base_array[100];
    int64_t offset_array[100];
    int64_t result_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        base_array[i] = i * 10;
        offset_array[i] = i * 8;
    }
    
    /* Create SVE vectors - these intrinsics expand to many operands */
    svbool_t pg = svwhilelt_b64(0, svcntd());
    svint64_t base = svld1_s64(pg, base_array);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* Gather operation with multiple vector operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, base_array, offsets);
    
    /* Store result */
    svst1_s64(pg, result_array, gathered);
    
    /* Copy to global */
    memcpy((void*)global_result, result_array, sizeof(result_array[0]) * 16);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vectors expands to multiple operands */
    vector signed int result = vec_perm(a, b, perm);
    
    /* Complex expression with multiple vec_perm calls */
    vector signed int temp = vec_add(result, a);
    temp = vec_perm(temp, b, perm);
    temp = vec_madd(temp, a, b); /* Multiply-add */
    
    /* Store to global */
    memcpy((void*)global_result, &result, sizeof(result));
    memcpy((void*)(global_result + 4), &temp, sizeof(temp));
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand instruction expansion...\n");
    
    /* Call all available test functions */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests completed\n");
#endif
    
    test_gcc_vector_shuffle();
    printf("GCC vector shuffle test completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("ARM SVE test completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec test completed\n");
#endif
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 64; i++) {
        checksum += (int)global_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
