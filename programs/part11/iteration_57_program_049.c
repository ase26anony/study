/* Test program to cover 10-11 operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[16] = {0};
volatile int g_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with mask - likely to generate many operands */
void test_avx512_permute(void) {
    /* Initialize vectors with test data */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx  = _mm512_set_epi32(1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31);
    
    /* Use volatile to prevent constant folding */
    volatile __mmask16 mask = 0xAAAA; /* 1010101010101010 binary */
    
    /* This intrinsic takes: dest, mask, idx, vec1, vec2
     * When expanded to RTL, this may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi32(
        vec1,                /* src/dest */
        (__mmask16)mask,     /* mask */
        idx,                 /* index */
        vec2                 /* src2 */
    );
    
    /* Another complex permutation with explicit destination */
    __m512i dest;
    dest = _mm512_permutex2var_epi64(vec1, idx, vec2);
    
    /* Store results to prevent optimization */
    _mm512_storeu_si512((void*)g_result, result);
    
    /* Force computation with inline asm */
    asm volatile("" : "+x"(result) : : "memory");
}

/* AVX-512 blend with multiple sources - may expand to many operands */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    volatile __mmask16 m1 = 0xF0F0;
    volatile __mmask16 m2 = 0x0F0F;
    
    /* Complex blend expression that might expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_or_si512(t1, t2);
    
    _mm512_storeu_si512((void*)(g_result + 8), result);
}
#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation using GCC extensions */
void test_gcc_vector_shuffle(void) {
    /* Define 256-bit vector type (8 ints) */
    typedef int v8si __attribute__((vector_size(32)));
    
    /* Initialize vectors */
    v8si v1 = {0,1,2,3,4,5,6,7};
    v8si v2 = {8,9,10,11,12,13,14,15};
    
    /* Variable index array - prevents constant folding */
    volatile int idx_array[16] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15};
    
    /* Create a shuffle with many indices - may expand to vec_perm with many operands */
    v8si result;
    for (int i = 0; i < 8; i++) {
        /* Complex expression that might require multi-operand expansion */
        result[i] = (idx_array[i] < 8) ? v1[idx_array[i]] : v2[idx_array[i] - 8];
    }
    
    /* Store result */
    memcpy((void*)g_result, &result, sizeof(result));
}

/* More complex GCC vector permutation */
void test_gcc_complex_permute(void) {
    typedef float v8sf __attribute__((vector_size(32)));
    typedef int v8si __attribute__((vector_size(32)));
    
    v8sf a = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};
    v8sf b = {8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f};
    v8si idx = {7,6,5,4,3,2,1,0};
    
    /* Use __builtin_shuffle which may expand to complex RTL */
    v8sf result = __builtin_shuffle(a, b, idx);
    
    memcpy((void*)(g_result + 4), &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    svbool_t pg = svptrue_b8();
    svint64_t base = svdup_s64(0);
    svint64_t offsets = svindex_s64(0, 1);
    
    /* Gather operation with multiple vector arguments */
    svint64_t data = svld1_gather_s64index_s64(pg, (const int64_t*)g_result, offsets);
    
    /* Store back */
    svst1_s64(pg, (int64_t*)g_result, data);
}
#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    vector unsigned char a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char perm = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    /* vec_perm with three vectors may expand to many operands */
    vector unsigned char result = vec_perm(a, b, perm);
    
    vec_st(result, 0, (vector unsigned char*)g_result);
}
#endif /* __ALTIVEC__ */

/* ==================== Main Test Driver ==================== */
int main(int argc, char **argv) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Initialize global array */
    for (int i = 0; i < 16; i++) {
        g_result[i] = i;
    }
    
    /* Run architecture-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests completed\n");
#endif
    
    test_gcc_vector_shuffle();
    test_gcc_complex_permute();
    printf("GCC vector tests completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("SVE tests completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec tests completed\n");
#endif
    
    /* Compute checksum to ensure computations aren't optimized away */
    g_checksum = 0;
    for (int i = 0; i < 16; i++) {
        g_checksum += (int)(g_result[i] & 0xFF);
    }
    
    printf("Checksum: %d\n", g_checksum);
    
    return g_checksum != 0 ? 0 : 1;
}
