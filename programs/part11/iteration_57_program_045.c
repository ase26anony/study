/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_results[64] = {0};
volatile int global_index = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle that may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, v8di mask, int *indices) {
    /* Force variable indices to prevent constant folding */
    volatile int idx0 = indices[0];
    volatile int idx1 = indices[1];
    volatile int idx2 = indices[2];
    volatile int idx3 = indices[3];
    volatile int idx4 = indices[4];
    volatile int idx5 = indices[5];
    volatile int idx6 = indices[6];
    volatile int idx7 = indices[7];
    
    /* Complex expression that may require multi-operand expansion */
    v8di temp1 = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    
    /* Another shuffle with different indices */
    v8di temp2 = __builtin_shufflevector(b, a,
        idx7, idx6, idx5, idx4, idx3, idx2, idx1, idx0);
    
    /* Blend based on mask - complex expression */
    v8di result = (mask > 0) ? temp1 : temp2;
    
    /* Store to prevent elimination */
    for (int i = 0; i < 8; i++) {
        global_results[global_index++] = result[i];
    }
}

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask has many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has 5 explicit args but expands to many RTL operands */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Another complex operation */
    __m512i result2 = _mm512_mask_permutex2var_epi64(
        result, 0xFF, _mm512_set1_epi64(2), b, result);
    
    /* Store results */
    _mm512_storeu_epi64((void*)&global_results[global_index], result2);
    global_index += 8;
    
    /* Test the 32-bit version too */
    __m512i a32 = _mm512_set1_epi32(1);
    __m512i b32 = _mm512_set1_epi32(2);
    __m512i idx32 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i res32 = _mm512_mask_permutex2var_epi32(a32, 0xAAAA, idx32, b32, a32);
    _mm512_storeu_epi32((void*)&global_results[global_index], res32);
    global_index += 16;
}

/* Complex blend of 4 vectors with 2 masks */
void test_avx512_complex_blend(__m512i v1, __m512i v2, __m512i v3, __m512i v4,
                               __mmask16 m1, __mmask16 m2) {
    /* This complex expression may expand to many operands */
    __m512i temp1 = _mm512_mask_mov_epi32(v1, m1, v2);
    __m512i temp2 = _mm512_mask_mov_epi32(v3, m2, v4);
    __m512i result = _mm512_mask_blend_epi32(m1 & m2, temp1, temp2);
    
    /* More operations to increase complexity */
    result = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    result = _mm512_slli_epi32(result, 2);
    
    _mm512_storeu_epi32((void*)&global_results[global_index], result);
    global_index += 16;
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments */
void test_sve_gather(int64_t *base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64 has base, offsets, predicate - may expand to many operands */
    svint64_t result = svld1_gather_s64(pg, base, offsets);
    
    /* Store with scatter - another multi-operand operation */
    svst1_scatter_s64(pg, &global_results[global_index], offsets, result);
    global_index += svcntd(); /* Increment by number of 64-bit lanes */
}

/* SVE complex arithmetic with multiple operands */
void test_sve_complex(svint64_t a, svint64_t b, svint64_t c, svint64_t d,
                      svbool_t p1, svbool_t p2) {
    /* Complex expression that may require many RTL operands */
    svint64_t temp1 = svadd_m(p1, a, b);
    svint64_t temp2 = svadd_m(p2, c, d);
    svint64_t result = svmla_m(p1, temp1, temp2, svdup_n_s64(2));
    
    /* Store result */
    svst1_s64(p1, &global_results[global_index], result);
    global_index += svcntd();
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* vec_perm with three vectors and complex index calculation */
void test_altivec_perm(vector signed long long a,
                       vector signed long long b,
                       vector unsigned char perm) {
    /* vec_perm takes 3 vectors, may expand further */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_vsx_st(result, 0, (vector signed long long*)&global_results[global_index]);
    global_index += 2;
    
    /* More complex: permute with computed indices */
    vector unsigned char perm2 = vec_add(perm, vec_splats((unsigned char)1));
    vector signed long long result2 = vec_perm(b, a, perm2);
    vec_vsx_st(result2, 0, (vector signed long long*)&global_results[global_index]);
    global_index += 2;
}
#endif

/* ==================== Main Test Driver ==================== */

int main() {
    int indices[8] = {0, 9, 2, 11, 4, 13, 6, 15}; /* Cross-lane indices */
    
    /* Test GCC vector extensions (always available) */
    {
        v8di a = {0,1,2,3,4,5,6,7};
        v8di b = {8,9,10,11,12,13,14,15};
        v8di mask = {1,0,1,0,1,0,1,0};
        test_gcc_vector_shuffle(a, b, mask, indices);
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 intrinsics */
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
        __m512i idx = _mm512_set_epi32(30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0);
        __mmask16 mask = 0xAAAA;
        
        test_avx512_permute(a, b, idx, mask);
        
        /* Test complex blend */
        __m512i v1 = _mm512_set1_epi32(1);
        __m512i v2 = _mm512_set1_epi32(2);
        __m512i v3 = _mm512_set1_epi32(3);
        __m512i v4 = _mm512_set1_epi32(4);
        test_avx512_complex_blend(v1, v2, v3, v4, 0x5555, 0xAAAA);
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    /* Test SVE intrinsics */
    {
        int64_t base_array[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        svint64_t offsets = svindex_s64(0, 2); /* 0, 2, 4, ... */
        svbool_t pg = svptrue_b64();
        
        test_sve_gather(base_array, offsets, pg);
        
        /* Test complex SVE operations */
        svint64_t a = svdup_n_s64(1);
        svint64_t b = svdup_n_s64(2);
        svint64_t c = svdup_n_s64(3);
        svint64_t d = svdup_n_s64(4);
        test_sve_complex(a, b, c, d, pg, pg);
    }
#endif
    
#ifdef __ALTIVEC__
    /* Test Altivec/VSX */
    {
        vector signed long long a = {1, 2};
        vector signed long long b = {3, 4};
        vector unsigned char perm = {0,1,2,3,4,5,6,7, 16,17,18,19,20,21,22,23};
        
        test_altivec_perm(a, b, perm);
    }
#endif
    
    /* Compute checksum to ensure all operations executed */
    int64_t checksum = 0;
    for (int i = 0; i < global_index && i < 64; i++) {
        checksum += global_results[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    printf("Total operations stored: %d\n", global_index);
    
    return 0;
}
