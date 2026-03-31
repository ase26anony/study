/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile = 0;

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function targeting 10-operand case using AVX-512 intrinsics */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 expands to many operands:
       dest, mask, idx, src1, src2 */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    g_volatile = _mm512_extract_epi64(result, 0);
}

/* Another 10-operand pattern using blend with multiple immediates */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86_2(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __mmask8 m = 0xAA;
    
    /* Complex masked operation */
    __m512d result = _mm512_mask_blend_pd(m, a, b);
    
    /* Force use of result */
    g_volatile = (int)_mm512_cvtsd_f64(result);
}

/* Function targeting 11-operand case */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_x86(void) {
    /* Use inline assembly with exactly 11 operands */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint64_t result1, result2, result3;
    
    /* Complex inline assembly with 11 operands:
       5 inputs, 3 outputs, 3 clobbers = 11 total */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "imul %[d], %[r2]\n\t"
        "mov %[e], %[r3]\n\t"
        "xor %[f], %[r3]\n\t"
        "add %[g], %[r1]\n\t"
        "sub %[h], %[r2]\n\t"
        "or %[i], %[r3]\n\t"
        "and %[j], %[r1]\n\t"
        "shl $3, %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc", "memory"
    );
    
    g_volatile = result1 + result2 + result3 + k;
}

#elif defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>

/* Function targeting 10-operand case using ARM NEON */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_arm(void) {
    /* Complex NEON operation chain */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Multiple operations that might combine */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vmulq_u8(r1, r2);
    uint8x16_t r4 = vshlq_u8(r3, vdupq_n_u8(2));
    
    /* Table lookup with multiple registers - can generate many operands */
    uint8x16_t result = vqtbl1q_u8(r4, vcreate_u8(0x0706050403020100));
    
    g_volatile = vgetq_lane_u8(result, 0);
}

/* Function targeting 11-operand case for ARM */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_arm(void) {
    /* Inline assembly with 11 operands for ARM */
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    uint32_t out1, out2, out3;
    
    /* Initialize registers */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5;
    r5 = 6; r6 = 7; r7 = 8; r8 = 9; r9 = 10; r10 = 11;
    
    asm volatile (
        "add %[out1], %[r0], %[r1]\n\t"
        "add %[out1], %[out1], %[r2]\n\t"
        "mul %[out2], %[r3], %[r4]\n\t"
        "add %[out2], %[out2], %[r5]\n\t"
        "eor %[out3], %[r6], %[r7]\n\t"
        "orr %[out3], %[out3], %[r8]\n\t"
        "and %[out1], %[out1], %[r9]\n\t"
        "lsl %[out2], %[out2], #2\n\t"
        : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=&r" (out3)
        : [r0] "r" (r0), [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3),
          [r4] "r" (r4), [r5] "r" (r5), [r6] "r" (r6), [r7] "r" (r7),
          [r8] "r" (r8), [r9] "r" (r9)
        : "cc"
    );
    
    g_volatile = out1 + out2 + out3 + r10;
}

#else
/* Generic fallback using complex inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result1, result2;
    
    /* 10 operands: 8 inputs, 2 outputs */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "add %[d], %[r2]\n\t"
        "imul %[e], %[r1]\n\t"
        "sub %[f], %[r2]\n\t"
        "xor %[g], %[r1]\n\t"
        "or %[h], %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc"
    );
    
    g_volatile = result1 + result2 + i + j;
}

__attribute__((optimize("O3,no-inline")))
void test_11_operand_generic(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result1, result2, result3;
    
    /* 11 operands: 8 inputs, 3 outputs */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "add %[d], %[r2]\n\t"
        "mov %[e], %[r3]\n\t"
        "add %[f], %[r3]\n\t"
        "imul %[g], %[r1]\n\t"
        "sub %[h], %[r2]\n\t"
        "xor %[i], %[r3]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    g_volatile = result1 + result2 + result3 + j + k;
}
#endif

/* OpenMP SIMD function that might generate complex patterns */
__attribute__((optimize("O3,no-inline")))
void test_simd_combine(void) {
    #define N 1024
    float a[N], b[N], c[N], d[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
        c[i] = i * 3.0f;
        d[i] = i * 4.0f;
    }
    
    /* Complex SIMD operation chain that might combine */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        a[i] = a[i] + b[i] * c[i] - d[i] / (a[i] + 1.0f);
        b[i] = c[i] * d[i] - a[i] / (b[i] + 1.0f);
        c[i] = d[i] + a[i] * b[i] - c[i] / (d[i] + 1.0f);
    }
    
    g_volatile = (int)a[N-1] + (int)b[N-2] + (int)c[N-3];
}

/* Main driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
#ifdef __x86_64__
    test_10_operand_x86();
    test_10_operand_x86_2();
    test_11_operand_x86();
#elif defined(__aarch64__) || defined(__arm__)
    test_10_operand_arm();
    test_11_operand_arm();
#else
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    test_simd_combine();
    
    printf("Result: %d\n", g_volatile);
    return 0;
}
