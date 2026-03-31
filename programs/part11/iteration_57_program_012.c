/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_results[64] = {0};
volatile int global_index = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - can generate many operands */
void test_avx512_permute(void) {
    /* Initialize vectors with test data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Use a mask with alternating bits */
    __mmask16 mask = 0xAAAA;  /* 0b1010101010101010 */
    
    /* This intrinsic takes 5 arguments but expands to many more operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store to volatile global to prevent optimization */
    _mm512_store_epi64((void*)&global_results[global_index], result);
    global_index += 8;
}

/* AVX-512 blend with multiple sources - complex expression */
void test_avx512_complex_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __m512i mask1 = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i mask2 = _mm512_set1_epi32(0x00000000);
    
    /* Complex expression that may expand to many operands */
    __m512i temp1 = _mm512_and_si512(mask1, a);
    __m512i temp2 = _mm512_andnot_si512(mask1, b);
    __m512i temp3 = _mm512_and_si512(mask2, c);
    __m512i temp4 = _mm512_andnot_si512(mask2, d);
    
    __m512i result = _mm512_or_si512(
        _mm512_or_si512(temp1, temp2),
        _mm512_or_si512(temp3, temp4)
    );
    
    _mm512_store_epi64((void*)&global_results[global_index], result);
    global_index += 8;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    int64_t base_array[32] = {0};
    int64_t offset_array[32] = {0};
    int64_t data_array[32] = {0};
    
    for (int i = 0; i < 32; i++) {
        base_array[i] = i * 100;
        offset_array[i] = i * 8;
        data_array[i] = i;
    }
    
    svbool_t pg = svwhilelt_b64(0, svcntd());
    svint64_t base = svld1(pg, base_array);
    svint64_t offsets = svld1(pg, offset_array);
    
    /* Gather operation with multiple vector arguments */
    svint64_t result = svld1_gather_offset(pg, base_array, offsets);
    
    /* Store result */
    svst1(pg, &global_results[global_index], result);
    global_index += svcntd();
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */
/* Portable version using GCC vector extensions */

typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[16] = {
        0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23
    };
    
    /* Use __builtin_shuffle with variable indices */
    v16si result;
    for (int i = 0; i < 16; i++) {
        int idx = idx_array[i];
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Store to global */
    memcpy((void*)&global_results[global_index], &result, sizeof(result));
    global_index += 8;
}

/* Complex expression with multiple vector operations */
void test_gcc_complex_expr(void) {
    v8di v1 = {1,2,3,4,5,6,7,8};
    v8di v2 = {9,10,11,12,13,14,15,16};
    v8di v3 = {17,18,19,20,21,22,23,24};
    v8di v4 = {25,26,27,28,29,30,31,32};
    v8di mask1 = {0xFFFFFFFFFFFFFFFFLL, 0, 0xFFFFFFFFFFFFFFFFLL, 0,
                  0xFFFFFFFFFFFFFFFFLL, 0, 0xFFFFFFFFFFFFFFFFLL, 0};
    v8di mask2 = {0, 0xFFFFFFFFFFFFFFFFLL, 0, 0xFFFFFFFFFFFFFFFFLL,
                  0, 0xFFFFFFFFFFFFFFFFLL, 0, 0xFFFFFFFFFFFFFFFFLL};
    
    /* Complex blend expression that may expand to many operands */
    v8di result = (mask1 & v1) | (~mask1 & mask2 & v2) | 
                  (~mask1 & ~mask2 & v3) | (mask1 & mask2 & v4);
    
    memcpy((void*)&global_results[global_index], &result, sizeof(result));
    global_index += 8;
}

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansions...\n");
    
    /* Call target-specific tests */
#ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    test_avx512_permute();
    test_avx512_complex_blend();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Running ARM SVE tests...\n");
    test_sve_gather();
#endif
    
    /* Always run GCC vector tests (portable) */
    printf("Running GCC vector tests...\n");
    test_gcc_vector_shuffle();
    test_gcc_complex_expr();
    
    /* Compute checksum to ensure code isn't optimized away */
    int64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += global_results[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    printf("Test completed.\n");
    
    return 0;
}
