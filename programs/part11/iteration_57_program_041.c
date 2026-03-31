/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[8] = {0};
volatile int checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

/* Test AVX-512 permute with mask - may expand to many operands */
void test_avx512_permute(void) {
    /* Create vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vec2 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx  = _mm512_set_epi32(1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31);
    
    /* Use volatile to prevent constant folding */
    volatile __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    /* This intrinsic has many operands when expanded:
     * dest, mask, idx, vec1, vec2
     * May expand to 10+ operands in RTL */
    __m512i result = _mm512_mask_permutex2var_epi32(
        vec1,                /* src1 */
        (__mmask16)mask,     /* mask */
        idx,                 /* idx */
        vec2                 /* src2 */
    );
    
    /* Store to volatile global to prevent dead code elimination */
    _mm512_storeu_epi32((void*)global_result, result);
    
    /* Compute simple checksum */
    int64_t *res = (int64_t*)&result;
    for (int i = 0; i < 8; i++) {
        checksum += (int)(res[i] & 0xFF);
    }
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    /* Complex expression that may require many operands */
    __m512i mask1 = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i mask2 = _mm512_set1_epi32(0x00000000);
    
    /* This complex blend may expand to many RTL operands */
    __m512i temp1 = _mm512_and_si512(mask1, a);
    __m512i temp2 = _mm512_andnot_si512(mask1, b);
    __m512i temp3 = _mm512_and_si512(mask2, c);
    __m512i temp4 = _mm512_andnot_si512(mask2, d);
    
    __m512i result = _mm512_or_si512(
        _mm512_or_si512(temp1, temp2),
        _mm512_or_si512(temp3, temp4)
    );
    
    _mm512_storeu_epi32((void*)(global_result + 8), result);
}
#endif

/* ==================== GCC Vector Extensions ==================== */
/* Portable vector implementation using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));

void test_gcc_vector_shuffle(void) {
    /* Initialize vectors */
    v16si v1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si v2 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable indices to prevent constant folding */
    volatile int idx_array[16] = {
        0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23
    };
    
    /* Create index vector from array */
    v16si indices = {
        idx_array[0], idx_array[1], idx_array[2], idx_array[3],
        idx_array[4], idx_array[5], idx_array[6], idx_array[7],
        idx_array[8], idx_array[9], idx_array[10], idx_array[11],
        idx_array[12], idx_array[13], idx_array[14], idx_array[15]
    };
    
    /* Manual permutation using GCC vector extensions
     * This may expand to vec_perm with many operands */
    v16si result;
    for (int i = 0; i < 16; i++) {
        int idx = indices[i];
        if (idx < 16) {
            result[i] = v1[idx];
        } else {
            result[i] = v2[idx - 16];
        }
    }
    
    /* Store result */
    memcpy((void*)global_result, &result, sizeof(result));
    
    /* Update checksum */
    for (int i = 0; i < 16; i++) {
        checksum += result[i];
    }
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    volatile int64_t base_array[100];
    volatile int64_t offset_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        base_array[i] = i * 2;
        offset_array[i] = i;
    }
    
    /* Create SVE vectors - actual implementation would use SVE intrinsics
     * This is a placeholder showing the pattern */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather intrinsic pattern (commented as exact intrinsic varies):
     * svint64_t result = svld1_gather_s64offset(pg, base_array, offset_vector);
     * This can expand to many RTL operands */
    
    /* For compilation without actual SVE hardware, we'll use a fallback */
    int64_t temp_result[16];
    for (int i = 0; i < 16; i++) {
        temp_result[i] = base_array[offset_array[i]];
    }
    
    memcpy((void*)global_result, temp_result, sizeof(temp_result));
}
#endif

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__
#include <altivec.h>

void test_altivec_permute(void) {
    /* PowerPC vec_perm takes 3 vectors and may expand further */
    vector unsigned char v1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char v2 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char perm = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    /* vec_perm with 3 vectors - may expand to many RTL operands */
    vector unsigned char result = vec_perm(v1, v2, perm);
    
    /* Store result */
    vec_st(result, 0, (vector unsigned char*)global_result);
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call architecture-specific tests */
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests executed\n");
#endif
    
    /* GCC vector extensions (portable) */
    test_gcc_vector_shuffle();
    printf("GCC vector shuffle test executed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("ARM SVE test executed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("PowerPC Altivec test executed\n");
#endif
    
    /* Final checksum to ensure computations aren't optimized away */
    printf("Checksum: %d\n", checksum);
    
    /* Print first few results for verification */
    printf("First 4 results: %ld %ld %ld %ld\n", 
           (long)global_result[0], (long)global_result[1],
           (long)global_result[2], (long)global_result[3]);
    
    return 0;
}
