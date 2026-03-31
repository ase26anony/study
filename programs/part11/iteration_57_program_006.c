/* Test program to trigger 10-11 operand RTL expansions in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int checksum = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

#ifdef __GNUC__
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle that may expand to many operands */
void test_gcc_vector_shuffle(v8di a, v8di b, v8di c, v8di d, 
                             v8di mask1, v8di mask2, int *indices) {
    /* Complex expression that may require many operands when expanded */
    v8di temp1 = __builtin_shufflevector(a, b, 
        indices[0], indices[1], indices[2], indices[3],
        indices[4], indices[5], indices[6], indices[7]);
    
    v8di temp2 = __builtin_shufflevector(c, d,
        indices[8], indices[9], indices[10], indices[11],
        indices[12], indices[13], indices[14], indices[15]);
    
    /* Blend operation using masks - complex expression */
    v8di result = (mask1 & temp1) | (~mask1 & temp2);
    result = (mask2 & result) | (~mask2 & (temp1 + temp2));
    
    /* Store to prevent optimization */
    memcpy((void*)global_result, &result, sizeof(result));
}

/* Permute with variable indices - may trigger vec_perm expansion */
void test_gcc_permute(v4di a, v4di b, int idx0, int idx1, int idx2, int idx3,
                      int idx4, int idx5, int idx6, int idx7) {
    v4di result = __builtin_shufflevector(a, b, 
        idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7);
    
    /* Complex use of result to prevent optimization */
    for (int i = 0; i < 4; i++) {
        global_result[i] += ((int64_t*)&result)[i];
    }
}
#endif

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask - takes many operands */
void test_avx512_permutex2var(__m512i a, __m512i b, __m512i idx, 
                              __mmask16 mask, __m512i c, __m512i d) {
    /* First permutation: a[idx] using b as second source */
    __m512i temp1 = _mm512_mask_permutex2var_epi64(a, mask, idx, b);
    
    /* Second permutation with different sources */
    __m512i temp2 = _mm512_mask_permutex2var_epi64(c, ~mask, idx, d);
    
    /* Blend the results */
    __m512i result = _mm512_mask_blend_epi64(mask, temp1, temp2);
    
    /* Store to volatile global */
    _mm512_store_epi64((void*)global_result, result);
}

/* Complex AVX-512 expression with multiple operands */
void test_avx512_complex(__m512i v0, __m512i v1, __m512i v2, __m512i v3,
                         __m512i v4, __m512i v5, __m512i v6, __m512i v7,
                         __mmask16 m0, __mmask16 m1, __m512i idx) {
    /* Chain of operations that may expand to many operands */
    __m512i t0 = _mm512_mask_permutex2var_epi32(v0, m0, idx, v1);
    __m512i t1 = _mm512_mask_permutex2var_epi32(v2, m1, idx, v3);
    __m512i t2 = _mm512_mask_permutex2var_epi32(v4, m0 & m1, idx, v5);
    __m512i t3 = _mm512_mask_permutex2var_epi32(v6, m0 | m1, idx, v7);
    
    /* Final blend - complex expression */
    __m512i result = _mm512_mask_blend_epi32(0xAAAA, 
                    _mm512_add_epi32(t0, t1),
                    _mm512_add_epi32(t2, t3));
    
    _mm512_store_epi64((void*)global_result, result);
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg,
                     svint64_t data1, svint64_t data2, svint64_t data3) {
    /* Complex gather/scatter pattern */
    svint64_t gathered = svld1_gather_s64index_s64(pg, (const int64_t*)&base, offsets);
    
    /* Manipulate with multiple data vectors */
    svint64_t temp1 = svadd_s64_z(pg, gathered, data1);
    svint64_t temp2 = svadd_s64_z(pg, temp1, data2);
    svint64_t result = svadd_s64_z(pg, temp2, data3);
    
    /* Store result */
    svst1_s64(pg, (int64_t*)global_result, result);
}

/* SVE permute with multiple vector arguments */
void test_sve_permute(svint64_t v0, svint64_t v1, svint64_t v2, svint64_t v3,
                      svint64_t indices, svbool_t pg) {
    /* Complex permutation pattern using tbl */
    svint64_t t0 = svtbl_s64(v0, indices);
    svint64_t t1 = svtbl_s64(v1, indices);
    svint64_t t2 = svtbl_s64(v2, indices);
    svint64_t t3 = svtbl_s64(v3, indices);
    
    /* Blend operations */
    svint64_t result = svsel_s64(pg, 
                    svadd_s64_z(pg, t0, t1),
                    svadd_s64_z(pg, t2, t3));
    
    svst1_s64(pg, (int64_t*)global_result, result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* PowerPC vec_perm with many operands */
void test_ppc_permute(vector signed long long a, vector signed long long b,
                      vector signed long long c, vector signed long long d,
                      vector unsigned char perm1, vector unsigned char perm2,
                      vector unsigned char mask) {
    /* Chain of permutations */
    vector signed long long t1 = vec_perm(a, b, perm1);
    vector signed long long t2 = vec_perm(c, d, perm2);
    
    /* Complex blend */
    vector signed long long result = vec_sel(t1, t2, mask);
    
    /* Store result */
    vec_st(result, 0, (vector signed long long*)global_result);
}
#endif

/* ==================== Main Test Driver ==================== */

int main() {
    /* Initialize test data */
    int64_t data[32];
    for (int i = 0; i < 32; i++) {
        data[i] = i * 3 + 1;
    }
    
    int indices[16] = {0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15};
    
    /* Test GCC vector extensions if available */
#ifdef __GNUC__
    {
        v8di v1 = {data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]};
        v8di v2 = {data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]};
        v8di v3 = {data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23]};
        v8di v4 = {data[24], data[25], data[26], data[27], data[28], data[29], data[30], data[31]};
        v8di mask1 = {0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF, 0,
                      0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF, 0};
        v8di mask2 = {0, 0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF,
                      0, 0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF};
        
        test_gcc_vector_shuffle(v1, v2, v3, v4, mask1, mask2, indices);
        
        /* Also test smaller vector permute */
        v4di a4 = {data[0], data[1], data[2], data[3]};
        v4di b4 = {data[4], data[5], data[6], data[7]};
        test_gcc_permute(a4, b4, 0, 5, 2, 7, 1, 6, 3, 4);
    }
#endif
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512i av = _mm512_load_epi64(data);
        __m512i bv = _mm512_load_epi64(data + 8);
        __m512i cv = _mm512_load_epi64(data + 16);
        __m512i dv = _mm512_load_epi64(data + 24);
        
        __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __mmask16 mask = 0xAA55;
        
        test_avx512_permutex2var(av, bv, idx, mask, cv, dv);
        
        /* Test complex expression */
        __m512i v0 = av, v1 = bv, v2 = cv, v3 = dv;
        __m512i v4 = _mm512_add_epi64(av, bv);
        __m512i v5 = _mm512_sub_epi64(cv, dv);
        __m512i v6 = _mm512_mullo_epi64(av, cv);
        __m512i v7 = _mm512_mullo_epi64(bv, dv);
        
        __mmask16 m0 = 0xF0F0;
        __mmask16 m1 = 0x0F0F;
        __m512i idx32 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        test_avx512_complex(_mm512_castsi512_si512(v0), 
                           _mm512_castsi512_si512(v1),
                           _mm512_castsi512_si512(v2),
                           _mm512_castsi512_si512(v3),
                           _mm512_castsi512_si512(v4),
                           _mm512_castsi512_si512(v5),
                           _mm512_castsi512_si512(v6),
                           _mm512_castsi512_si512(v7),
                           m0, m1, idx32);
    }
#endif
    
    /* Test ARM SVE if available */
#ifdef __ARM_FEATURE_SVE
    {
        /* Note: SVE vectors are variable-length, so we use the appropriate size */
        svint64_t sve_data = svld1_s64(svptrue_b64(), data);
        svint64_t sve_offsets = svindex_s64(0, 1);
        svbool_t pg = svptrue_b64();
        
        test_sve_gather(sve_data, sve_offsets, pg, 
                       svadd_s64_z(pg, sve_data, sve_data),
                       svmul_s64_z(pg, sve_data, sve_data),
                       svsub_s64_z(pg, sve_data, sve_data));
    }
#endif
    
    /* Test PowerPC Altivec if available */
#ifdef __ALTIVEC__
    {
        vector signed long long va = vec_ld(0, (vector signed long long*)data);
        vector signed long long vb = vec_ld(16, (vector signed long long*)data);
        vector signed long long vc = vec_ld(32, (vector signed long long*)data);
        vector signed long long vd = vec_ld(48, (vector signed long long*)data);
        
        vector unsigned char perm1 = {0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23};
        vector unsigned char perm2 = {8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31};
        vector unsigned char mask = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                                    0,0,0,0,0,0,0,0};
        
        test_ppc_permute(va, vb, vc, vd, perm1, perm2, mask);
    }
#endif
    
    /* Compute checksum from results */
    for (int i = 0; i < 16; i++) {
        checksum += (int)global_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
