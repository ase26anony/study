/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* x86_64: Use AVX-512 masked permute with multiple operands */
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic expands to an RTL pattern with many operands:
       dest, mask, idx, src1, src2, plus various immediates */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
    
#elif defined(__aarch64__)
    /* AArch64: Use complex NEON operations */
    #include <arm_neon.h>
    
    /* Create multiple vector registers */
    uint8x16_t v0 = vdupq_n_u8(1);
    uint8x16_t v1 = vdupq_n_u8(2);
    uint8x16_t v2 = vdupq_n_u8(3);
    uint8x16_t v3 = vdupq_n_u8(4);
    uint8x16_t v4 = vdupq_n_u8(5);
    
    /* Complex sequence that might generate multi-operand RTL */
    uint8x16_t r1 = vaddq_u8(v0, v1);
    uint8x16_t r2 = vaddq_u8(v2, v3);
    uint8x16_t result = vaddq_u8(r1, r2);
    result = vaddq_u8(result, v4);
    
    /* Use result */
    uint8_t temp[16];
    vst1q_u8(temp, result);
    global_counter += temp[0];
    
#else
    /* Generic: Use inline assembly with 10 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9, j = 10;
    long result;
    
    asm volatile (
        /* Complex multi-operand operation */
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* x86_64: Complex AVX-512 blend with multiple sources and mask */
    #include <immintrin.h>
    
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __m512d d = _mm512_set1_pd(4.0);
    __mmask8 m1 = 0xAA;
    __mmask8 m2 = 0x55;
    
    /* Complex sequence that may generate 11-operand RTL */
    __m512d t1 = _mm512_mask_blend_pd(m1, a, b);
    __m512d t2 = _mm512_mask_blend_pd(m2, c, d);
    __m512d result = _mm512_add_pd(t1, t2);
    
    /* Further complex operation with immediate */
    result = _mm512_maskz_mov_pd(0xFF, result);
    
    /* Use result */
    double temp[8];
    _mm512_storeu_pd(temp, result);
    global_counter += (int)temp[0];
    
#elif defined(__aarch64__)
    /* AArch64: Multiple vector operations in sequence */
    #include <arm_neon.h>
    
    float32x4_t v0 = vdupq_n_f32(1.0f);
    float32x4_t v1 = vdupq_n_f32(2.0f);
    float32x4_t v2 = vdupq_n_f32(3.0f);
    float32x4_t v3 = vdupq_n_f32(4.0f);
    float32x4_t v4 = vdupq_n_f32(5.0f);
    float32x4_t v5 = vdupq_n_f32(6.0f);
    
    /* Complex reduction chain */
    float32x4_t r1 = vaddq_f32(v0, v1);
    float32x4_t r2 = vaddq_f32(v2, v3);
    float32x4_t r3 = vaddq_f32(v4, v5);
    float32x4_t r4 = vaddq_f32(r1, r2);
    float32x4_t result = vaddq_f32(r4, r3);
    
    /* Additional operation with immediate */
    result = vmulq_n_f32(result, 2.0f);
    
    /* Use result */
    float temp[4];
    vst1q_f32(temp, result);
    global_counter += (int)temp[0];
    
#else
    /* Generic: Inline assembly with exactly 11 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result;
    
    asm volatile (
        /* 11-operand computation */
        "mov %[res], %[a]\n\t"
        "add %[res], %[res], %[b]\n\t"
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j),
          [k] "r" (k)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test using OpenMP SIMD pragmas to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (int i = 0; i < 1024; i++) {
        /* Complex computation that might generate multi-operand RTL */
        float a = i * 1.0f;
        float b = i * 2.0f;
        float c = i * 3.0f;
        float d = i * 4.0f;
        float e = i * 5.0f;
        float f = i * 6.0f;
        float g = i * 7.0f;
        float h = i * 8.0f;
        
        /* Chain of operations */
        float r1 = a + b + c + d;
        float r2 = e + f + g + h;
        float result = r1 * r2;
        
        global_counter += (int)result;
    }
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Result: %d\n", global_counter);
    printf("Test completed successfully.\n");
    
    return 0;
}
