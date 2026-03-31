/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand case */
__attribute__((optimize("O3,no-inline")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 complex permute with mask - can generate 10 operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 has many operands in RTL:
       dest, mask, idx, src1, src2 + implicit operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Complex blend with multiple sources and mask */
    __m512i blend_result = _mm512_mask_blend_epi64(mask, src1, src2);
    
    /* Force use of result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result)[0];
    global_counter += _mm512_reduce_add_epi64(blend_result)[0];
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE complex operations */
    #include <arm_neon.h>
    
    /* Use multiple vector operations that might combine */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    
    /* Complex sequence that might generate multi-operand RTL */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(r1, c);
    uint8x16_t r3 = vmulq_u8(r2, a);
    
    /* Table lookup with multiple registers */
    uint8x16_t table_result = vqtbl1q_u8(r3, vdupq_n_u8(0));
    
    /* Force use */
    global_counter += vgetq_lane_u8(table_result, 0);
#endif

    /* Generic multi-operand inline assembly as fallback */
    /* 10 operands: 5 inputs, 2 outputs, 3 clobbers */
    asm volatile (
        "# 10-operand asm block\n"
        "mov %[out1], %[in1]\n\t"
        "add %[out1], %[in2]\n\t"
        "add %[out1], %[in3]\n\t"
        "mov %[out2], %[in4]\n\t"
        "sub %[out2], %[in5]"
        : [out1] "=r" (global_counter), [out2] "=r" (global_counter)
        : [in1] "r" (global_counter), 
          [in2] "r" (1),
          [in3] "r" (2),
          [in4] "r" (global_counter),
          [in5] "r" (3)
        : "cc", "memory", "eax"
    );
}

/* Function to trigger 11-operand case */
__attribute__((optimize("O3,no-inline")))
void test_11_operand(void) {
    /* Explicit 11-operand inline assembly */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    int out1, out2, out3;
    
    asm volatile (
        "# 11-operand asm block\n"
        "mov %[o1], %[a]\n\t"
        "add %[o1], %[b]\n\t"
        "mov %[o2], %[c]\n\t"
        "imul %[o2], %[d]\n\t"
        "mov %[o3], %[e]\n\t"
        "sub %[o3], %[f]"
        : [o1] "=&r" (out1), 
          [o2] "=&r" (out2),
          [o3] "=&r" (out3)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f)
        : "cc", "memory", "rax", "rdx"
    );
    
    global_counter += out1 + out2 + out3;

#ifdef __x86_64__
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    /* AVX-512 masked gather with scale - potentially many operands */
    __m512i base = _mm512_set1_epi64(0);
    __m512i vindex = _mm512_set_epi64(0, 8, 16, 24, 32, 40, 48, 56);
    __mmask8 mask = 0xFF;
    long long scale = 1;
    
    /* This intrinsic expands to complex RTL with many operands */
    __m512i gathered = _mm512_mask_i64gather_epi64(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        vindex,                  // index
        (void*)&global_counter,  // base
        scale                    // scale
    );
    
    /* Another complex operation */
    __m512i shuffled = _mm512_permutexvar_epi64(
        _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7),
        gathered
    );
    
    global_counter += _mm512_reduce_add_epi64(shuffled)[0];
    #endif
#endif
}

/* Complex vector operation chain that might generate multi-operand RTL */
__attribute__((optimize("O3,no-inline")))
void test_vector_chain(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Chain of operations that might be combined into one RTL pattern */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    /* Complex sequence with multiple operations */
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_mullo_epi32(r1, v3);
    __m512i r3 = _mm512_slli_epi32(r2, 2);
    __m512i r4 = _mm512_maskz_permutexvar_epi32(0xFF, v4, r3);
    
    /* Force use */
    int sum = _mm512_reduce_add_epi32(r4);
    global_counter += sum;
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
