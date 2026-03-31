/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function to trigger 10-operand case using AVX-512 intrinsics */
__attribute__((optimize("O3")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands:
       _mm512_mask_permutex2var_epi64 has 4 register operands + 1 mask + constants
       When expanded, this should create an RTL pattern with 10 operands */
    __m512i src1 = _mm512_set_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i src2 = _mm512_set_epi64(9, 10, 11, 12, 13, 14, 15, 16);
    __mmask8 mask = 0xFF;
    
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
}

/* Function to trigger 11-operand case using multiple AVX-512 operations */
__attribute__((optimize("O3")))
void test_11_operand_x86(void) {
    /* Complex sequence that might be combined into a single RTL pattern */
    __m512i a = _mm512_set_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    __m512i b = _mm512_set_epi32(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    __m512i c = _mm512_set_epi32(1, 3, 5, 7, 9, 11, 13, 15, 2, 4, 6, 8, 10, 12, 14, 16);
    
    /* Complex blend with multiple masks and shuffles */
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    
    __m512i temp = _mm512_mask_blend_epi32(mask1, a, b);
    __m512i result = _mm512_mask_permutexvar_epi32(c, mask2, b, temp);
    
    /* Use result */
    global_counter += _mm512_reduce_add_epi32(result);
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* Function to trigger 10-operand case using ARM NEON */
__attribute__((optimize("O3")))
void test_10_operand_arm(void) {
    /* Complex NEON operations with multiple vector registers */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Table lookup with multiple registers - can generate complex RTL */
    uint8x16_t result = vqtbl4q_u8(
        vcombine_u8x4(
            vcreate_u8x4(vcombine_u8(vget_low_u8(a), vget_high_u8(b))),
            vcreate_u8x4(vcombine_u8(vget_low_u8(c), vget_high_u8(d))),
            vcreate_u8x4(vcombine_u8(vget_low_u8(b), vget_high_u8(a))),
            vcreate_u8x4(vcombine_u8(vget_low_u8(d), vget_high_u8(c)))
        ),
        vdupq_n_u8(0)
    );
    
    /* Use result */
    global_counter += vaddvq_u8(result);
}

/* Function to trigger 11-operand case using ARM NEON */
__attribute__((optimize("O3")))
void test_11_operand_arm(void) {
    /* Multiple vector operations that might combine */
    float32x4_t a = vdupq_n_f32(1.0f);
    float32x4_t b = vdupq_n_f32(2.0f);
    float32x4_t c = vdupq_n_f32(3.0f);
    float32x4_t d = vdupq_n_f32(4.0f);
    
    /* Complex FMA chain */
    float32x4_t r1 = vfmaq_f32(a, b, c);
    float32x4_t r2 = vfmsq_f32(d, a, b);
    float32x4_t r3 = vmulq_f32(r1, r2);
    float32x4_t result = vmlaq_f32(r3, c, d);
    
    /* Use result */
    global_counter += vaddvq_f32(result);
}

#else
/* Generic fallback using inline assembly with many operands */

/* 10-operand inline assembly */
__attribute__((optimize("O3")))
void test_10_operand_generic(void) {
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10;
    long result;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10;
    
    /* Inline assembly with 10 input/output operands */
    asm volatile (
        "mov %[res], %[a1] \n\t"
        "add %[res], %[res], %[a2] \n\t"
        "add %[res], %[res], %[a3] \n\t"
        "add %[res], %[res], %[a4] \n\t"
        "add %[res], %[res], %[a5] \n\t"
        "add %[res], %[res], %[a6] \n\t"
        "add %[res], %[res], %[a7] \n\t"
        "add %[res], %[res], %[a8] \n\t"
        "add %[res], %[res], %[a9] \n\t"
        "add %[res], %[res], %[a10] \n\t"
        : [res] "=r" (result)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    global_counter += result;
}

/* 11-operand inline assembly */
__attribute__((optimize("O3")))
void test_11_operand_generic(void) {
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, op11;
    long result;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10; op11 = 11;
    
    /* Inline assembly with 11 input/output operands */
    asm volatile (
        "mov %[res], %[a1] \n\t"
        "add %[res], %[res], %[a2] \n\t"
        "add %[res], %[res], %[a3] \n\t"
        "add %[res], %[res], %[a4] \n\t"
        "add %[res], %[res], %[a5] \n\t"
        "add %[res], %[res], %[a6] \n\t"
        "add %[res], %[res], %[a7] \n\t"
        "add %[res], %[res], %[a8] \n\t"
        "add %[res], %[res], %[a9] \n\t"
        "add %[res], %[res], %[a10] \n\t"
        "add %[res], %[res], %[a11] \n\t"
        : [res] "=r" (result)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10), [a11] "r" (op11)
        : "cc"
    );
    
    global_counter += result;
}
#endif

/* OpenMP SIMD function that might generate complex RTL patterns */
__attribute__((optimize("O3")))
void test_omp_simd(void) {
    #define N 1024
    float a[N], b[N], c[N], d[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
        c[i] = i * 3.0f;
        d[i] = i * 4.0f;
    }
    
    /* Complex SIMD operation chain that might be combined */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        /* Multiple operations that could be combined into one RTL pattern */
        float t1 = a[i] * b[i] + c[i];
        float t2 = d[i] - a[i] * c[i];
        float t3 = t1 * t2 + b[i];
        float t4 = t3 / (c[i] + 1.0f);
        a[i] = t4 * d[i] - t2;
    }
    
    /* Use result */
    for (int i = 0; i < N; i++) {
        global_counter += (int)a[i];
    }
}

/* Main driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
#ifdef __x86_64__
    printf("Using x86_64 intrinsics\n");
    test_10_operand_x86();
    test_11_operand_x86();
#elif defined(__aarch64__)
    printf("Using AArch64 NEON intrinsics\n");
    test_10_operand_arm();
    test_11_operand_arm();
#else
    printf("Using generic inline assembly\n");
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    /* Also test OpenMP SIMD */
    test_omp_simd();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
