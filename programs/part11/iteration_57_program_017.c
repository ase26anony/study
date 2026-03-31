/* Test program to cover 10-11 operand RTL expansion cases in optabs.cc */
/* Compile with: -O2 -mavx512f -mavx512vl -ftree-vectorize -fdump-rtl-expand */
/* For SVE: -O3 -march=armv8-a+sve -ftree-vectorize -fdump-rtl-expand */
/* For portable: -O2 -ftree-vectorize -fdump-rtl-expand */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[8] = {0};
volatile int global_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test function using _mm512_mask_permutex2var_epi32 - can expand to many operands */
void test_avx512_permute(void) {
    /* Create source vectors with sequential data */
    __m512i vec1 = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    __m512i vec2 = _mm512_set_epi32(31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16);
    
    /* Index vector - controls which elements to select */
    __m512i idx = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    
    /* Mask - controls which elements to write */
    __mmask16 mask = 0xAAAA; /* Alternating pattern: 1010101010101010 */
    
    /* This intrinsic can expand to many operands:
       - Destination (implicit)
       - Mask
       - Index vector
       - Two source vectors
       Plus various intermediate operands during expansion */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Force computation and prevent dead code elimination */
    _mm512_storeu_si512((void*)global_result, result);
    
    /* Compute simple checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += global_result[i];
    }
    global_checksum += sum;
}

/* Another AVX-512 test with blend operation using multiple masks */
void test_avx512_blend(void) {
    __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
    __m512i c = _mm512_set_epi64(23, 22, 21, 20, 19, 18, 17, 16);
    __m512i d = _mm512_set_epi64(31, 30, 29, 28, 27, 26, 25, 24);
    
    __mmask8 mask1 = 0xAA; /* 10101010 */
    __mmask8 mask2 = 0xCC; /* 11001100 */
    
    /* Complex blend expression that may expand to many operands */
    __m512i temp1 = _mm512_mask_blend_epi64(mask1, a, b);
    __m512i temp2 = _mm512_mask_blend_epi64(mask2, c, d);
    __m512i result = _mm512_mask_blend_epi64(mask1 ^ mask2, temp1, temp2);
    
    _mm512_storeu_si512((void*)global_result, result);
}

#endif /* __AVX512F__ */

/* ==================== SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands:
       - Base pointer
       - Offset vector
       - Predicate
       - Scale
       - Result vector */
    
    /* Use variable length to prevent constant folding */
    volatile int n = 256;
    svbool_t pg = svwhilelt_b32(0, n);
    
    int64_t base_array[256];
    for (int i = 0; i < 256; i++) base_array[i] = i;
    
    svint64_t offsets = svindex_s64(0, 1);
    
    /* This gather operation expands to many RTL operands */
    svint64_t gathered = svld1_gather_s64offset_s64(pg, base_array, offsets);
    
    /* Store to prevent optimization */
    svst1_s64(pg, (int64_t*)global_result, gathered);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== Portable GCC Vector Extensions ==================== */

/* Define vector types using GCC extensions */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));

/* Complex shuffle operation that may require many operands */
void test_gcc_vector_shuffle(void) {
    v8si a = {0, 1, 2, 3, 4, 5, 6, 7};
    v8si b = {8, 9, 10, 11, 12, 13, 14, 15};
    
    /* Variable index array prevents constant folding */
    volatile int idx_array[8] = {0, 8, 1, 9, 2, 10, 3, 11};
    
    /* Create index vector from array */
    v8si idx = {
        idx_array[0], idx_array[1], idx_array[2], idx_array[3],
        idx_array[4], idx_array[5], idx_array[6], idx_array[7]
    };
    
    /* Complex permutation using __builtin_shuffle
       This often expands to vec_perm with many operands */
    v8si result = __builtin_shuffle(a, b, idx);
    
    /* Force computation */
    memcpy((void*)global_result, &result, sizeof(result));
    
    /* Update checksum */
    int sum = 0;
    int* res_ptr = (int*)&result;
    for (int i = 0; i < 8; i++) {
        sum += res_ptr[i];
    }
    global_checksum += sum;
}

/* Another portable test with ternary operation */
void test_gcc_ternary_op(void) {
    v4di x = {0, 1, 2, 3};
    v4di y = {4, 5, 6, 7};
    v4di z = {8, 9, 10, 11};
    
    /* Complex conditional expression */
    v4di mask = x > y;
    v4di result = (mask & x) | (~mask & (y + z));
    
    memcpy((void*)global_result, &result, sizeof(result));
}

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call portable tests first */
    test_gcc_vector_shuffle();
    test_gcc_ternary_op();
    
    /* Call architecture-specific tests if available */
#ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    test_avx512_permute();
    test_avx512_blend();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Running SVE tests...\n");
    test_sve_gather();
#endif
    
    /* Final checksum to ensure computations aren't optimized away */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        final_sum += global_result[i];
    }
    
    printf("Final checksum: %d\n", final_sum + global_checksum);
    
    return 0;
}
