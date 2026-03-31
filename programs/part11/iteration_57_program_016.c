/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int global_checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

#ifdef __GNUC__
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, int *indices) {
    /* Create a complex permutation pattern */
    v8di result;
    
    /* Use __builtin_shuffle with variable indices - forces runtime permutation */
    /* This may expand to a vec_perm RTL with many operands */
    result = __builtin_shuffle(a, b, 
        (v8di){indices[0], indices[1], indices[2], indices[3],
               indices[4], indices[5], indices[6], indices[7]});
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Complex blend operation with multiple vectors */
void test_gcc_complex_blend(v4di a, v4di b, v4di c, v4di d, v4di mask1, v4di mask2) {
    /* Complex expression that might expand to multi-operand RTL */
    v4di result = (mask1 & a) | (~mask1 & mask2 & b) | 
                  (~mask1 & ~mask2 & c) | (mask1 & mask2 & d);
    
    /* Force computation */
    memcpy((void*)(global_result + 8), &result, sizeof(result));
}
#endif

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permute with mask - can generate many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has many parameters when expanded */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Alternative with even more operands: blend of three vectors */
    __m512i result2 = _mm512_mask_blend_epi32(mask, a, 
        _mm512_mask_blend_epi32(mask >> 1, b, a));
    
    /* Store results */
    _mm512_storeu_si512((void*)global_result, result);
    _mm512_storeu_si512((void*)(global_result + 8), result2);
}

/* Complex AVX-512 gather operation */
void test_avx512_gather(__m512i base, __m512i index, __mmask16 mask) {
    /* Gather with scale - may expand to many operands */
    __m512i result = _mm512_mask_i64gather_epi64(_mm512_setzero_si512(), 
                                                mask, index, 
                                                (const void*)global_result, 8);
    
    /* Store */
    _mm512_storeu_si512((void*)global_result, result);
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* SVE gather can have many operands when expanded */
    svint64_t result = svld1_gather_s64_index(pg, (const int64_t*)global_result, 
                                             svadd_x(pg, base, offsets));
    
    /* Store using scatter - another multi-operand operation */
    svst1_scatter_s64_index(pg, (int64_t*)global_result, 
                           svadd_x(pg, base, offsets), result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* vec_perm with three vectors - may expand further */
void test_altivec_perm(vector signed long long a, 
                       vector signed long long b,
                       vector unsigned char perm) {
    /* vec_perm with variable permutation vector */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Complex permute of permute */
    vector signed long long result2 = vec_perm(result, a, perm);
    
    /* Store */
    vec_st(result, 0, (vector signed long long*)global_result);
    vec_st(result2, 16, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main() {
    /* Initialize test data */
    int64_t data_a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int64_t data_b[8] = {8, 9, 10, 11, 12, 13, 14, 15};
    int indices[8] = {0, 8, 2, 10, 4, 12, 6, 14}; /* Interleave a and b */
    
    /* Test GCC vector extensions */
#ifdef __GNUC__
    {
        v8di va, vb;
        memcpy(&va, data_a, sizeof(va));
        memcpy(&vb, data_b, sizeof(vb));
        test_gcc_vector_shuffle(va, vb, indices);
        
        /* Test complex blend */
        v4di v1, v2, v3, v4, m1, m2;
        for (int i = 0; i < 4; i++) {
            ((int64_t*)&v1)[i] = i;
            ((int64_t*)&v2)[i] = i + 4;
            ((int64_t*)&v3)[i] = i + 8;
            ((int64_t*)&v4)[i] = i + 12;
            ((int64_t*)&m1)[i] = (i % 2) ? -1 : 0;
            ((int64_t*)&m2)[i] = (i < 2) ? -1 : 0;
        }
        test_gcc_complex_blend(v1, v2, v3, v4, m1, m2);
    }
#endif
    
    /* Test AVX-512 */
#ifdef __AVX512F__
    {
        __m512i va = _mm512_loadu_si512(data_a);
        __m512i vb = _mm512_loadu_si512(data_b);
        __m512i vidx = _mm512_set_epi32(14, 13, 12, 11, 10, 9, 8, 7,
                                        6, 5, 4, 3, 2, 1, 0, 15);
        __mmask16 mask = 0xAAAA; /* Alternating bits */
        
        test_avx512_permute(va, vb, vidx, mask);
        test_avx512_gather(va, vidx, mask);
    }
#endif
    
    /* Test ARM SVE */
#ifdef __ARM_FEATURE_SVE
    {
        /* Create SVE vectors - width unknown at compile time */
        svint64_t base = svdup_s64(0);
        svint64_t offsets = svindex_s64(0, 1);
        svbool_t pg = svwhilelt_b64(0, svcntd());
        
        test_sve_gather(base, offsets, pg);
    }
#endif
    
    /* Test PowerPC Altivec */
#ifdef __ALTIVEC__
    {
        vector signed long long va = vec_ld(0, (vector signed long long*)data_a);
        vector signed long long vb = vec_ld(0, (vector signed long long*)data_b);
        vector unsigned char perm = {0,1,2,3,4,5,6,7, 16,17,18,19,20,21,22,23};
        
        test_altivec_perm(va, vb, perm);
    }
#endif
    
    /* Compute checksum to ensure all operations executed */
    for (int i = 0; i < 16; i++) {
        global_checksum += (int)(global_result[i] & 0xFF);
    }
    
    printf("Checksum: %d\n", global_checksum);
    return global_checksum != 0 ? 0 : 1;
}
