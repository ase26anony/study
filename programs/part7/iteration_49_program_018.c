/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 complex permute with mask - typically expands to many operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 has many operands in RTL:
       dest, mask, idx, src1, src2 + various immediates */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src1, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
    /* Alternative: Complex blend with multiple sources and mask */
    #ifdef __AVX512VL__
    #include <immintrin.h>
    __m256i a = _mm256_set1_epi32(1);
    __m256i b = _mm256_set1_epi32(2);
    __m256i c = _mm256_set1_epi32(3);
    __m256i d = _mm256_set1_epi32(4);
    __mmask8 m1 = 0xAA;
    __mmask8 m2 = 0x55;
    
    /* Complex sequence that might combine into multi-operand pattern */
    __m256i t1 = _mm256_mask_blend_epi32(m1, a, b);
    __m256i t2 = _mm256_mask_blend_epi32(m2, c, d);
    __m256i final = _mm256_add_epi32(t1, t2);
    
    global_counter += _mm256_extract_epi32(final, 0);
    #endif
#endif

#ifdef __aarch64__
    /* ARM NEON complex operations with multiple vector registers */
    #include <arm_neon.h>
    
    /* Create multiple vector registers for complex operations */
    uint8x16_t v0 = vdupq_n_u8(1);
    uint8x16_t v1 = vdupq_n_u8(2);
    uint8x16_t v2 = vdupq_n_u8(3);
    uint8x16_t v3 = vdupq_n_u8(4);
    uint8x16_t v4 = vdupq_n_u8(5);
    uint8x16_t v5 = vdupq_n_u8(6);
    
    /* Complex sequence of operations that might generate multi-operand RTL */
    uint8x16_t r1 = vaddq_u8(v0, v1);
    uint8x16_t r2 = vaddq_u8(v2, v3);
    uint8x16_t r3 = vaddq_u8(v4, v5);
    
    /* Table lookup with multiple vectors - can generate complex patterns */
    uint8x16_t indices = vdupq_n_u8(0);
    uint8x16x4_t table = {v0, v1, v2, v3};
    uint8x16_t result = vqtbl4q_u8(table, indices);
    
    global_counter += vgetq_lane_u8(result, 0);
#endif

    /* Generic fallback: Inline assembly with 10 operands */
    /* This should directly trigger the 10-operand case */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result = 0;
    
    asm volatile (
        /* 10 operands: 5 inputs, 1 output, 4 clobbers */
        "mov %[res], %[in1] \n\t"
        "add %[res], %[in2] \n\t"
        "add %[res], %[in3] \n\t"
        "add %[res], %[in4] \n\t"
        "add %[res], %[in5] \n\t"
        "add %[res], %[in6] \n\t"
        "add %[res], %[in7] \n\t"
        "add %[res], %[in8] \n\t"
        "add %[res], %[in9] \n\t"
        "add %[res], %[in10] \n\t"
        : [res] "=&r" (result)
        : [in1] "r" (op1), [in2] "r" (op2), [in3] "r" (op3),
          [in4] "r" (op4), [in5] "r" (op5), [in6] "r" (op6),
          [in7] "r" (op7), [in8] "r" (op8), [in9] "r" (op9),
          [in10] "r" (op10)
        : "cc", "memory"
    );
    
    global_counter += result;
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* AVX-512 masked gather with complex addressing - can have many operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    __m512i vindex = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __mmask16 mask = 0xFFFF;
    int base[64] = {0};
    
    /* _mm512_mask_i32gather_epi32 has many operands:
       dest, mask, vindex, base, scale, src */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),
        mask,
        vindex,
        base,
        4
    );
    
    global_counter += _mm512_extract_epi32(gathered, 0);
    #endif
#endif

    /* Generic fallback: Inline assembly with exactly 11 operands */
    /* This should directly trigger the 11-operand case */
    long in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    long in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10, in11 = 11;
    long out1 = 0, out2 = 0;
    
    asm volatile (
        /* 11 operands: 6 inputs, 2 outputs, 3 clobbers = 11 total */
        /* Complex multi-instruction sequence */
        "mov %[out1], %[a1] \n\t"
        "mov %[out2], %[a2] \n\t"
        "add %[out1], %[a3] \n\t"
        "add %[out2], %[a4] \n\t"
        "imul %[out1], %[a5] \n\t"
        "imul %[out2], %[a6] \n\t"
        "add %[out1], %[a7] \n\t"
        "add %[out2], %[a8] \n\t"
        "sub %[out1], %[a9] \n\t"
        "sub %[out2], %[a10] \n\t"
        "xor %[out1], %[a11] \n\t"
        : [out1] "=&r" (out1), [out2] "=&r" (out2)
        : [a1] "r" (in1), [a2] "r" (in2), [a3] "r" (in3),
          [a4] "r" (in4), [a5] "r" (in5), [a6] "r" (in6),
          [a7] "r" (in7), [a8] "r" (in8), [a9] "r" (in9),
          [a10] "r" (in10), [a11] "r" (in11)
        : "cc", "memory"
    );
    
    global_counter += out1 + out2;
}

/* Additional test using OpenMP SIMD pragmas to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef _OPENMP
    #pragma omp declare simd
#endif
    float arr1[64], arr2[64], arr3[64], arr4[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        arr1[i] = i * 1.0f;
        arr2[i] = i * 2.0f;
        arr3[i] = i * 3.0f;
        arr4[i] = i * 4.0f;
    }
    
    /* Complex vector operation chain that might be combined */
    #ifdef __AVX512F__
    #pragma omp simd
    #endif
    for (int i = 0; i < 64; i++) {
        /* Multiple operations that could be combined into one RTL pattern */
        float t1 = arr1[i] + arr2[i];
        float t2 = arr3[i] - arr4[i];
        float t3 = t1 * t2;
        float t4 = t3 / (arr1[i] + 1.0f);
        result[i] = t4 + arr2[i] - arr3[i] * arr4[i];
    }
    
    global_counter += (int)result[0];
}

int main(void) {
    printf("Testing 10 and 11 operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
