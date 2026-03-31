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
    /* AVX-512 intrinsic that often expands to multi-operand RTL */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON intrinsic with multiple operands */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex table lookup that may expand to multi-operand RTL */
    uint8x16_t result = vqtbl4q_u8(
        (uint8x16x4_t){a, b, c, d},
        vdupq_n_u8(0)
    );
    
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        /* Complex multi-operand pattern */
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

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Use inline assembly with exactly 11 operands */
    int64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    int64_t f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int64_t result;
    
    asm volatile (
        /* 11-operand computation pattern */
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result;
    
#elif defined(__aarch64__)
    /* ARM-specific multi-operand pattern using NEON */
    #include <arm_neon.h>
    
    /* Create a complex operation chain */
    uint8x16_t v0 = vdupq_n_u8(0);
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    uint8x16_t v5 = vdupq_n_u8(5);
    uint8x16_t v6 = vdupq_n_u8(6);
    uint8x16_t v7 = vdupq_n_u8(7);
    uint8x16_t v8 = vdupq_n_u8(8);
    uint8x16_t v9 = vdupq_n_u8(9);
    
    /* Complex sequence that may generate multi-operand RTL */
    uint8x16_t r1 = vaddq_u8(v0, v1);
    uint8x16_t r2 = vaddq_u8(r1, v2);
    uint8x16_t r3 = vaddq_u8(r2, v3);
    uint8x16_t r4 = vaddq_u8(r3, v4);
    uint8x16_t r5 = vaddq_u8(r4, v5);
    uint8x16_t r6 = vaddq_u8(r5, v6);
    uint8x16_t r7 = vaddq_u8(r6, v7);
    uint8x16_t r8 = vaddq_u8(r7, v8);
    uint8x16_t result = vaddq_u8(r8, v9);
    
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic 11-operand inline assembly */
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test using OpenMP SIMD to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Complex vector operation chain that might merge into multi-operand RTL */
    __m512i v0 = _mm512_set1_epi32(global_counter);
    __m512i v1 = _mm512_slli_epi32(v0, 1);
    __m512i v2 = _mm512_add_epi32(v1, v0);
    __m512i v3 = _mm512_srai_epi32(v2, 2);
    __m512i v4 = _mm512_and_si512(v3, _mm512_set1_epi32(0xFF));
    __m512i v5 = _mm512_or_si512(v4, _mm512_set1_epi32(0x100));
    __m512i v6 = _mm512_xor_si512(v5, _mm512_set1_epi32(0x55));
    
    /* Store with mask - potentially many operands */
    __mmask16 store_mask = 0xFFFF;
    int32_t buffer[16] __attribute__((aligned(64)));
    _mm512_mask_store_epi32(buffer, store_mask, v6);
    
    global_counter += buffer[0];
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_vector_chain();
    }
    
    printf("Result: %d\n", global_counter);
    return global_counter > 0 ? 0 : 1;
}
