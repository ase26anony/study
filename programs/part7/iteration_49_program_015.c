/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force aggressive optimization on specific functions */
#define AGGRESSIVE_OPT __attribute__((optimize("O3", "unroll-loops")))

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
    #define X86_ARCH 1
    #include <immintrin.h>
    #include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
    #define ARM_ARCH 1
    #include <arm_neon.h>
#else
    #define GENERIC_ARCH 1
#endif

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* ============================================
 * Function targeting 10-operand case
 * ============================================ */
AGGRESSIVE_OPT
void test_10_operand(void) {
    printf("Testing 10-operand case...\n");
    
#if X86_ARCH && defined(__AVX512F__)
    /* Complex AVX-512 masked permute with multiple immediates - likely to generate 10-operand RTL */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic often expands to complex RTL with many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    global_counter += _mm512_extract_epi64(result, 0);
    
#elif ARM_ARCH
    /* ARM NEON complex table lookup with multiple registers */
    uint8x16_t table[4];
    uint8x16_t indices = vdupq_n_u8(0);
    
    /* Initialize tables */
    for (int i = 0; i < 4; i++) {
        table[i] = vdupq_n_u8(i);
    }
    
    /* Complex multi-register table lookup - may generate multi-operand RTL */
    uint8x16_t result = vqtbl4q_u8(*(uint8x16x4_t*)table, indices);
    
    /* Use result */
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with exactly 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    asm volatile (
        /* Complex multi-operand pattern with constraints forcing RTL expansion */
        "mov %[res], %[a1] \n\t"
        "add %[res], %[a2] \n\t"
        "add %[res], %[a3] \n\t"
        "add %[res], %[a4] \n\t"
        "add %[res], %[a5] \n\t"
        "add %[res], %[a6] \n\t"
        "add %[res], %[a7] \n\t"
        "add %[res], %[a8] \n\t"
        "add %[res], %[a9] \n\t"
        "add %[res], %[a10]"
        : [res] "=r" (result)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    global_counter += result;
#endif
    
    printf("10-operand test completed: %d\n", global_counter);
}

/* ============================================
 * Function targeting 11-operand case  
 * ============================================ */
AGGRESSIVE_OPT
void test_11_operand(void) {
    printf("Testing 11-operand case...\n");
    
#if X86_ARCH
    /* AVX-512 complex blend with multiple sources and mask - may need 11 operands */
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __mmask8 mask1 = 0xAA;
    __mmask8 mask2 = 0x55;
    
    /* Complex sequence that might be combined into multi-operand RTL */
    __m512d temp = _mm512_mask_blend_pd(mask1, a, b);
    __m512d result = _mm512_mask_blend_pd(mask2, temp, c);
    
    /* Use result */
    global_counter += (int)_mm512_cvtpd_epi64(result)[0];
    
#elif ARM_ARCH
    /* ARM complex vector operations that might expand to many operands */
    float32x4_t v1 = vdupq_n_f32(1.0f);
    float32x4_t v2 = vdupq_n_f32(2.0f);
    float32x4_t v3 = vdupq_n_f32(3.0f);
    float32x4_t v4 = vdupq_n_f32(4.0f);
    
    /* Complex FMA chain that might be combined */
    float32x4_t result = vfmaq_f32(v1, v2, v3);
    result = vfmaq_f32(result, v4, v1);
    
    global_counter += (int)vgetq_lane_f32(result, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long result;
    
    asm volatile (
        /* 11-operand pattern with memory constraints to force complex RTL */
        "mov %[res], %[a1] \n\t"
        "imul %[res], %[a2] \n\t"
        "add %[res], %[a3] \n\t"
        "sub %[res], %[a4] \n\t"
        "add %[res], %[a5] \n\t"
        "sub %[res], %[a6] \n\t"
        "add %[res], %[a7] \n\t"
        "sub %[res], %[a8] \n\t"
        "add %[res], %[a9] \n\t"
        "sub %[res], %[a10] \n\t"
        "add %[res], %[a11]"
        : [res] "=r" (result)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10), [a11] "r" (op11)
        : "cc"
    );
    
    global_counter += result;
#endif
    
    printf("11-operand test completed: %d\n", global_counter);
}

/* ============================================
 * Additional complex vector pattern that might
 * trigger multi-operand RTL during optimization
 * ============================================ */
AGGRESSIVE_OPT
void complex_vector_pattern(void) {
#if X86_ARCH && defined(__AVX512F__)
    /* Complex shuffle/permute chain that GCC might combine */
    __m512i v1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i v2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i v3 = _mm512_set1_epi32(0xFFFFFFFF);
    
    /* Complex operation chain */
    __m512i shuffled = _mm512_shuffle_epi32(v1, _MM_PERM_ABCD);
    __m512i blended = _mm512_mask_blend_epi32(0xAAAA, shuffled, v2);
    __m512i result = _mm512_and_epi32(blended, v3);
    
    /* Force use */
    global_counter += _mm512_extract_epi32(result, 0);
#endif
}

/* ============================================
 * Main driver
 * ============================================ */
int main(void) {
    printf("Starting multi-operand RTL pattern test...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    complex_vector_pattern();
    
    printf("All tests completed. Global counter: %d\n", global_counter);
    
    /* Simple validation */
    if (global_counter > 0) {
        printf("SUCCESS: Code executed and produced results.\n");
        return 0;
    } else {
        printf("WARNING: No operations performed (may be unsupported arch).\n");
        return 1;
    }
}
