/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle with variable indices - may expand to vec_perm with many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, int idx[16]) {
    /* Create a complex permutation pattern */
    v8di temp1 = __builtin_shufflevector(a, b, 
        idx[0], idx[1], idx[2], idx[3], 
        idx[4], idx[5], idx[6], idx[7]);
    
    v8di temp2 = __builtin_shufflevector(b, a,
        idx[8], idx[9], idx[10], idx[11],
        idx[12], idx[13], idx[14], idx[15]);
    
    /* Blend operation that might expand to many operands */
    v8di mask = (a > b);
    v8di result = (mask & temp1) | (~mask & temp2);
    
    /* Store to prevent elimination */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask - can expand to many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has 5 explicit args but expands to more */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Another complex operation to increase operand count */
    __m512i rotated = _mm512_alignr_epi32(result, result, 4);
    __m512i blended = _mm512_mask_blend_epi32(mask, result, rotated);
    
    /* Store with volatile asm to prevent optimization */
    asm volatile("" : "+x"(blended));
    _mm512_store_epi64((void*)global_result, blended);
}

/* Complex blend of 4 vectors using 2 masks - may expand to high operand count */
void test_avx512_complex_blend(__m512i v1, __m512i v2, __m512i v3, __m512i v4,
                               __mmask16 m1, __mmask16 m2) {
    /* This complex expression may require many operands during RTL expansion */
    __m512i t1 = _mm512_mask_mov_epi32(v1, m1, v2);
    __m512i t2 = _mm512_mask_mov_epi32(v3, m2, v4);
    __m512i result = _mm512_mask_blend_epi32(m1 & m2, t1, t2);
    
    /* Mix with arithmetic */
    result = _mm512_add_epi32(result, _mm512_slli_epi32(result, 1));
    result = _mm512_xor_si512(result, _mm512_rol_epi32(result, 16));
    
    _mm512_store_epi64((void*)(global_result + 8), result);
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments - can have many operands */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64 has base, offsets, predicate - may expand further */
    svint64_t data = svld1_gather_s64(pg, (const int64_t*)global_result, offsets);
    
    /* Complex operation with the gathered data */
    svint64_t scaled = svmul_s64_z(pg, data, svdup_s64(2));
    svint64_t result = svadd_s64_z(pg, base, scaled);
    
    /* Store with scatter - another multi-operand operation */
    svst1_scatter_s64(pg, (int64_t*)global_result, offsets, result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* vec_perm with three vectors and complex mask */
void test_altivec_permute(vector signed long long a,
                          vector signed long long b,
                          vector signed long long c,
                          vector unsigned char mask) {
    /* vec_perm with 3 sources by using two vec_perms */
    vector signed long long ab = vec_perm(a, b, mask);
    vector signed long long bc = vec_perm(b, c, mask);
    
    /* Blend based on comparison */
    vector bool long long cmp = vec_cmpgt(a, b);
    vector signed long long result = vec_sel(ab, bc, cmp);
    
    /* Store to global */
    vec_st(result, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main() {
    /* Initialize test data */
    int64_t data_a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int64_t data_b[8] = {8, 9, 10, 11, 12, 13, 14, 15};
    int indices[16] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15};
    
    /* Test GCC vector extensions (always available) */
    {
        v8di va, vb;
        memcpy(&va, data_a, sizeof(va));
        memcpy(&vb, data_b, sizeof(vb));
        test_gcc_vector_shuffle(va, vb, indices);
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 intrinsics */
    {
        __m512i a = _mm512_loadu_epi64(data_a);
        __m512i b = _mm512_loadu_epi64(data_b);
        __m512i idx = _mm512_set_epi32(7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8);
        __mmask16 mask = 0xAAAA;
        
        test_avx512_permute(a, b, idx, mask);
        
        /* Test complex blend */
        __m512i c = _mm512_add_epi64(a, _mm512_set1_epi64(16));
        __m512i d = _mm512_add_epi64(b, _mm512_set1_epi64(16));
        test_avx512_complex_blend(a, b, c, d, 0xAAAA, 0x5555);
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    /* Test SVE intrinsics */
    {
        svbool_t pg = svwhilelt_b64(0, 8);
        svint64_t base = svld1_s64(pg, data_a);
        svint64_t offsets = svindex_s64(0, 1);
        
        test_sve_gather(base, offsets, pg);
    }
#endif
    
#ifdef __ALTIVEC__
    /* Test Altivec/VSX */
    {
        vector signed long long a = vec_ld(0, (vector signed long long*)data_a);
        vector signed long long b = vec_ld(0, (vector signed long long*)data_b);
        vector signed long long c = vec_add(a, vec_splats((long long)8));
        vector unsigned char mask = {0,1,2,3,16,17,18,19,4,5,6,7,20,21,22,23};
        
        test_altivec_permute(a, b, c, mask);
    }
#endif
    
    /* Compute checksum to ensure all operations executed */
    for (int i = 0; i < 16; i++) {
        checksum += (int)(global_result[i] & 0xFF);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
