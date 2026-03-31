/* Test program to cover 10- and 11-operand RTL expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Global volatile arrays to prevent optimization */
volatile int64_t global_result[16] = {0};
volatile int global_checksum = 0;

/* ==================== AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Test AVX-512 permute with mask - may expand to many operands */
void test_avx512_permute(void) {
    /* Initialize vectors with test data */
    __m512i vec1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vec2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i idx  = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xAAAA;  /* Alternating bits pattern */
    
    /* This intrinsic takes 5 arguments but may expand to more operands
       during RTL expansion due to mask handling and register allocation */
    __m512i result = _mm512_mask_permutex2var_epi32(vec1, mask, idx, vec1, vec2);
    
    /* Store result to prevent optimization */
    _mm512_storeu_si512((void*)global_result, result);
    
    /* Compute simple checksum */
    int32_t* res_ptr = (int32_t*)&result;
    for (int i = 0; i < 16; i++) {
        global_checksum += res_ptr[i];
    }
}

/* Another AVX-512 test with blend operation */
void test_avx512_blend(void) {
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0x5555;
    __mmask16 m2 = 0x3333;
    
    /* Complex expression that might expand to many operands */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i result = _mm512_add_epi32(t1, t2);
    
    _mm512_storeu_si512((void*)(global_result + 8), result);
}

#endif /* __AVX512F__ */

/* ==================== GCC Vector Extensions ==================== */

/* Portable vector types using GCC extensions */
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

/* Test with GCC vector shuffle - may trigger vec_perm expansion */
void test_gcc_vector_shuffle(void) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Variable indices to prevent constant folding */
    volatile int idx_array[16];
    for (int i = 0; i < 16; i++) {
        idx_array[i] = (i * 3) % 32;
    }
    
    /* Create shuffled vector - this may expand to complex RTL */
    v16si result;
    for (int i = 0; i < 16; i++) {
        int idx = idx_array[i];
        if (idx < 16) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 16];
        }
    }
    
    /* Store result */
    memcpy((void*)global_result, &result, sizeof(result));
    
    /* Update checksum */
    for (int i = 0; i < 16; i++) {
        global_checksum += result[i];
    }
}

/* Another GCC vector test with ternary operation */
void test_gcc_vector_ternary(void) {
    v8di x = {0,1,2,3,4,5,6,7};
    v8di y = {8,9,10,11,12,13,14,15};
    v8di z = {16,17,18,19,20,21,22,23};
    
    /* Complex conditional expression */
    v8di mask = x > y;
    v8di result = mask ? (x + z) : (y - z);
    
    memcpy((void*)(global_result + 8), &result, sizeof(result));
}

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

void test_sve_gather(void) {
    /* SVE gather operations can have many operands */
    int64_t base_array[16] = {0};
    int64_t offset_array[16] = {0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120};
    int64_t data_array[16] = {0};
    
    for (int i = 0; i < 16; i++) {
        base_array[i] = (int64_t)(data_array + i);
    }
    
    svbool_t pg = svwhilelt_b64(0, 16);
    svint64_t offsets = svld1_s64(pg, offset_array);
    
    /* Gather operation with multiple vector arguments */
    svint64_t result = svld1_gather_s64offset_s64(pg, base_array, offsets);
    
    /* Store result */
    svst1_s64(pg, global_result, result);
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

void test_altivec_permute(void) {
    vector signed int a = {0,1,2,3};
    vector signed int b = {4,5,6,7};
    vector unsigned char perm = {0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27};
    
    /* vec_perm with three vectors may expand to many operands */
    vector signed int result = vec_perm(a, b, perm);
    
    vec_st(result, 0, (vector signed int*)global_result);
}

#endif /* __ALTIVEC__ */

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing multi-operand RTL expansion...\n");
    
    /* Call all available tests */
    
#ifdef __AVX512F__
    test_avx512_permute();
    test_avx512_blend();
    printf("AVX-512 tests executed\n");
#endif
    
    test_gcc_vector_shuffle();
    test_gcc_vector_ternary();
    printf("GCC vector tests executed\n");
    
#ifdef __ARM_FEATURE_SVE
    test_sve_gather();
    printf("ARM SVE tests executed\n");
#endif
    
#ifdef __ALTIVEC__
    test_altivec_permute();
    printf("Altivec tests executed\n");
#endif
    
    /* Final checksum to ensure all operations executed */
    int final_checksum = global_checksum;
    for (int i = 0; i < 16; i++) {
        final_checksum += global_result[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Test completed.\n");
    
    return 0;
}
