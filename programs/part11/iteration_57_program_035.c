/* Test program to cover optabs.cc lines 8254-8263 (10-11 operand expansions) */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_results[1024];
volatile int global_idx = 0;

/* ==================== GCC Vector Extensions (Portable) ==================== */

#ifdef __GNUC__
typedef int64_t v2di __attribute__((vector_size(16)));
typedef int64_t v4di __attribute__((vector_size(32)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Complex shuffle that may expand to many operands */
void test_gcc_shuffle(v8di a, v8di b, v8di mask, v8di idx) {
    /* Use __builtin_shuffle with variable indices - may expand to vec_perm 
       with many operands during RTL expansion */
    v8di result = __builtin_shuffle(a, b, idx);
    
    /* Store to prevent optimization */
    for (int i = 0; i < 8; i++) {
        global_results[global_idx++] = result[i];
    }
}

/* Complex blend operation with multiple masks */
void test_gcc_blend(v4di a, v4di b, v4di c, v4di d, 
                    v4di mask1, v4di mask2, v4di mask3) {
    /* Complex expression that may require many operands */
    v4di result = (mask1 & a) | 
                  (~mask1 & mask2 & b) | 
                  (~mask1 & ~mask2 & mask3 & c) |
                  (mask1 & mask2 & mask3 & d);
    
    for (int i = 0; i < 4; i++) {
        global_results[global_idx++] = result[i];
    }
}
#endif

/* ==================== x86_64 AVX-512 Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

/* AVX-512 permutex2var with mask - takes many operands */
void test_avx512_permute(__m512i a, __m512i b, __m512i idx, __mmask16 mask) {
    /* _mm512_mask_permutex2var_epi32 has 5 explicit args but expands to many RTL operands:
       dest, mask, idx, a, b + implicit operands for mask register, etc. */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b, a);
    
    /* Store result */
    _mm512_store_epi64((void*)&global_results[global_idx], result);
    global_idx += 8;
}

/* AVX-512 ternary logic - can have many operands in expansion */
void test_avx512_ternary(__m512i a, __m512i b, __m512i c, __m512i d,
                         __m512i e, __m512i f, __m512i g) {
    /* Complex sequence that may combine into multi-operand pattern */
    __m512i t1 = _mm512_ternarylogic_epi64(a, b, c, 0x96);  /* XOR pattern */
    __m512i t2 = _mm512_ternarylogic_epi64(d, e, f, 0x69);  /* Another pattern */
    __m512i result = _mm512_mask_blend_epi64(0xAA, t1, t2);
    
    /* Blend with g using another operation */
    result = _mm512_or_epi64(result, g);
    
    _mm512_store_epi64((void*)&global_results[global_idx], result);
    global_idx += 8;
}
#endif

/* ==================== ARM SVE Intrinsics ==================== */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* SVE gather with multiple vector arguments - can have many operands */
void test_sve_gather(svint64_t base, svint64_t offsets, svbool_t pg) {
    /* svld1_gather_s64_index has base, offsets, predicate - expands to many operands */
    svint64_t result = svld1_gather_s64_index(pg, (const int64_t*)&global_results[0], 
                                              svadd_x(pg, base, offsets));
    
    /* Store using scatter - another multi-operand operation */
    svst1_scatter_s64_index(pg, (int64_t*)&global_results[64], 
                           svadd_x(pg, base, offsets), result);
}
#endif

/* ==================== PowerPC Altivec/VSX ==================== */

#ifdef __ALTIVEC__
#include <altivec.h>

/* vec_perm with three vectors - may expand to many operands */
void test_altivec_perm(vector signed long long a, 
                       vector signed long long b,
                       vector unsigned char perm) {
    /* vec_perm takes 3 vectors, but during expansion may need more operands */
    vector signed long long result = vec_perm(a, b, perm);
    
    /* Store result */
    vec_st(result, 0, (vector signed long long*)&global_results[global_idx]);
    global_idx += 2;
}
#endif

/* ==================== Main Test Driver ==================== */

int main() {
    /* Initialize with some data */
    int64_t init_data[128];
    for (int i = 0; i < 128; i++) {
        init_data[i] = i * 3 + 7;
        global_results[i] = 0;
    }
    
    /* Test GCC vector extensions if available */
#ifdef __GNUC__
    {
        v8di a = {0,1,2,3,4,5,6,7};
        v8di b = {8,9,10,11,12,13,14,15};
        v8di idx = {0,9,2,11,4,13,6,15};
        v4di mask1 = {0, -1, 0, -1};
        v4di mask2 = {-1, 0, -1, 0};
        v4di mask3 = {0, 0, -1, -1};
        v4di c = {16,17,18,19};
        v4di d = {20,21,22,23};
        
        test_gcc_shuffle(a, b, a, idx);
        test_gcc_blend(a, b, c, d, mask1, mask2, mask3);
    }
#endif
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
        __m512i idx = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __mmask16 mask = 0xAAAA;
        
        test_avx512_permute(a, b, idx, mask);
        
        __m512i c = _mm512_set1_epi64(1);
        __m512i d = _mm512_set1_epi64(2);
        __m512i e = _mm512_set1_epi64(3);
        __m512i f = _mm512_set1_epi64(4);
        __m512i g = _mm512_set1_epi64(5);
        
        test_avx512_ternary(a, b, c, d, e, f, g);
    }
#endif
    
    /* Test SVE if available */
#ifdef __ARM_FEATURE_SVE
    {
        svbool_t pg = svptrue_b64();
        svint64_t base = svdup_s64(0);
        svint64_t offsets = svindex_s64(0, 1);
        
        test_sve_gather(base, offsets, pg);
    }
#endif
    
    /* Test Altivec if available */
#ifdef __ALTIVEC__
    {
        vector signed long long a = {1, 2};
        vector signed long long b = {3, 4};
        vector unsigned char perm = {0,1,2,3,4,5,6,7, 8,9,10,11,12,13,14,15};
        
        test_altivec_perm(a, b, perm);
    }
#endif
    
    /* Compute checksum to ensure all operations executed */
    int64_t checksum = 0;
    for (int i = 0; i < 128; i++) {
        checksum += global_results[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    printf("Global index: %d\n", global_idx);
    
    return 0;
}
