/* Test program to trigger 10-11 operand RTL expansions in GCC's optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_results[64] = {0};
volatile int global_index = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - likely to generate many operands */
void test_avx512_permute(void) {
    /* Initialize vectors with test data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Create a complex mask - alternating pattern */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
    
    /* This intrinsic takes: dest, mask, idx, vec1, vec2
       When expanded, this often becomes many operands */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Force computation and prevent dead code elimination */
    _mm512_store_epi64((void*)&global_results[global_index], result);
    global_index += 8;
    
    /* Try another variant with more operands */
    __m512i vec3 = _mm512_set_epi32(63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48);
    __m512i idx2 = _mm512_set_epi32(16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
    __mmask16 mask2 = 0x5555;  /* 0101010101010101 binary */
    
    /* Complex expression that might expand to many operands */
    __m512i temp = _mm512_mask_permutex2var_epi32(vec2, mask2, idx2, vec3, vec1);
    result = _mm512_mask_add_epi32(result, mask, temp, vec2);
    
    _mm512_store_epi64((void*)&global_results[global_index], result);
    global_index += 8;
}

/* AVX-512 blend with multiple vectors and masks */
void test_avx512_complex_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    __mmask16 m1 = 0xF0F0;
    __mmask16 m2 = 0x0F0F;
    __mmask16 m3 = 0xCCCC;
    
    /* Complex blend expression that may expand to many operands */
    __m512i ab = _mm512_mask_blend_epi32(m1, a, b);
    __m512i cd = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_mask_blend_epi32(m3, ab, cd);
    
    /* Add another operation to increase complexity */
    result = _mm512_add_epi32(result, _mm512_set1_epi32(global_index));
    
    _mm512_store_epi64((void*)&global_results[global_index], result);
    global_index += 8;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    const int64_t base_array[32] = {0};
    int64_t offsets[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    
    svint64_t base = svld1_s64(svptrue_b64(), base_array);
    svint64_t offset_vec = svld1_s64(svptrue_b64(), offsets);
    
    /* Create a complex predicate */
    svbool_t pg = svwhilelt_b64(0, 8);
    
    /* Gather operation with multiple vector operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, base, offset_vec);
    
    /* Store to prevent optimization */
    svst1_s64(pg, (int64_t*)&global_results[global_index], gathered);
    global_index += 8;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable indices to prevent constant folding */
    volatile int idx0 = global_index % 16;
    volatile int idx1 = (global_index + 1) % 16;
    volatile int idx2 = (global_index + 2) % 16;
    volatile int idx3 = (global_index + 3) % 16;
    volatile int idx4 = (global_index + 4) % 16;
    volatile int idx5 = (global_index + 5) % 16;
    volatile int idx6 = (global_index + 6) % 16;
    volatile int idx7 = (global_index + 7) % 16;
    volatile int idx8 = (global_index + 8) % 16;
    volatile int idx9 = (global_index + 9) % 16;
    volatile int idx10 = (global_index + 10) % 16;
    volatile int idx11 = (global_index + 11) % 16;
    volatile int idx12 = (global_index + 12) % 16;
    volatile int idx13 = (global_index + 13) % 16;
    volatile int idx14 = (global_index + 14) % 16;
    volatile int idx15 = (global_index + 15) % 16;
    
    /* __builtin_shufflevector with many indices - may expand to high operand count */
    v16si result = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7,
        idx8, idx9, idx10, idx11, idx12, idx13, idx14, idx15);
    
    /* Store result to prevent optimization */
    memcpy((void*)&global_results[global_index], &result, sizeof(result));
    global_index += 8;
}

/* Complex vector expression that might expand to many operands */
void test_gcc_complex_expression(void) {
    v8di v1 = {0,1,2,3,4,5,6,7};
    v8di v2 = {8,9,10,11,12,13,14,15};
    v8di v3 = {16,17,18,19,20,21,22,23};
    v8di v4 = {24,25,26,27,28,29,30,31};
    
    /* Complex expression combining multiple vectors */
    v8di temp1 = v1 + v2;
    v8di temp2 = v3 - v4;
    v8di temp3 = v1 * v4;
    v8di temp4 = v2 / (v3 + v8di){1,1,1,1,1,1,1,1};
    
    /* Even more complex expression */
    v8di result = (temp1 & temp2) | (temp3 ^ temp4);
    result = result + (v1 << 2) - (v2 >> 1);
    
    memcpy((void*)&global_results[global_index], &result, sizeof(result));
    global_index += 8;
}

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing high-operand RTL expansions...\n");
    
    /* Call target-specific tests if supported */
#ifdef __AVX512F__
    printf("Testing AVX-512 permutations...\n");
    test_avx512_permute();
    test_avx512_complex_blend();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE gather...\n");
    test_sve_gather();
#endif
    
    /* Always test GCC vector extensions */
    printf("Testing GCC vector extensions...\n");
    test_gcc_vector_shuffle();
    test_gcc_complex_expression();
    
    /* Compute checksum to ensure computations aren't optimized away */
    int64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += global_results[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    printf("Test completed.\n");
    
    return 0;
}
