/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* x86_64: Use AVX-512 masked permute with multiple operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Extract and use result to prevent optimization */
    long long temp[8];
    _mm512_storeu_si512(temp, result);
    global_counter += (int)temp[0];
    #endif
    
#elif defined(__aarch64__)
    /* ARM64: Use complex NEON operations */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex sequence that might generate multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    uint8x16_t r4 = vmulq_u8(r3, a);
    
    /* Extract lane to prevent optimization */
    global_counter += vgetq_lane_u8(r4, 0);
    
#else
    /* Generic: Use inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        /* Complex operation with 10 operands */
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
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* x86_64: Complex AVX-512 operation with immediate */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    /* Complex blend with multiple sources and mask */
    __m512i r1 = _mm512_add_epi32(a, b);
    __m512i r2 = _mm512_add_epi32(c, d);
    __mmask16 mask = _mm512_cmp_epi32_mask(r1, r2, _MM_CMPINT_EQ);
    __m512i result = _mm512_mask_blend_epi32(mask, r1, r2);
    
    /* Further operations to encourage complex pattern */
    result = _mm512_slli_epi32(result, 2);
    result = _mm512_add_epi32(result, a);
    
    int temp[16];
    _mm512_storeu_si512(temp, result);
    global_counter += temp[0];
    #endif
    
#elif defined(__aarch64__)
    /* ARM64: Extended inline assembly with 11 operands */
    int64x2_t v1 = vdupq_n_s64(1);
    int64x2_t v2 = vdupq_n_s64(2);
    int64x2_t v3 = vdupq_n_s64(3);
    int64x2_t v4 = vdupq_n_s64(4);
    int64x2_t v5 = vdupq_n_s64(5);
    int64x2_t result;
    
    /* Complex inline assembly that might expand to 11 operands */
    asm volatile (
        "add %0.2d, %1.2d, %2.2d\n\t"
        "add %0.2d, %0.2d, %3.2d\n\t"
        "add %0.2d, %0.2d, %4.2d\n\t"
        "add %0.2d, %0.2d, %5.2d\n\t"
        "mul %0.2d, %0.2d, %1.2d"
        : "=w" (result)
        : "w" (v1), "w" (v2), "w" (v3), "w" (v4), "w" (v5)
        : /* No clobbers */
    );
    
    global_counter += vgetq_lane_s64(result, 0);
    
#else
    /* Generic: Explicit inline assembly with 11 operands */
    int op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    int op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    int result;
    
    asm volatile (
        /* Operation using all 11 input operands */
        "mov %0, %1\n\t"
        "imul %0, %0, %2\n\t"
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
        : "r" (op1), "r" (op2), "r" (op3), "r" (op4), "r" (op5),
          "r" (op6), "r" (op7), "r" (op8), "r" (op9), "r" (op10), "r" (op11)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test using OpenMP SIMD to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (int i = 0; i < 1024; i++) {
        /* Complex expression that might generate multi-operand pattern */
        float a = i * 1.0f;
        float b = i * 2.0f;
        float c = i * 3.0f;
        float d = i * 4.0f;
        float e = i * 5.0f;
        float f = i * 6.0f;
        float g = i * 7.0f;
        float h = i * 8.0f;
        float j = i * 9.0f;
        float k = i * 10.0f;
        
        /* Very complex expression - might be optimized into single pattern */
        float result = (a + b) * (c - d) + (e * f) / (g + h) - (j * k);
        
        global_counter += (int)result;
    }
}

int main(void) {
    printf("Testing multi-operand patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", global_counter);
    return 0;
}
