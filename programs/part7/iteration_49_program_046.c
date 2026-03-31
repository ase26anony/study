/* Test program to trigger 10 and 11 operand RTL patterns in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile = 0;

/* Function to trigger 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 intrinsic that often expands to many operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    g_volatile += _mm512_extract_epi64(result, 0);
    #endif
    
    /* Alternative: Multi-operand inline assembly for x86_64 */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    uint64_t out1, out2;
    
    asm volatile (
        /* 10 explicit operands */
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "mov %[c], %[out2]\n\t"
        "add %[d], %[out2]\n\t"
        "imul %[e], %[out1]\n\t"
        "imul %[f], %[out2]\n\t"
        "add %[g], %[out1]\n\t"
        "add %[h], %[out2]\n\t"
        "sub %[i], %[out1]\n\t"
        "sub %[j], %[out2]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    g_volatile += out1 + out2;
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE intrinsics */
    #ifdef __ARM_NEON
    #include <arm_neon.h>
    
    /* Complex vector operations that may expand to many operands */
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    
    /* Chain of operations that might be combined */
    uint8x16_t r1 = vaddq_u8(v1, v2);
    uint8x16_t r2 = vaddq_u8(v3, v4);
    uint8x16_t r3 = vmulq_u8(r1, r2);
    uint8x16_t r4 = veorq_u8(r3, v1);
    
    /* Extract and use result */
    uint8_t lane = vgetq_lane_u8(r4, 0);
    g_volatile += lane;
    #endif
    
    /* Multi-operand inline assembly for ARM */
    uint64_t x0 = 1, x1 = 2, x2 = 3, x3 = 4, x4 = 5;
    uint64_t x5 = 6, x6 = 7, x7 = 8, x8 = 9, x9 = 10;
    uint64_t res1, res2;
    
    asm volatile (
        /* 10 register operands */
        "add %[res1], %[x0], %[x1]\n\t"
        "add %[res2], %[x2], %[x3]\n\t"
        "mul %[res1], %[res1], %[x4]\n\t"
        "mul %[res2], %[res2], %[x5]\n\t"
        "add %[res1], %[res1], %[x6]\n\t"
        "add %[res2], %[res2], %[x7]\n\t"
        "sub %[res1], %[res1], %[x8]\n\t"
        "sub %[res2], %[res2], %[x9]"
        : [res1] "=&r" (res1), [res2] "=&r" (res2)
        : [x0] "r" (x0), [x1] "r" (x1), [x2] "r" (x2), [x3] "r" (x3),
          [x4] "r" (x4), [x5] "r" (x5), [x6] "r" (x6), [x7] "r" (x7),
          [x8] "r" (x8), [x9] "r" (x9)
        : "cc"
    );
    
    g_volatile += res1 + res2;
#else
    /* Generic fallback with multi-operand inline assembly */
    uint64_t v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    uint64_t v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    uint64_t o1, o2;
    
    asm volatile (
        "mov %[v1], %[o1]\n\t"
        "add %[v2], %[o1]\n\t"
        "mov %[v3], %[o2]\n\t"
        "add %[v4], %[o2]\n\t"
        "imul %[v5], %[o1]\n\t"
        "imul %[v6], %[o2]\n\t"
        "add %[v7], %[o1]\n\t"
        "add %[v8], %[o2]\n\t"
        "sub %[v9], %[o1]\n\t"
        "sub %[v10], %[o2]"
        : [o1] "=&r" (o1), [o2] "=&r" (o2)
        : [v1] "r" (v1), [v2] "r" (v2), [v3] "r" (v3), [v4] "r" (v4),
          [v5] "r" (v5), [v6] "r" (v6), [v7] "r" (v7), [v8] "r" (v8),
          [v9] "r" (v9), [v10] "r" (v10)
        : "cc"
    );
    
    g_volatile += o1 + o2;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Complex AVX-512 operation with mask and immediate */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i v1 = _mm512_set1_epi32(1);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __mmask16 m1 = 0xAAAA;
    __mmask16 m2 = 0x5555;
    
    /* Multi-operation chain that might be combined */
    __m512i t1 = _mm512_mask_add_epi32(v1, m1, v2, v3);
    __m512i t2 = _mm512_mask_sub_epi32(v2, m2, v3, v1);
    __m512i result = _mm512_mask_blend_epi32(m1, t1, t2);
    
    g_volatile += _mm512_extract_epi32(result, 0);
    #endif
    
    /* Inline assembly with exactly 11 operands */
    uint64_t a=1, b=2, c=3, d=4, e=5, f=6, g_val=7, h=8, i=9, j=10, k=11;
    uint64_t out1, out2, out3;
    
    asm volatile (
        /* 11 explicit operands */
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "mov %[c], %[out2]\n\t"
        "add %[d], %[out2]\n\t"
        "mov %[e], %[out3]\n\t"
        "add %[f], %[out3]\n\t"
        "imul %[g], %[out1]\n\t"
        "imul %[h], %[out2]\n\t"
        "imul %[i], %[out3]\n\t"
        "add %[j], %[out1]\n\t"
        "add %[k], %[out2]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=&r" (out3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g_val), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    g_volatile += out1 + out2 + out3;
    
#elif defined(__aarch64__)
    /* ARM SVE/NEON with many operands */
    #ifdef __ARM_NEON
    #include <arm_neon.h>
    
    /* Complex vector shuffle/permute that may need many operands */
    uint8x16_t va = vdupq_n_u8(1);
    uint8x16_t vb = vdupq_n_u8(2);
    uint8x16_t vc = vdupq_n_u8(3);
    uint8x16_t vd = vdupq_n_u8(4);
    uint8x16_t ve = vdupq_n_u8(5);
    
    uint8x16_t r1 = vaddq_u8(va, vb);
    uint8x16_t r2 = vaddq_u8(vc, vd);
    uint8x16_t r3 = vmulq_u8(r1, r2);
    uint8x16_t r4 = vaddq_u8(r3, ve);
    uint8x16_t result = vextq_u8(r4, va, 8);
    
    g_volatile += vgetq_lane_u8(result, 0);
    #endif
    
    /* 11-operand inline assembly for ARM */
    uint64_t x0=1, x1=2, x2=3, x3=4, x4=5, x5=6, x6=7, x7=8, x8=9, x9=10, x10=11;
    uint64_t r1, r2, r3;
    
    asm volatile (
        "add %[r1], %[x0], %[x1]\n\t"
        "add %[r2], %[x2], %[x3]\n\t"
        "add %[r3], %[x4], %[x5]\n\t"
        "mul %[r1], %[r1], %[x6]\n\t"
        "mul %[r2], %[r2], %[x7]\n\t"
        "mul %[r3], %[r3], %[x8]\n\t"
        "add %[r1], %[r1], %[x9]\n\t"
        "add %[r2], %[r2], %[x10]"
        : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3)
        : [x0] "r" (x0), [x1] "r" (x1), [x2] "r" (x2), [x3] "r" (x3),
          [x4] "r" (x4), [x5] "r" (x5), [x6] "r" (x6), [x7] "r" (x7),
          [x8] "r" (x8), [x9] "r" (x9), [x10] "r" (x10)
        : "cc"
    );
    
    g_volatile += r1 + r2 + r3;
#else
    /* Generic 11-operand inline assembly */
    uint64_t v1=1, v2=2, v3=3, v4=4, v5=5, v6=6, v7=7, v8=8, v9=9, v10=10, v11=11;
    uint64_t o1, o2, o3;
    
    asm volatile (
        "mov %[v1], %[o1]\n\t"
        "add %[v2], %[o1]\n\t"
        "mov %[v3], %[o2]\n\t"
        "add %[v4], %[o2]\n\t"
        "mov %[v5], %[o3]\n\t"
        "add %[v6], %[o3]\n\t"
        "imul %[v7], %[o1]\n\t"
        "imul %[v8], %[o2]\n\t"
        "imul %[v9], %[o3]\n\t"
        "add %[v10], %[o1]\n\t"
        "add %[v11], %[o2]"
        : [o1] "=&r" (o1), [o2] "=&r" (o2), [o3] "=&r" (o3)
        : [v1] "r" (v1), [v2] "r" (v2), [v3] "r" (v3), [v4] "r" (v4),
          [v5] "r" (v5), [v6] "r" (v6), [v7] "r" (v7), [v8] "r" (v8),
          [v9] "r" (v9), [v10] "r" (v10), [v11] "r" (v11)
        : "cc"
    );
    
    g_volatile += o1 + o2 + o3;
#endif
}

/* Additional complex function that might generate multi-operand RTL */
__attribute__((noinline, optimize("O3")))
void complex_vector_operations(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Complex sequence that might be combined into a single pattern */
    __m512i a = _mm512_set1_epi64(1);
    __m512i b = _mm512_set1_epi64(2);
    __m512i c = _mm512_set1_epi64(3);
    __m512i d = _mm512_set1_epi64(4);
    __mmask8 m = 0x0F;
    
    __m512i t1 = _mm512_mask_add_epi64(a, m, b, c);
    __m512i t2 = _mm512_mask_sub_epi64(b, m, c, d);
    __m512i t3 = _mm512_mask_mul_epi64(t1, m, t2, a);
    __m512i result = _mm512_mask_slli_epi64(t3, m, t3, 2);
    
    g_volatile += _mm512_extract_epi64(result, 0);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call both test functions */
    test_10_operand();
    test_11_operand();
    complex_vector_operations();
    
    printf("Result: %d\n", g_volatile);
    return g_volatile != 0 ? 0 : 1;
}
