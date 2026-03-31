/* Test program to cover 10-11 operand RTL expansion cases in optabs.cc */
/* Compile with: -O2 -ftree-vectorize -fdump-rtl-expand */
/* For AVX-512: add -mavx512f -mavx512vl */
/* For SVE: add -march=armv8-a+sve */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector permutation that may trigger vec_perm expansion */

typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, const int *indices) {
    /* Create a complex shuffle operation that might need many operands */
    v8di temp1 = __builtin_shufflevector(a, b, 
        indices[0], indices[1], indices[2], indices[3],
        indices[4], indices[5], indices[6], indices[7]);
    
    v8di temp2 = __builtin_shufflevector(b, a,
        indices[7], indices[6], indices[5], indices[4],
        indices[3], indices[2], indices[1], indices[0]);
    
    /* Blend operation that might expand to many operands */
    v8di mask = (temp1 > temp2);
    v8di result = (mask & temp1) | (~mask & temp2);
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== x86_64 AVX-512 ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permute with mask - can generate many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has many operands:
       dest, mask, idx, a, b -> potentially expands to 10+ operands */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, a, b);
    
    /* Additional complex operation to increase operand count */
    __m512i rotated = _mm512_rol_epi32(result, 4);
    __m512i blended = _mm512_mask_blend_epi32(mask, result, rotated);
    
    /* Store to volatile global */
    _mm512_storeu_si512((void*)global_result, blended);
}

/* Another AVX-512 test with blend of multiple vectors */
void test_avx512_complex_blend(__m512i v1, __m512i v2, __m512i v3, 
                               __m512i v4, __mmask16 m1, __mmask16 m2) {
    /* Complex expression that might expand to many operands */
    __m512i t1 = _mm512_mask_add_epi32(v1, m1, v2, v3);
    __m512i t2 = _mm512_mask_sub_epi32(v2, m2, v3, v4);
    __m512i t3 = _mm512_mask_mullo_epi32(v3, m1, v4, v1);
    
    /* Triple blend - might need many operands */
    __m512i blend1 = _mm512_mask_blend_epi32(m1, t1, t2);
    __m512i blend2 = _mm512_mask_blend_epi32(m2, blend1, t3);
    __m512i final = _mm512_mask_blend_epi32(m1 | m2, blend2, v4);
    
    _mm512_storeu_si512((void*)(global_result + 8), final);
}
#endif

/* ==================== AArch64 SVE ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments - can have many operands */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64 has base, offsets, predicate -> may expand */
    svint64_t data = svld1_gather_s64(pg, (const int64_t*)global_result, offsets);
    
    /* Complex operation with the result */
    svint64_t scaled = svmul_s64_z(pg, data, base);
    svint64_t added = svadd_s64_z(pg, scaled, offsets);
    
    /* Store back */
    svst1_scatter_s64(pg, (int64_t*)global_result, offsets, added);
}

/* SVE permute with multiple vector inputs */
void test_sve_permute(svint64_t a, svint64_t b, svint64_t indices, svbool_t pg) {
    /* Complex permutation pattern */
    svint64_t t1 = svtbl_s64(a, indices);
    svint64_t t2 = svtbl_s64(b, indices);
    
    /* Blend based on predicate */
    svint64_t blended = svsel_s64(pg, t1, t2);
    
    /* Additional operations to increase complexity */
    svint64_t rotated = svror_s64_z(pg, blended, 3);
    svint64_t final = svadd_s64_z(pg, blended, rotated);
    
    svst1_s64(pg, (int64_t*)global_result, final);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

/* PowerPC vec_perm with three vectors - may expand to many operands */
void test_powerpc_permute(vector signed long long a,
                          vector signed long long b,
                          vector unsigned char perm) {
    /* vec_perm takes two data vectors and one permute control */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Additional complex operation */
    vector signed long long rotated = vec_rl(result, (vector unsigned long long){4, 4});
    vector signed long long blended = vec_sel(result, rotated, 
        (vector unsigned long long){0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL});
    
    /* Store result */
    vec_st(blended, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */
int main() {
    /* Initialize test data */
    int indices[8] = {0, 7, 1, 6, 2, 5, 3, 4};
    
    /* Test GCC vector extensions (most portable) */
    {
        v8di a = {0, 1, 2, 3, 4, 5, 6, 7};
        v8di b = {8, 9, 10, 11, 12, 13, 14, 15};
        test_gcc_vector_shuffle(a, b, indices);
        
        /* Update checksum */
        for (int i = 0; i < 8; i++) {
            checksum += global_result[i];
        }
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    {
        __m512i a = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8,
                                     7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi32(31, 30, 29, 28, 27, 26, 25, 24,
                                     23, 22, 21, 20, 19, 18, 17, 16);
        __m512i idx = _mm512_set_epi32(1, 3, 5, 7, 9, 11, 13, 15,
                                       0, 2, 4, 6, 8, 10, 12, 14);
        __mmask16 mask = 0xAAAA;  /* Alternating bits */
        
        test_avx512_permute(a, b, idx, mask);
        
        /* Additional complex blend test */
        __m512i v3 = _mm512_set1_epi32(42);
        __m512i v4 = _mm512_set1_epi32(99);
        __mmask16 m2 = 0x5555;
        test_avx512_complex_blend(a, b, v3, v4, mask, m2);
        
        /* Update checksum */
        for (int i = 0; i < 16; i++) {
            checksum += global_result[i];
        }
    }
#endif

#ifdef __ARM_FEATURE_SVE
    /* Test SVE if available */
    {
        /* Note: SVE vector length is runtime determined */
        svbool_t pg = svptrue_b64();
        svint64_t base = svdup_s64(2);
        svint64_t offsets = svindex_s64(0, 1);
        
        test_sve_gather(base, offsets, pg);
        
        /* Update checksum */
        for (int i = 0; i < 8; i++) {
            checksum += global_result[i];
        }
    }
#endif

#ifdef __ALTIVEC__
    /* Test PowerPC if available */
    {
        vector signed long long a = {0x0123456789ABCDEFLL, 0xFEDCBA9876543210LL};
        vector signed long long b = {0x1122334455667788LL, 0x8877665544332211LL};
        vector unsigned char perm = {0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15};
        
        test_powerpc_permute(a, b, perm);
        
        /* Update checksum */
        checksum += global_result[0] + global_result[1];
    }
#endif

    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
