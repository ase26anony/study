/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Enable architecture-specific intrinsics */
#ifdef __x86_64__
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Always inline to force expansion */
__attribute__((always_inline, hot))
static inline void complex_vector_operations(void) {
#ifdef __AVX512F__
    /* Strategy 1: AVX-512 mask operations with many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __m512 d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;
    
    /* These intrinsics often expand to patterns with many operands */
    __m512 r1 = _mm512_mask_add_ps(a, mask, b, c);
    __m512 r2 = _mm512_mask_fmadd_ps(r1, mask, c, d);
    __m512 r3 = _mm512_mask_sub_ps(r2, mask, d, a);
    
    /* Complex blend with multiple sources */
    __m512 blend = _mm512_mask_blend_ps(mask, r1, r2);
    
    /* Force use of results */
    volatile __m512 sink = blend;
    (void)sink;
#endif

#ifdef __AVX__
    /* Strategy 2: GCC vector extensions with complex expressions */
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    v8sf v3 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf v4 = {9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f};
    
    /* Complex expression that may require multi-operand expansion */
    v8sf result = v1 * v2 + v3 * v4 - v1 * v3 + v2 * v4;
    
    /* Nested FMA-like operations using builtins */
    for (int i = 0; i < 8; i++) {
        result[i] = __builtin_fmaf(result[i], v1[i], v2[i]);
    }
    
    volatile v8sf sink2 = result;
    (void)sink2;
#endif

#ifdef __aarch64__
    /* Strategy 3: ARM NEON multi-vector operations */
    int8x16_t a1 = vdupq_n_s8(1);
    int8x16_t a2 = vdupq_n_s8(2);
    int8x16_t a3 = vdupq_n_s8(3);
    int8x16_t a4 = vdupq_n_s8(4);
    
    /* Complex permute/table lookup operations */
    int8x16x4_t quad = {a1, a2, a3, a4};
    
    /* Table lookup with multiple registers can expand to many operands */
    int8x16_t tbl_result = vqtbl4q_s8(quad, a1);
    
    /* Multiple vector arithmetic */
    int8x16_t arith = vaddq_s8(vaddq_s8(a1, a2), vaddq_s8(a3, a4));
    arith = vmulq_s8(arith, tbl_result);
    
    volatile int8x16_t sink3 = arith;
    (void)sink3;
#endif

    /* Strategy 4: Inline assembly with many operands */
    /* This directly tests the operand handling path */
    int op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    int op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    int result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    
    asm volatile (
        /* 11 operands total: 4 outputs, 5 inputs, 2 clobbers */
        "mov %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "mov %1, %7\n\t"
        "sub %1, %1, %8\n\t"
        "mov %2, %9\n\t"
        "mov %3, %10\n\t"
        : "=r"(result1), "=r"(result2), "=r"(result3), "=r"(result4)
        : "0"(result1), "r"(op1), "r"(op2), "r"(op3), "r"(op4), 
          "r"(op5), "r"(op6)
        : "cc", "memory"
    );
    
    volatile int sink4 = result1 + result2 + result3 + result4;
    (void)sink4;
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
__attribute__((noinline, hot))
static float complex_reduction(float* array, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex expression that may expand to multi-operand patterns */
        sum += array[i] * array[i] + 
               array[i] * 2.0f - 
               array[i] / 3.0f +
               __builtin_fmaf(array[i], 4.0f, 5.0f);
    }
    
    return sum;
}

/* Main function to execute all strategies */
int main(void) {
    /* Initialize data */
    float data[1024];
    for (int i = 0; i < 1024; i++) {
        data[i] = (i % 100) * 0.01f;
    }
    
    /* Execute complex vector operations */
    complex_vector_operations();
    
    /* Execute reduction */
    float result = complex_reduction(data, 1024);
    
    /* Use result to prevent optimization */
    printf("Result: %f\n", result);
    
    /* Additional builtin usage for complex math */
    double complex_result = 0.0;
    for (int i = 0; i < 100; i++) {
        /* Nested FMA operations */
        complex_result = __builtin_fma(complex_result, 1.1, 2.2);
        complex_result = __builtin_fma(complex_result, 3.3, 4.4);
    }
    
    printf("Complex result: %f\n", complex_result);
    
    return 0;
}
