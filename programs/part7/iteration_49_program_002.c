/* test_optabs_coverage.c - Test program to cover 10 and 11 operand cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force aggressive optimization on specific functions */
#define AGGRESSIVE_OPT __attribute__((optimize("O3", "unroll-loops")))

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
    #define X86_ARCH 1
    #include <immintrin.h>
    #include <x86intrin.h>
#endif

#if defined(__aarch64__) || defined(__arm__)
    #define ARM_ARCH 1
    #include <arm_neon.h>
#endif

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* ============================================
 * Function targeting 10-operand RTL pattern
 * ============================================ */
AGGRESSIVE_OPT
void test_10_operand(void) {
    printf("Testing 10-operand pattern...\n");
    
#if defined(X86_ARCH) && defined(__AVX512F__)
    /* Complex AVX-512 masked permute with multiple immediates - likely to generate 10-operand RTL */
    __m512i src1 = _mm512_set_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i src2 = _mm512_set_epi64(9, 10, 11, 12, 13, 14, 15, 16);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic with mask, two sources, index, and implicit control
       often expands to complex RTL with many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent dead code elimination */
    global_result += _mm512_reduce_add_epi64(result);
    
#elif defined(ARM_ARCH)
    /* ARM NEON complex table lookup with multiple registers */
    uint8x16_t tab1 = vdupq_n_u8(1);
    uint8x16_t tab2 = vdupq_n_u8(2);
    uint8x16_t tab3 = vdupq_n_u8(3);
    uint8x16_t idx = vdupq_n_u8(0);
    
    /* vqtbl3q_u8 with 3 table registers + index = complex multi-operand pattern */
    uint8x16x3_t tables = {tab1, tab2, tab3};
    uint8x16_t result = vqtbl3q_u8(tables, idx);
    
    global_result += vaddvq_u8(result);
    
#else
    /* Generic fallback: Inline assembly with exactly 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    /* Complex inline asm with 10 explicit operands plus clobbers */
    __asm__ volatile (
        /* Multiple operations chained together */
        "add %[r1], %[a1], %[a2]\n\t"
        "sub %[r2], %[a3], %[a4]\n\t"
        "mul %[r1], %[r1], %[a5]\n\t"
        "div %[r2], %[r2], %[a6]\n\t"
        "and %[r1], %[r1], %[a7]\n\t"
        "or  %[r2], %[r2], %[a8]\n\t"
        "xor %[r1], %[r1], %[a9]\n\t"
        "add %[r2], %[r2], %[a10]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc", "memory"
    );
    
    global_result += result1 + result2;
#endif
    
    global_counter++;
}

/* ============================================
 * Function targeting 11-operand RTL pattern
 * ============================================ */
AGGRESSIVE_OPT
void test_11_operand(void) {
    printf("Testing 11-operand pattern...\n");
    
#if defined(X86_ARCH) && defined(__AVX512F__)
    /* AVX-512 complex blend with multiple masks and immediates */
    __m512d a = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_set_pd(9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
    __m512d c = _mm512_set_pd(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0);
    __mmask8 m1 = 0xAA;
    __mmask8 m2 = 0x55;
    
    /* Complex sequence that might combine into multi-operand pattern */
    __m512d t1 = _mm512_mask_blend_pd(m1, a, b);
    __m512d t2 = _mm512_mask_blend_pd(m2, c, t1);
    __m512d result = _mm512_add_pd(t2, _mm512_set1_pd(100.0));
    
    /* Force use of result */
    double sum = _mm512_reduce_add_pd(result);
    global_result += (int)sum;
    
#elif defined(ARM_ARCH)
    /* ARM complex vector operations chained together */
    float32x4_t v1 = vdupq_n_f32(1.0f);
    float32x4_t v2 = vdupq_n_f32(2.0f);
    float32x4_t v3 = vdupq_n_f32(3.0f);
    float32x4_t v4 = vdupq_n_f32(4.0f);
    
    /* Complex sequence that might generate multi-operand RTL */
    float32x4_t r1 = vaddq_f32(v1, v2);
    float32x4_t r2 = vmulq_f32(v3, v4);
    float32x4_t r3 = vfmaq_f32(r1, r2, vdupq_n_f32(5.0f));
    float32x4_t result = vaddq_f32(r3, vdupq_n_f32(10.0f));
    
    global_result += vaddvq_f32(result);
    
#else
    /* Generic fallback: Inline assembly with exactly 11 operands */
    long ops[11];
    long results[3];
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* Complex inline assembly with 11 input operands */
    __asm__ volatile (
        /* Multiple operations using all 11 operands */
        "mov %[r1], %[a1]\n\t"
        "add %[r1], %[r1], %[a2]\n\t"
        "sub %[r1], %[r1], %[a3]\n\t"
        "mul %[r1], %[r1], %[a4]\n\t"
        "div %[r1], %[r1], %[a5]\n\t"
        "mov %[r2], %[a6]\n\t"
        "add %[r2], %[r2], %[a7]\n\t"
        "sub %[r2], %[r2], %[a8]\n\t"
        "mul %[r2], %[r2], %[a9]\n\t"
        "div %[r2], %[r2], %[a10]\n\t"
        "add %[r3], %[r1], %[r2]\n\t"
        "add %[r3], %[r3], %[a11]\n\t"
        : [r1] "=&r" (results[0]), 
          [r2] "=&r" (results[1]), 
          [r3] "=&r" (results[2])
        : [a1] "r" (ops[0]), [a2] "r" (ops[1]), [a3] "r" (ops[2]),
          [a4] "r" (ops[3]), [a5] "r" (ops[4]), [a6] "r" (ops[5]),
          [a7] "r" (ops[6]), [a8] "r" (ops[7]), [a9] "r" (ops[8]),
          [a10] "r" (ops[9]), [a11] "r" (ops[10])
        : "cc", "memory"
    );
    
    global_result += results[0] + results[1] + results[2];
#endif
    
    global_counter++;
}

/* ============================================
 * Additional complex function that might trigger
 * pattern merging into multi-operand RTL
 * ============================================ */
AGGRESSIVE_OPT
void complex_vector_chain(void) {
    printf("Testing complex vector chain...\n");
    
#if defined(X86_ARCH)
    /* Chain of operations that might be combined */
    __m256i v1 = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    __m256i v2 = _mm256_set_epi32(9, 10, 11, 12, 13, 14, 15, 16);
    __m256i v3 = _mm256_set_epi32(17, 18, 19, 20, 21, 22, 23, 24);
    
    /* Complex sequence */
    __m256i r1 = _mm256_add_epi32(v1, v2);
    __m256i r2 = _mm256_mullo_epi32(r1, v3);
    __m256i r3 = _mm256_slli_epi32(r2, 2);
    __m256i r4 = _mm256_srai_epi32(r3, 1);
    __m256i result = _mm256_blend_epi32(r4, v1, 0xAA);
    
    /* Extract and use result */
    int32_t sum = 0;
    int32_t* ptr = (int32_t*)&result;
    for (int i = 0; i < 8; i++) {
        sum += ptr[i];
    }
    global_result += sum;
#endif
    
    global_counter++;
}

/* ============================================
 * Main driver
 * ============================================ */
int main(void) {
    printf("Starting optabs coverage test...\n");
    
    /* Call all test functions multiple times */
    for (int i = 0; i < 3; i++) {
        test_10_operand();
        test_11_operand();
        complex_vector_chain();
    }
    
    printf("Test completed. Global result: %d (counter: %d)\n", 
           global_result, global_counter);
    
    /* Simple validation */
    if (global_counter > 0 && global_result != 0) {
        printf("SUCCESS: Tests executed and produced non-zero result.\n");
        return 0;
    } else {
        printf("WARNING: No operations performed or result is zero.\n");
        return 1;
    }
}
