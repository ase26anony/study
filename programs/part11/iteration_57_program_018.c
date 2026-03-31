/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int g_result_int[64] = {0};
volatile long long g_result_ll[32] = {0};

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with mask - likely to generate many operands */
void test_avx512_permute(void) {
    /* Initialize vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    /* Create a complex mask with alternating pattern */
    __mmask16 mask = 0xAAAA; /* 1010101010101010 binary */
    
    /* This intrinsic takes 5 arguments but expands to many operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec2, vec1);
    
    /* Another permutation with different sources */
    __m512i idx2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i result2 = _mm512_mask_permutex2var_epi32(result, 0x5555, idx2, vec2, vec1);
    
    /* Store results to prevent optimization */
    _mm512_storeu_si512((void*)g_result_int, result);
    _mm512_storeu_si512((void*)(g_result_int + 16), result2);
    
    /* Complex blend operation that might expand to many operands */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    /* Create complex mask pattern */
    __mmask16 m1 = 0xF0F0;
    __mmask16 m2 = 0x0FF0;
    
    /* This should generate complex RTL with many operands */
    __m512i blend1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i blend2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i final_blend = _mm512_mask_blend_epi32(0xAAAA, blend1, blend2);
    
    _mm512_storeu_si512((void*)(g_result_int + 32), final_blend);
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector implementation using GCC extensions */
typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable index array to prevent constant folding */
    volatile int idx_array[16];
    for (int i = 0; i < 16; i++) {
        idx_array[i] = (i * 3) % 32;
    }
    
    /* Complex permutation using __builtin_shuffle */
    v16si perm_result;
    for (int i = 0; i < 16; i++) {
        int idx = idx_array[i];
        if (idx < 16) {
            perm_result[i] = a[idx];
        } else {
            perm_result[i] = b[idx - 16];
        }
    }
    
    /* Store result */
    memcpy((void*)g_result_int, &perm_result, sizeof(perm_result));
    
    /* Another complex operation: conditional blend */
    v16si mask1 = {0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1};
    v16si mask2 = {-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0};
    
    v16si x = a;
    v16si y = b;
    v16si z = {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
    v16si w = {48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
    
    /* Complex blend expression that might expand to many operands */
    v16si blend_result = (mask1 & x) | (~mask1 & mask2 & y) | 
                         (~mask1 & ~mask2 & z) | (mask1 & mask2 & w);
    
    memcpy((void*)(g_result_int + 16), &blend_result, sizeof(blend_result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    svbool_t pg = svptrue_b64();
    svint64_t base = svdup_s64(0);
    
    /* Create offset vector */
    int64_t offsets[4] = {0, 8, 16, 24};
    svint64_t offset_vec = svld1_s64(pg, offsets);
    
    /* This gather operation expands to many RTL operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, (const int64_t*)g_result_ll, offset_vec);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)g_result_ll, gathered);
    
    /* Complex scatter operation */
    svint64_t data = svdup_s64(0x123456789ABCDEF);
    svst1_scatter_s64offset_s64(pg, (int64_t*)g_result_ll, offset_vec, data);
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    /* vec_perm with three vectors can expand to many operands */
    vector unsigned char a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char perm = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    /* vec_perm takes 3 operands but may expand further */
    vector unsigned char result = vec_perm(a, b, perm);
    
    /* Store result */
    memcpy((void*)g_result_int, &result, sizeof(result));
}
#endif

/* ==================== Main Function ==================== */
int main() {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 64; i++) {
        g_result_int[i] = i;
    }
    for (int i = 0; i < 32; i++) {
        g_result_ll[i] = i * 2LL;
    }
    
    /* Call architecture-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    printf("AVX-512 test completed\n");
#endif
    
    test_gcc_vector_shuffle();
    printf("GCC vector shuffle test completed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("ARM SVE test completed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivc_permute();
    printf("PowerPC Altivec test completed\n");
#endif
    
    /* Compute checksum to ensure operations weren't optimized away */
    long long checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += g_result_int[i];
    }
    for (int i = 0; i < 32; i++) {
        checksum += g_result_ll[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
