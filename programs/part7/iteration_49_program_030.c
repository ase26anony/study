/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile = 0;

/* Function to trigger 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 complex permute with mask - can generate 10 operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    __m512i result;
    
    /* Complex permute with mask - potentially expands to 10 operands */
    result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    g_volatile += _mm512_extract_epi64(result, 0);
    #endif
    
    /* Alternative: Multi-operand inline assembly for x86_64 */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result_asm;
    
    asm volatile (
        /* 10 operands: 5 inputs, 1 output, 4 clobbers */
        "mov %[op1], %[res]\n\t"
        "add %[op2], %[res]\n\t"
        "add %[op3], %[res]\n\t"
        "add %[op4], %[res]\n\t"
        "add %[op5], %[res]\n\t"
        "imul %[op6], %[res]\n\t"
        "sub %[op7], %[res]\n\t"
        "xor %[op8], %[res]\n\t"
        "or %[op9], %[res]\n\t"
        "and %[op10], %[res]\n\t"
        : [res] "=&r" (result_asm)
        : [op1] "r" (op1), [op2] "r" (op2), [op3] "r" (op3),
          [op4] "r" (op4), [op5] "r" (op5), [op6] "r" (op6),
          [op7] "r" (op7), [op8] "r" (op8), [op9] "r" (op9),
          [op10] "r" (op10)
        : "cc", "memory"
    );
    
    g_volatile += result_asm;
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE complex operations */
    #ifdef __ARM_NEON
    #include <arm_neon.h>
    
    /* Complex vector operation chain that might expand to 10 operands */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Multiple operations chained - compiler might combine */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vmulq_u8(r1, r2);
    uint8x16_t r4 = veorq_u8(r3, a);
    uint8x16_t r5 = vbslq_u8(r4, b, c);  /* Bitwise select - 3 operands */
    
    /* Extract and use */
    g_volatile += vgetq_lane_u8(r5, 0);
    #endif
    
    /* Generic multi-operand inline assembly for ARM */
    register long r0 asm("x0") = 1;
    register long r1 asm("x1") = 2;
    register long r2 asm("x2") = 3;
    register long r3 asm("x3") = 4;
    register long r4 asm("x4") = 5;
    register long r5 asm("x5") = 6;
    register long r6 asm("x6") = 7;
    register long r7 asm("x7") = 8;
    register long r8 asm("x8") = 9;
    register long r9 asm("x9") = 10;
    register long result_reg asm("x10");
    
    asm volatile (
        /* 10 register operands */
        "add %[res], %[r0], %[r1]\n\t"
        "add %[res], %[res], %[r2]\n\t"
        "add %[res], %[res], %[r3]\n\t"
        "add %[res], %[res], %[r4]\n\t"
        "mul %[res], %[res], %[r5]\n\t"
        "sub %[res], %[res], %[r6]\n\t"
        "eor %[res], %[res], %[r7]\n\t"
        "orr %[res], %[res], %[r8]\n\t"
        "and %[res], %[res], %[r9]\n\t"
        : [res] "=&r" (result_reg)
        : [r0] "r" (r0), [r1] "r" (r1), [r2] "r" (r2),
          [r3] "r" (r3), [r4] "r" (r4), [r5] "r" (r5),
          [r6] "r" (r6), [r7] "r" (r7), [r8] "r" (r8),
          [r9] "r" (r9)
        : "cc"
    );
    
    g_volatile += result_reg;
    
#else
    /* Generic fallback with complex expression that might generate multi-operand RTL */
    long a = 1, b = 2, c = 3, d = 4, e = 5;
    long f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Complex expression that might be optimized into a multi-operand pattern */
    long result = (((((((((a + b) * c) - d) ^ e) | f) & g) + h) - i) * j);
    g_volatile += result;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Use inline assembly with exactly 11 operands */
    long in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    long in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10, in11 = 11;
    long out1, out2;
    
    asm volatile (
        /* 11 operands: 11 inputs/outputs/clobbers total */
        "mov %[i1], %[o1]\n\t"
        "add %[i2], %[o1]\n\t"
        "mov %[i3], %[o2]\n\t"
        "add %[i4], %[o2]\n\t"
        "imul %[o1], %[o2]\n\t"
        "add %[i5], %[o2]\n\t"
        "sub %[i6], %[o2]\n\t"
        "xor %[i7], %[o2]\n\t"
        "or %[i8], %[o2]\n\t"
        "and %[i9], %[o2]\n\t"
        "add %[i10], %[o2]\n\t"
        "sub %[i11], %[o2]\n\t"
        : [o1] "=&r" (out1), [o2] "=&r" (out2)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7), [i8] "r" (in8), [i9] "r" (in9),
          [i10] "r" (in10), [i11] "r" (in11)
        : "cc"
    );
    
    g_volatile += out1 + out2;
    
    /* AVX-512 masked gather with multiple parameters - can need 11 operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i vindex = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i src = _mm512_set1_epi64(42);
    __mmask8 kmask = 0xFF;
    int scale = 8;
    long base_addr = (long)&g_volatile;
    __m512i gather_result;
    
    /* Gather with mask - many operands */
    gather_result = _mm512_mask_i64gather_epi64(src, kmask, vindex, 
                                               (void*)base_addr, scale);
    
    g_volatile += _mm512_extract_epi64(gather_result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM inline assembly with 11 operands */
    register long a0 asm("x0") = 1;
    register long a1 asm("x1") = 2;
    register long a2 asm("x2") = 3;
    register long a3 asm("x3") = 4;
    register long a4 asm("x4") = 5;
    register long a5 asm("x5") = 6;
    register long a6 asm("x6") = 7;
    register long a7 asm("x7") = 8;
    register long a8 asm("x8") = 9;
    register long a9 asm("x9") = 10;
    register long a10 asm("x10") = 11;
    register long out_a asm("x11");
    register long out_b asm("x12");
    
    asm volatile (
        /* 11 register operands in use */
        "add %[out1], %[a0], %[a1]\n\t"
        "add %[out2], %[a2], %[a3]\n\t"
        "mul %[out1], %[out1], %[out2]\n\t"
        "add %[out1], %[out1], %[a4]\n\t"
        "add %[out1], %[out1], %[a5]\n\t"
        "sub %[out1], %[out1], %[a6]\n\t"
        "eor %[out1], %[out1], %[a7]\n\t"
        "orr %[out1], %[out1], %[a8]\n\t"
        "and %[out1], %[out1], %[a9]\n\t"
        "add %[out1], %[out1], %[a10]\n\t"
        : [out1] "=&r" (out_a), [out2] "=&r" (out_b)
        : [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2),
          [a3] "r" (a3), [a4] "r" (a4), [a5] "r" (a5),
          [a6] "r" (a6), [a7] "r" (a7), [a8] "r" (a8),
          [a9] "r" (a9), [a10] "r" (a10)
        : "cc"
    );
    
    g_volatile += out_a + out_b;
    
#else
    /* Generic fallback - complex expression with 11 variables */
    long v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    long v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    
    /* Very complex expression that might generate 11-operand RTL */
    long result = v1 + v2 * v3 - v4 ^ v5 | v6 & v7 + v8 - v9 * v10 + v11;
    result = ((((((((((v1 + v2) * v3) - v4) ^ v5) | v6) & v7) + v8) - v9) * v10) + v11);
    g_volatile += result;
#endif
}

/* Additional test using OpenMP SIMD to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (int i = 0; i < 100; i++) {
        /* Complex vectorizable operation chain */
        g_volatile += i * 2 - i / 3 + (i % 7) ^ (i & 15) | (i << 2);
    }
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call both test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_vector_chain();
    }
    
    printf("Result: %d\n", g_volatile);
    return 0;
}
