/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
/* Compile with: -O2 -ftree-vectorize -fdump-rtl-expand */
/* For AVX-512: -O2 -mavx512f -mavx512vl -mavx512bw -ftree-vectorize */
/* For SVE: -O3 -march=armv8-a+sve -ftree-vectorize */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle that may expand to many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, v8di c, v8di d, 
                            v8di indices, v8di mask) {
    /* Complex expression that might require many operands */
    v8di temp1 = __builtin_shufflevector(a, b, 0, 9, 2, 11, 4, 13, 6, 15);
    v8di temp2 = __builtin_shufflevector(c, d, 1, 8, 3, 10, 5, 12, 7, 14);
    
    /* Blend operation with mask - complex expression */
    v8di result = (mask & temp1) | (~mask & temp2);
    
    /* Store to global to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== AVX-512 Specific Tests ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permute with many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has many operands when expanded */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Another complex operation */
    __m512i result2 = _mm512_mask_permutex2var_epi64(
        result, 0xFF, _mm512_set1_epi64(2), b, result);
    
    /* Store results */
    _mm512_storeu_si512((void*)global_result, result2);
}

/* AVX-512 blend with multiple sources */
void test_avx512_blend(__m512i a, __m512i b, __m512i c, __m512i d, __mmask16 k1, __mmask16 k2) {
    /* Complex blend expression that might expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(k1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(k2, c, d);
    __m512i result = _mm512_mask_blend_epi32(k1 | k2, t1, t2);
    
    /* Additional permutation */
    __m512i idx = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    result = _mm512_permutexvar_epi32(idx, result);
    
    _mm512_storeu_si512((void*)global_result, result);
}
#endif

/* ==================== ARM SVE Specific Tests ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with many operands */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64 expands to many operands */
    svint64_t result = svld1_gather_s64(pg, &global_result[0], offsets);
    
    /* Store back with scatter - another multi-operand operation */
    svst1_scatter_s64(pg, &global_result[0], offsets, result);
    
    /* Complex expression with predicate */
    svint64_t added = svadd_s64_z(pg, result, base);
    svst1_scatter_s64(pg, &global_result[0], offsets, added);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_perm(vector signed long long a, 
                      vector signed long long b,
                      vector unsigned char perm) {
    /* vec_perm with three vectors expands to many operands */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_st(result, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main() {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Initialize test data */
    int64_t data_a[8] = {0,1,2,3,4,5,6,7};
    int64_t data_b[8] = {8,9,10,11,12,13,14,15};
    int64_t data_c[8] = {16,17,18,19,20,21,22,23};
    int64_t data_d[8] = {24,25,26,27,28,29,30,31};
    int64_t indices[8] = {0,2,4,6,1,3,5,7};
    int64_t mask_data[8] = {0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF, 0,
                           0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF, 0};
    
    /* Test GCC vector extensions (always available) */
    {
        v8di va = *(v8di*)data_a;
        v8di vb = *(v8di*)data_b;
        v8di vc = *(v8di*)data_c;
        v8di vd = *(v8di*)data_d;
        v8di vidx = *(v8di*)indices;
        v8di vmask = *(v8di*)mask_data;
        
        test_gcc_vector_shuffle(va, vb, vc, vd, vidx, vmask);
        
        /* Compute checksum */
        for (int i = 0; i < 8; i++) {
            checksum += global_result[i];
        }
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    {
        __m512i a = _mm512_loadu_si512(data_a);
        __m512i b = _mm512_loadu_si512(data_b);
        __m512i c = _mm512_loadu_si512(data_c);
        __m512i d = _mm512_loadu_si512(data_d);
        __m512i idx = _mm512_loadu_si512(indices);
        __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
        
        test_avx512_permute(a, b, idx, mask);
        test_avx512_blend(a, b, c, d, mask, 0x5555);
        
        /* Update checksum */
        for (int i = 0; i < 8; i++) {
            checksum += global_result[i];
        }
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    /* Test SVE if available */
    {
        svbool_t pg = svwhilelt_b64(0, 8);
        svint64_t base = svld1_s64(pg, data_a);
        svint64_t offsets = svld1_s64(pg, indices);
        
        test_sve_gather(base, offsets, pg);
        
        /* Update checksum */
        for (int i = 0; i < 8; i++) {
            checksum += global_result[i];
        }
    }
#endif
    
#ifdef __ALTIVEC__
    /* Test Altivec if available */
    {
        vector signed long long a = *(vector signed long long*)data_a;
        vector signed long long b = *(vector signed long long*)data_b;
        vector unsigned char perm = {0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23};
        
        test_altivec_perm(a, b, perm);
        
        /* Update checksum */
        for (int i = 0; i < 2; i++) {
            checksum += global_result[i];
        }
    }
#endif
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
