/* Test program to cover 10-11 operand RTL expansion cases in optabs.cc */
/* Compile with: -O2 -ftree-vectorize -fdump-rtl-expand */
/* For AVX-512: add -mavx512f -mavx512vl -march=skylake-avx512 */
/* For SVE: add -march=armv8-a+sve */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t g_result[16] = {0};
volatile int g_checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle that may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, v8di c, v8di d, 
                            v8di indices1, v8di indices2) {
    /* Complex permutation expression that may require many operands */
    v8di temp1 = __builtin_shufflevector(a, b, 
        0, 9, 2, 11, 4, 13, 6, 15);
    v8di temp2 = __builtin_shufflevector(c, d,
        8, 1, 10, 3, 12, 5, 14, 7);
    
    /* Blend based on mask - complex expression */
    v8di mask = (indices1 > indices2);
    v8di result = (mask & temp1) | (~mask & temp2);
    
    /* Store to prevent optimization */
    memcpy((void*)g_result, &result, sizeof(result));
}

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask uses many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, 
                        __mmask16 mask, __m512i c, __m512i d) {
    /* First permutation: a, idx, b */
    __m512i perm1 = _mm512_permutex2var_epi32(a, idx, b);
    
    /* Second permutation with mask: c, idx, d */
    __m512i perm2 = _mm512_mask_permutex2var_epi32(c, mask, idx, d);
    
    /* Blend them - creates complex expression */
    __m512i result = _mm512_mask_blend_epi32(mask, perm1, perm2);
    
    /* Store to volatile global */
    _mm512_storeu_si512((void*)g_result, result);
}

/* Test AVX-512 gather with many operands */
void test_avx512_gather(__m512i base, __m512i index, __mmask16 mask) {
    /* Gather with scale 4 */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // indices
        (const void*)&base,      // base addr
        4                        // scale
    );
    
    _mm512_storeu_si512((void*)g_result, gathered);
}
#endif /* __AVX512F__ */

/* ==================== AArch64 SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with predicate, base, and offset - many operands */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* Gather operation with predicate */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, &base, offsets);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)g_result, gathered);
}

/* SVE complex arithmetic with multiple operands */
void test_sve_complex(svint64_t a, svint64_t b, svint64_t c, 
                     svint64_t d, svbool_t pg) {
    /* FMA-like chain: a * b + c * d */
    svint64_t mul1 = svmul_s64_z(pg, a, b);
    svint64_t mul2 = svmul_s64_z(pg, c, d);
    svint64_t result = svadd_s64_z(pg, mul1, mul2);
    
    svst1_s64(pg, (int64_t*)g_result, result);
}
#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* vec_perm with three vectors - may expand to many operands */
void test_altivec_perm(vector signed long long a,
                      vector signed long long b,
                      vector unsigned char perm) {
    /* vec_perm takes two vectors and permute control */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_st(result, 0, (vector signed long long*)g_result);
}
#endif /* __ALTIVEC__ */

/* ==================== Main Test Driver ==================== */

int main() {
    /* Initialize test data */
    int64_t data_a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int64_t data_b[8] = {8, 9, 10, 11, 12, 13, 14, 15};
    int64_t data_c[8] = {16, 17, 18, 19, 20, 21, 22, 23};
    int64_t data_d[8] = {24, 25, 26, 27, 28, 29, 30, 31};
    int64_t indices[8] = {0, 2, 4, 6, 1, 3, 5, 7};
    
    /* Test GCC vector extensions (always available) */
    {
        v8di va, vb, vc, vd, vidx1, vidx2;
        memcpy(&va, data_a, sizeof(va));
        memcpy(&vb, data_b, sizeof(vb));
        memcpy(&vc, data_c, sizeof(vc));
        memcpy(&vd, data_d, sizeof(vd));
        memcpy(&vidx1, indices, sizeof(vidx1));
        memcpy(&vidx2, indices + 4, sizeof(vidx2));
        
        test_gcc_vector_shuffle(va, vb, vc, vd, vidx1, vidx2);
        
        /* Update checksum */
        for (int i = 0; i < 8; i++) {
            g_checksum += g_result[i];
        }
    }
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512i a = _mm512_loadu_si512(data_a);
        __m512i b = _mm512_loadu_si512(data_b);
        __m512i idx = _mm512_loadu_si512(indices);
        __m512i c = _mm512_loadu_si512(data_c);
        __m512i d = _mm512_loadu_si512(data_d);
        __mmask16 mask = 0xAAAA;  /* Alternating bits */
        
        test_avx512_permute(a, b, idx, mask, c, d);
        
        /* Update checksum */
        for (int i = 0; i < 8; i++) {
            g_checksum += g_result[i];
        }
        
        /* Test gather */
        test_avx512_gather(a, idx, mask);
        for (int i = 0; i < 8; i++) {
            g_checksum += g_result[i];
        }
    }
#endif
    
    /* Test SVE if available */
#ifdef __ARM_FEATURE_SVE
    {
        /* Note: SVE vector length is runtime determined */
        svbool_t pg = svptrue_b64();
        svint64_t a = svld1_s64(pg, data_a);
        svint64_t b = svld1_s64(pg, data_b);
        svint64_t c = svld1_s64(pg, data_c);
        svint64_t d = svld1_s64(pg, data_d);
        svint64_t offsets = svld1_s64(pg, indices);
        
        test_sve_gather(a, offsets, pg);
        for (int i = 0; i < 8; i++) {
            g_checksum += g_result[i];
        }
        
        test_sve_complex(a, b, c, d, pg);
        for (int i = 0; i < 8; i++) {
            g_checksum += g_result[i];
        }
    }
#endif
    
    /* Test Altivec if available */
#ifdef __ALTIVEC__
    {
        vector signed long long a = vec_ld(0, (vector signed long long*)data_a);
        vector signed long long b = vec_ld(0, (vector signed long long*)data_b);
        vector unsigned char perm = {0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23};
        
        test_altivec_perm(a, b, perm);
        for (int i = 0; i < 2; i++) {
            g_checksum += g_result[i];
        }
    }
#endif
    
    printf("Final checksum: %d\n", g_checksum);
    return g_checksum != 0 ? 0 : 1;
}
