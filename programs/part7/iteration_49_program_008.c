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
    /* AVX-512 intrinsic that often expands to multi-operand RTL */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i a = _mm512_set1_epi64(1);
    __m512i b = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 k = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(a, k, idx, b);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE intrinsic with multiple operands */
    #include <arm_neon.h>
    
    /* Create a complex vector operation chain */
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    
    /* Multiple operations that might combine into one RTL pattern */
    uint8x16_t r1 = vaddq_u8(v1, v2);
    uint8x16_t r2 = vaddq_u8(v3, v4);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    uint8x16_t r4 = vmulq_u8(r3, v1);
    
    /* Extract and use result */
    global_counter += vgetq_lane_u8(r4, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to trigger 11-operand RTL pattern */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Use inline assembly with exactly 11 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result;
    
    asm volatile (
        /* Complex operation with 11 operands */
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result;
    
#elif defined(__aarch64__)
    /* ARM-specific multi-operand operation */
    #include <arm_neon.h>
    
    /* Create multiple vector registers */
    int32x4_t v1 = vdupq_n_s32(1);
    int32x4_t v2 = vdupq_n_s32(2);
    int32x4_t v3 = vdupq_n_s32(3);
    int32x4_t v4 = vdupq_n_s32(4);
    int32x4_t v5 = vdupq_n_s32(5);
    
    /* Complex chain of operations */
    int32x4_t r1 = vaddq_s32(v1, v2);
    int32x4_t r2 = vaddq_s32(v3, v4);
    int32x4_t r3 = vaddq_s32(r1, r2);
    int32x4_t r4 = vaddq_s32(r3, v5);
    int32x4_t r5 = vmulq_s32(r4, v1);
    int32x4_t r6 = vmlaq_s32(r5, v2, v3);
    
    /* Extract result */
    global_counter += vgetq_lane_s32(r6, 0);
    
#else
    /* Generic fallback with 11 operands in inline assembly */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
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
        /* Complex expression that might generate multi-operand RTL */
        float a = i * 1.0f;
        float b = i * 2.0f;
        float c = i * 3.0f;
        float d = i * 4.0f;
        float e = i * 5.0f;
        float f = i * 6.0f;
        float g = i * 7.0f;
        float h = i * 8.0f;
        
        /* Very complex expression that might require many operands */
        float result = (a + b) * (c + d) + (e * f) / (g + h) - 
                      (a * c) + (b * d) - (e * g) + (f * h);
        
        global_counter += (int)result;
    }
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_chain();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
