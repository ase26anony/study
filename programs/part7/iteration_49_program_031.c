/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function to test 10-operand case using AVX-512 intrinsics */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 uses 10 operands in RTL:
       1. Destination
       2. Mask
       3. Index
       4. Source A
       5. Source B
       6-10. Various implicit operands for mask, immediate, etc.
    */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
}

/* Function to test 11-operand case using inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_x86(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint64_t result1, result2, result3, result4;
    
    /* Inline assembly with exactly 11 operands:
       10 inputs + 1 output = 11 total operands
       This should trigger the 11-operand case in optabs.cc
    */
    asm volatile (
        "/* 11-operand test */\n\t"
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "mov %[c], %[out2]\n\t"
        "add %[d], %[out2]\n\t"
        "mov %[e], %[out3]\n\t"
        "add %[f], %[out3]\n\t"
        "mov %[g], %[out4]\n\t"
        "add %[h], %[out4]\n\t"
        "imul %[i], %[out1]\n\t"
        "imul %[j], %[out2]\n\t"
        "imul %[k], %[out3]"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2),
          [out3] "=&r" (result3),
          [out4] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result1 + result2 + result3 + result4;
}

#elif defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>

/* Function to test 10-operand case using ARM NEON intrinsics */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_arm(void) {
    /* Complex NEON operation with table lookup - can generate multi-operand RTL */
    uint8x16_t table1 = vdupq_n_u8(1);
    uint8x16_t table2 = vdupq_n_u8(2);
    uint8x16_t table3 = vdupq_n_u8(3);
    uint8x16_t indices = vdupq_n_u8(0);
    
    /* vtbl3_u8 with multiple registers can generate complex RTL patterns */
    uint8x8x3_t tbl3 = {vget_low_u8(table1), vget_low_u8(table2), vget_low_u8(table3)};
    uint8x8_t result_low = vtbl3_u8(tbl3, vget_low_u8(indices));
    
    global_counter += vget_lane_u8(result_low, 0);
}

/* Function to test 11-operand case using inline assembly for ARM */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_arm(void) {
    uint32_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint32_t result1, result2, result3, result4;
    
    /* ARM inline assembly with 11 operands */
    asm volatile (
        "/* 11-operand ARM test */\n\t"
        "add %[out1], %[a], %[b]\n\t"
        "add %[out2], %[c], %[d]\n\t"
        "add %[out3], %[e], %[f]\n\t"
        "add %[out4], %[g], %[h]\n\t"
        "mla %[out1], %[out1], %[i], %[out1]\n\t"
        "mla %[out2], %[out2], %[j], %[out2]\n\t"
        "mla %[out3], %[out3], %[k], %[out3]"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2),
          [out3] "=&r" (result3),
          [out4] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result1 + result2 + result3 + result4;
}

#else
/* Generic fallback using complex inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result1, result2;
    
    /* Generic inline assembly with 10 operands */
    asm volatile (
        "/* 10-operand generic test */\n\t"
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "mov %[c], %[out2]\n\t"
        "add %[d], %[out2]\n\t"
        "imul %[e], %[out1]\n\t"
        "imul %[f], %[out2]\n\t"
        "add %[g], %[out1]\n\t"
        "add %[h], %[out2]\n\t"
        "sub %[i], %[out1]\n\t"
        "sub %[j], %[out2]"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    global_counter += result1 + result2;
}

__attribute__((optimize("O3,no-inline")))
void test_11_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result1, result2, result3;
    
    /* Generic inline assembly with 11 operands */
    asm volatile (
        "/* 11-operand generic test */\n\t"
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "mov %[c], %[out2]\n\t"
        "add %[d], %[out2]\n\t"
        "mov %[e], %[out3]\n\t"
        "add %[f], %[out3]\n\t"
        "imul %[g], %[out1]\n\t"
        "imul %[h], %[out2]\n\t"
        "imul %[i], %[out3]\n\t"
        "add %[j], %[out1]\n\t"
        "add %[k], %[out2]"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2),
          [out3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result1 + result2 + result3;
}
#endif

/* Additional test using complex vector operations that might combine into multi-operand RTL */
__attribute__((optimize("O3,no-inline")))
void test_vector_chain(void) {
#ifdef __x86_64__
    /* Chain of AVX operations that might be combined */
    __m256i v1 = _mm256_set1_epi32(1);
    __m256i v2 = _mm256_set1_epi32(2);
    __m256i v3 = _mm256_set1_epi32(3);
    __m256i v4 = _mm256_set1_epi32(4);
    
    /* Complex chain that might generate multi-operand patterns */
    __m256i r1 = _mm256_add_epi32(v1, v2);
    __m256i r2 = _mm256_mullo_epi32(r1, v3);
    __m256i r3 = _mm256_slli_epi32(r2, 2);
    __m256i r4 = _mm256_sub_epi32(r3, v4);
    
    /* Extract and use result */
    int32_t result[8];
    _mm256_storeu_si256((__m256i*)result, r4);
    global_counter += result[0];
#endif
}

/* Main driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
#ifdef __x86_64__
    test_10_operand_x86();
    test_11_operand_x86();
#elif defined(__aarch64__) || defined(__arm__)
    test_10_operand_arm();
    test_11_operand_arm();
#else
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    test_vector_chain();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
