/* Test program to cover optabs.cc lines 8254-8263 */
/* Compile with: -O2 -ftree-vectorize -fdump-rtl-expand */
/* For AVX-512: add -mavx512f -mavx512vl */
/* For SVE: add -march=armv8-a+sve */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int global_checksum = 0;

/* ========== GCC Vector Extensions (Portable) ========== */
#ifdef __GNUC__

typedef int32_t v8si __attribute__((vector_size(32)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v2di __attribute__((vector_size(16)));

/* Complex shuffle that may expand to vec_perm with many operands */
__attribute__((noinline))
void test_gcc_vector_shuffle(v8si a, v8si b, v8si c, v8si d, 
                             v8si *out1, v8si *out2) {
    /* Create a complex expression that might need many operands */
    v8si idx1 = {0, 8, 2, 10, 4, 12, 6, 14};
    v8si idx2 = {1, 9, 3, 11, 5, 13, 7, 15};
    
    /* Multiple shuffle operations combined */
    v8si tmp1 = __builtin_shufflevector(a, b, 0, 8, 2, 10, 4, 12, 6, 14);
    v8si tmp2 = __builtin_shufflevector(c, d, 1, 9, 3, 11, 5, 13, 7, 15);
    
    /* Blend operation that might expand further */
    v8si mask = {0, -1, 0, -1, 0, -1, 0, -1};
    *out1 = (tmp1 & ~mask) | (tmp2 & mask);
    
    /* Another complex permutation */
    v8si idx3 = {7, 6, 5, 4, 3, 2, 1, 0};
    *out2 = __builtin_shufflevector(*out1, *out1, 
                                    7, 6, 5, 4, 3, 2, 1, 0);
}

/* Test with 64-bit vectors for different expansion */
__attribute__((noinline))
void test_gcc_v4di_permute(v4di a, v4di b, v4di c, v4di *out) {
    /* Variable indices to prevent constant folding */
    volatile int idx0 = 0, idx1 = 1, idx2 = 2, idx3 = 3;
    volatile int idx4 = 4, idx5 = 5, idx6 = 6, idx7 = 7;
    
    /* This shuffle with variable indices may need general expansion */
    *out = __builtin_shufflevector(a, b, 
                                   idx0, idx1, idx2, idx3,
                                   idx4, idx5, idx6, idx7);
}

#endif /* __GNUC__ */

/* ========== x86 AVX-512 Intrinsics ========== */
#ifdef __AVX512F__

#include <immintrin.h>

__attribute__((noinline))
void test_avx512_permutex2var(__m512i a, __m512i b, __m512i idx, 
                              __mmask16 mask, __m512i *result) {
    /* _mm512_mask_permutex2var_epi32 has many operands:
       result, src, mask, idx, a, b
       This may expand to an RTL pattern with 10+ operands */
    *result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
}

__attribute__((noinline))
void test_avx512_fmadd_combine(__m512 a, __m512 b, __m512 c, 
                               __m512 d, __m512 *out1, __m512 *out2) {
    /* Complex expression combining multiple FMAs */
    __m512 tmp1 = _mm512_fmadd_ps(a, b, c);
    __m512 tmp2 = _mm512_fmadd_ps(b, c, d);
    __m512 tmp3 = _mm512_fmadd_ps(c, d, a);
    
    /* Blend with mask - may expand to many operands */
    __mmask16 mask = 0xAAAA;  /* Alternating bits */
    *out1 = _mm512_mask_blend_ps(mask, tmp1, tmp2);
    
    /* Another blend */
    *out2 = _mm512_mask_blend_ps(~mask, tmp3, *out1);
}

#endif /* __AVX512F__ */

/* ========== ARM SVE Intrinsics ========== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* SVE gather operations can have many operands */
__attribute__((noinline))
void test_sve_gather(svint64_t base, svint64_t offsets, 
                     svbool_t pg, svint64_t *result) {
    /* svld1_gather_s64 has: predicate, base, offsets
       Plus the result operand */
    *result = svld1_gather_s64(pg, &base, offsets);
}

/* SVE complex arithmetic */
__attribute__((noinline))
void test_sve_complex(svfloat64_t a, svfloat64_t b, 
                      svfloat64_t c, svfloat64_t d,
                      svbool_t pg, svfloat64_t *out) {
    /* FMA with predicate has many operands */
    svfloat64_t tmp = svmla_f64_z(pg, a, b, c);
    *out = svmad_f64_z(pg, tmp, d, a);
}

#endif /* __ARM_FEATURE_SVE */

/* ========== PowerPC Altivec/VSX ========== */
#ifdef __ALTIVEC__

#include <altivec.h>

__attribute__((noinline))
void test_altivec_permute(vector signed int a, vector signed int b,
                          vector unsigned char perm, vector signed int *out) {
    /* vec_perm with three vector arguments */
    *out = vec_perm(a, b, perm);
    
    /* Complex expression with multiple vec_perm */
    vector unsigned char perm2 = {16,17,18,19,20,21,22,23,
                                  24,25,26,27,28,29,30,31};
    vector signed int tmp = vec_perm(a, b, perm2);
    *out = vec_add(*out, tmp);
}

#endif /* __ALTIVEC__ */

/* ========== Main Test Driver ========== */
int main() {
    int checksum = 0;
    
    /* Test GCC vector extensions (always available with GCC) */
#ifdef __GNUC__
    {
        v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
        v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
        v8si c = {17, 18, 19, 20, 21, 22, 23, 24};
        v8si d = {25, 26, 27, 28, 29, 30, 31, 32};
        v8si out1, out2;
        
        test_gcc_vector_shuffle(a, b, c, d, &out1, &out2);
        
        /* Compute checksum */
        for (int i = 0; i < 8; i++) {
            checksum += out1[i] + out2[i];
        }
        
        /* Test 64-bit vector permutation */
        v4di a64 = {1, 2, 3, 4};
        v4di b64 = {5, 6, 7, 8};
        v4di out64;
        test_gcc_v4di_permute(a64, b64, a64, &out64);
        
        for (int i = 0; i < 4; i++) {
            checksum += out64[i];
        }
    }
#endif
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512i a = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
        __m512i b = _mm512_set_epi32(17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32);
        __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __m512i result;
        __mmask16 mask = 0xFFFF;
        
        test_avx512_permutex2var(a, b, idx, mask, &result);
        
        /* Store to volatile global to prevent optimization */
        _mm512_store_epi64((void*)global_result, result);
        
        /* Test floating point combination */
        __m512 fa = _mm512_set1_ps(1.0f);
        __m512 fb = _mm512_set1_ps(2.0f);
        __m512 fc = _mm512_set1_ps(3.0f);
        __m512 fd = _mm512_set1_ps(4.0f);
        __m512 fout1, fout2;
        
        test_avx512_fmadd_combine(fa, fb, fc, fd, &fout1, &fout2);
        
        /* Update checksum */
        int64_t *r = (int64_t*)&result;
        for (int i = 0; i < 8; i++) {
            checksum += (int)r[i];
        }
    }
#endif
    
    /* Test SVE if available */
#ifdef __ARM_FEATURE_SVE
    {
        /* Simple test - actual SVE code would need proper initialization */
        svint64_t base = svdup_s64(0);
        svint64_t offsets = svdup_s64(0);
        svbool_t pg = svptrue_b64();
        svint64_t result;
        
        test_sve_gather(base, offsets, pg, &result);
        
        /* Update checksum */
        int64_t tmp[2];
        svst1_s64(pg, tmp, result);
        checksum += (int)(tmp[0] + tmp[1]);
    }
#endif
    
    /* Test Altivec if available */
#ifdef __ALTIVEC__
    {
        vector signed int a = {1, 2, 3, 4};
        vector signed int b = {5, 6, 7, 8};
        vector unsigned char perm = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        vector signed int out;
        
        test_altivec_permute(a, b, perm, &out);
        
        /* Update checksum */
        int *p = (int*)&out;
        for (int i = 0; i < 4; i++) {
            checksum += p[i];
        }
    }
#endif
    
    global_checksum = checksum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
