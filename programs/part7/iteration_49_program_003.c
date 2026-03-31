/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand RTL pattern */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* x86_64: AVX-512 complex permute with mask and multiple immediates */
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic typically expands to complex RTL with many operands:
       dest, mask, src1, idx, src2, plus implicit control operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    
#elif defined(__aarch64__)
    /* ARM NEON: Complex vector table lookup with multiple registers */
    #include <arm_neon.h>
    uint8x16_t tab[4];
    uint8x16_t indices = vdupq_n_u8(0);
    
    /* Initialize table */
    for (int i = 0; i < 4; i++) {
        tab[i] = vdupq_n_u8(i);
    }
    
    /* Complex table lookup with multiple operands */
    uint8x16_t result = vqtbl4q_u8(*(uint8x16x4_t*)tab, indices);
    
    /* Use result */
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic: Inline assembly with 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    asm volatile (
        /* Multi-operand pattern that might expand to complex RTL */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (op1), "r" (op2), "r" (op3), "r" (op4),
          "r" (op5), "r" (op6), "r" (op7), "r" (op8),
          "r" (op9), "r" (op10)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to trigger 11-operand RTL pattern */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* x86_64: Complex AVX-512 masked gather with multiple parameters */
    #include <immintrin.h>
    double base[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    __m512i vindex = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512d src = _mm512_set1_pd(0.0);
    __mmask8 mask = 0xFF;
    int scale = 1;
    
    /* This gather operation has many operands in RTL representation */
    __m512d result = _mm512_mask_i64gather_pd(src, mask, vindex, base, scale);
    
    global_counter += (int)_mm512_cvtsd_f64(_mm512_castpd512_pd128(result));
    
#elif defined(__aarch64__)
    /* ARM: Complex vector multiply-accumulate with lane selection */
    #include <arm_neon.h>
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    
    /* Complex sequence that might combine into multi-operand pattern */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 1);
    int32x4_t t2 = vmlaq_laneq_s32(t1, d, a, 2);
    int32x4_t result = vaddq_s32(t2, vdupq_n_s32(5));
    
    global_counter += vgetq_lane_s32(result, 0);
    
#else
    /* Generic: Inline assembly with exactly 11 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long result;
    
    asm volatile (
        /* 11-operand pattern */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (op1), "r" (op2), "r" (op3), "r" (op4),
          "r" (op5), "r" (op6), "r" (op7), "r" (op8),
          "r" (op9), "r" (op10), "r" (op11)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test with complex vector operations that might combine */
__attribute__((noinline, optimize("O3")))
void test_complex_vector_chain(void) {
#ifdef __x86_64__
    #include <immintrin.h>
    /* Chain of operations that might be combined into a single RTL pattern */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    /* Complex sequence */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_mullo_epi32(t1, c);
    __m512i t3 = _mm512_slli_epi32(t2, 2);
    __m512i result = _mm512_sub_epi32(t3, d);
    
    global_counter += _mm512_extract_epi32(result, 0);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_complex_vector_chain();
    }
    
    printf("Result: %d\n", global_counter);
    return 0;
}
