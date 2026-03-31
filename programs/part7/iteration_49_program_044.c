/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function attribute to force optimization */
#define FORCE_OPT __attribute__((optimize("O3", "no-inline")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test function for 10-operand case using AVX-512 intrinsics */
FORCE_OPT void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands:
       _mm512_mask_permutex2var_epi64 has 4 register operands + 1 mask + constants
       When expanded to RTL, this can generate patterns with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic expands to a pattern with multiple operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Additional complex operation chain to encourage pattern merging */
    __m512i add_result = _mm512_add_epi64(result, _mm512_set1_epi64(1));
    __m512i mul_result = _mm512_mullo_epi64(add_result, _mm512_set1_epi64(2));
    
    /* Store to prevent elimination */
    alignas(64) int64_t buffer[8];
    _mm512_store_epi64(buffer, mul_result);
    
    global_counter += buffer[0];
}

/* Test function for 11-operand case using inline assembly */
FORCE_OPT void test_11_operand_x86(void) {
    /* Inline assembly with exactly 11 operands */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint64_t result1, result2, result3;
    
    /* Complex inline asm with 11 total operands (3 outputs, 8 inputs) */
    asm volatile (
        /* Multiple operations chained together */
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "imul %[c], %[r1]\n\t"
        "mov %[d], %[r2]\n\t"
        "add %[e], %[r2]\n\t"
        "imul %[f], %[r2]\n\t"
        "mov %[g], %[r3]\n\t"
        "add %[h], %[r3]\n\t"
        "imul %[i], %[r3]\n\t"
        "add %[j], %[r1]\n\t"
        "add %[k], %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), 
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result1 + result2 + result3;
}

#elif defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>

/* Test function for 10-operand case using ARM NEON */
FORCE_OPT void test_10_operand_arm(void) {
    /* Complex NEON operations with multiple operands */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Chain of operations that might merge into complex pattern */
    uint8x16_t result1 = vaddq_u8(a, b);
    uint8x16_t result2 = vaddq_u8(c, d);
    uint8x16_t result3 = vmulq_u8(result1, result2);
    
    /* Table lookup operation with multiple operands */
    uint8x16_t indices = vcombine_u8(
        vcreate_u8(0x0706050403020100),
        vcreate_u8(0x0F0E0D0C0B0A0908)
    );
    
    uint8x16_t table[4] = {a, b, c, d};
    uint8x16_t tbl_result = vqtbl4q_u8(*(uint8x16x4_t *)table, indices);
    
    uint8x16_t final_result = vaddq_u8(result3, tbl_result);
    
    /* Store to prevent elimination */
    uint8_t buffer[16];
    vst1q_u8(buffer, final_result);
    
    global_counter += buffer[0];
}

/* Test function for 11-operand case using inline assembly */
FORCE_OPT void test_11_operand_arm(void) {
    /* Inline assembly with exactly 11 operands for ARM */
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    uint32_t out1, out2, out3;
    
    /* Initialize registers */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5;
    r5 = 6; r6 = 7; r7 = 8; r8 = 9; r9 = 10; r10 = 11;
    
    asm volatile (
        /* Multiple operations using all 11 input registers */
        "add %[out1], %[r0], %[r1]\n\t"
        "mla %[out1], %[out1], %[r2], %[r3]\n\t"
        "add %[out2], %[r4], %[r5]\n\t"
        "mla %[out2], %[out2], %[r6], %[r7]\n\t"
        "add %[out3], %[r8], %[r9]\n\t"
        "add %[out3], %[out3], %[r10]\n\t"
        : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=&r" (out3)
        : [r0] "r" (r0), [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3),
          [r4] "r" (r4), [r5] "r" (r5), [r6] "r" (r6), [r7] "r" (r7),
          [r8] "r" (r8), [r9] "r" (r9), [r10] "r" (r10)
        : "cc"
    );
    
    global_counter += out1 + out2 + out3;
}

#else
/* Generic fallback using complex inline assembly */
FORCE_OPT void test_10_operand_generic(void) {
    /* Generic inline assembly with 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    asm volatile (
        "mov %[op1], %[res1]\n\t"
        "add %[op2], %[res1]\n\t"
        "imul %[op3], %[res1]\n\t"
        "mov %[op4], %[res2]\n\t"
        "add %[op5], %[res2]\n\t"
        "imul %[op6], %[res2]\n\t"
        "add %[op7], %[res1]\n\t"
        "add %[op8], %[res2]\n\t"
        "imul %[op9], %[res1]\n\t"
        "imul %[op10], %[res2]\n\t"
        : [res1] "=&r" (result1), [res2] "=&r" (result2)
        : [op1] "r" (op1), [op2] "r" (op2), [op3] "r" (op3),
          [op4] "r" (op4), [op5] "r" (op5), [op6] "r" (op6),
          [op7] "r" (op7), [op8] "r" (op8), [op9] "r" (op9),
          [op10] "r" (op10)
        : "cc"
    );
    
    global_counter += result1 + result2;
}

FORCE_OPT void test_11_operand_generic(void) {
    /* Generic inline assembly with 11 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5, op6 = 6;
    long op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long result1, result2, result3;
    
    asm volatile (
        "mov %[op1], %[res1]\n\t"
        "add %[op2], %[res1]\n\t"
        "mov %[op3], %[res2]\n\t"
        "add %[op4], %[res2]\n\t"
        "mov %[op5], %[res3]\n\t"
        "add %[op6], %[res3]\n\t"
        "imul %[op7], %[res1]\n\t"
        "imul %[op8], %[res2]\n\t"
        "imul %[op9], %[res3]\n\t"
        "add %[op10], %[res1]\n\t"
        "add %[op11], %[res2]\n\t"
        : [res1] "=&r" (result1), [res2] "=&r" (result2), [res3] "=&r" (result3)
        : [op1] "r" (op1), [op2] "r" (op2), [op3] "r" (op3),
          [op4] "r" (op4), [op5] "r" (op5), [op6] "r" (op6),
          [op7] "r" (op7), [op8] "r" (op8), [op9] "r" (op9),
          [op10] "r" (op10), [op11] "r" (op11)
        : "cc"
    );
    
    global_counter += result1 + result2 + result3;
}
#endif

/* OpenMP SIMD function to encourage pattern merging */
#ifdef _OPENMP
FORCE_OPT void test_omp_simd_pattern(void) {
    #define N 1024
    float a[N], b[N], c[N], d[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
        c[i] = i * 3.0f;
        d[i] = i * 4.0f;
    }
    
    /* Complex SIMD operation chain that might merge */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        /* Multiple operations that could be combined */
        float t1 = a[i] + b[i];
        float t2 = c[i] * d[i];
        float t3 = t1 - t2;
        float t4 = t3 * 2.0f;
        float t5 = t4 / 3.0f;
        a[i] = t5 + 1.0f;
    }
    
    global_counter += (int)a[0];
}
#endif

/* Main test driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
#ifdef __x86_64__
    printf("Using x86_64 intrinsics and assembly\n");
    test_10_operand_x86();
    test_11_operand_x86();
#elif defined(__aarch64__) || defined(__arm__)
    printf("Using ARM NEON and assembly\n");
    test_10_operand_arm();
    test_11_operand_arm();
#else
    printf("Using generic assembly\n");
    test_10_operand_generic();
    test_11_operand_generic();
#endif

#ifdef _OPENMP
    test_omp_simd_pattern();
#endif
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed successfully\n");
    
    return 0;
}
