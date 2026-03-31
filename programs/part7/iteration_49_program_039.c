/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand RTL pattern */
__attribute__((optimize("O3", "no-inline")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 complex permute with mask - can generate 10 operands */
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may expand to 10 operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    
#elif defined(__aarch64__)
    /* ARM NEON complex operations */
    #include <arm_neon.h>
    
    /* Create a complex vector operation chain */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Multiple operations that might combine into high-operand pattern */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    
    /* Complex tbl operation with multiple registers */
    uint8x16_t idx_vec = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8x16_t tbl_result = vqtbl1q_u8(r3, idx_vec);
    
    global_counter += vgetq_lane_u8(tbl_result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        "/* 10-operand test */\n\t"
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to trigger 11-operand RTL pattern */
__attribute__((optimize("O3", "no-inline")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* AVX-512 masked gather with complex addressing - can generate 11 operands */
    #include <immintrin.h>
    
    long long base[64];
    for (int i = 0; i < 64; i++) base[i] = i;
    
    __m512i vindex = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    int scale = 1;
    
    /* Gather operation with mask, base, index, scale - potentially 11 operands */
    __m512i gathered = _mm512_mask_i64gather_epi64(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        vindex,                  // index
        (void*)base,             // base
        scale                    // scale
    );
    
    global_counter += _mm512_extract_epi64(gathered, 0);
    
#elif defined(__aarch64__)
    /* ARM SVE-like multiple register operations */
    #include <arm_neon.h>
    
    /* Complex multi-register operation */
    uint8x16_t v0 = vdupq_n_u8(0);
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    uint8x16_t v5 = vdupq_n_u8(5);
    
    /* Chain of operations that might combine */
    uint8x16_t r1 = vaddq_u8(v0, v1);
    uint8x16_t r2 = vaddq_u8(v2, v3);
    uint8x16_t r3 = vaddq_u8(v4, v5);
    uint8x16_t r4 = vaddq_u8(r1, r2);
    uint8x16_t final = vaddq_u8(r4, r3);
    
    /* Complex tbl2 operation with two table registers */
    uint8x16_t idx = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8x16x2_t tbl_data = {r1, r2};
    uint8x16_t tbl_result = vqtbl2q_u8(tbl_data, idx);
    
    global_counter += vgetq_lane_u8(tbl_result, 0) + vgetq_lane_u8(final, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
        "/* 11-operand test */\n\t"
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test with complex vector operations that might combine */
__attribute__((optimize("O3", "no-inline")))
void test_vector_combine(void) {
#ifdef __x86_64__
    #include <immintrin.h>
    
    /* Complex AVX-512 operation chain that might combine into high-operand pattern */
    __m512i v1 = _mm512_set1_epi32(1);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    __m512i v5 = _mm512_set1_epi32(5);
    
    /* Chain of operations */
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_add_epi32(v3, v4);
    __m512i r3 = _mm512_add_epi32(r1, r2);
    __m512i r4 = _mm512_add_epi32(r3, v5);
    
    /* Masked store with complex addressing */
    int32_t array[16] = {0};
    __mmask16 store_mask = 0xFFFF;
    _mm512_mask_storeu_epi32(array, store_mask, r4);
    
    global_counter += array[0];
#endif
}

int main(void) {
    printf("Testing high-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_combine();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
