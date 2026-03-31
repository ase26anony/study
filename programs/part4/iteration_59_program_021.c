/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use target-specific vector intrinsics with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Strategy 2: GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage aggressive optimization */
__attribute__((hot, noinline))
void test_vector_intrinsics() {
    /* This function uses various multi-operand patterns */
    
#ifdef __AVX512F__
    /* AVX-512 masked operations often require many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __mmask16 mask = 0xAAAA;
    
    /* Fused multiply-add with mask - can expand to many operands */
    __m512 result1 = _mm512_mask_fmadd_ps(a, mask, b, c);
    
    /* Complex blend operation */
    __m512 d = _mm512_set1_ps(4.0f);
    __m512 result2 = _mm512_mask_blend_ps(mask, result1, d);
    
    /* Store to prevent elimination */
    volatile __m512 sink = result2;
#endif

#ifdef __ARM_NEON
    /* ARM NEON operations with lane selection */
    float32x4_t v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    float32x4_t v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    
    /* FMA operations */
    float32x4_t r1 = vfmaq_laneq_f32(v1, v2, v3, 1);
    float32x4_t r2 = vfmaq_laneq_f32(r1, v2, v3, 2);
    
    volatile float32x4_t sink = r2;
#endif
}

/* Strategy 3: Complex reduction with GCC vector extensions */
__attribute__((hot, noinline))
v8sf test_vector_reduction(v8sf* arr, int n) {
    v8sf acc = {0};
    
    #pragma omp simd reduction(+:acc)
    for (int i = 0; i < n; i++) {
        /* Complex expression that might generate multi-operand patterns */
        v8sf temp = arr[i] * arr[i] + arr[i] * 2.0f + 1.0f;
        acc = acc + temp;
    }
    
    return acc;
}

/* Strategy 4: Built-in functions for complex math */
__attribute__((hot, noinline))
double test_builtin_fma(double a, double b, double c, double d) {
    /* Nested FMA calls - each expands to a 3-operand instruction,
       but combined expressions might create larger patterns */
    double r1 = __builtin_fma(a, b, c);
    double r2 = __builtin_fma(r1, c, d);
    double r3 = __builtin_fma(a, d, r2);
    double r4 = __builtin_fma(b, c, r3);
    
    return __builtin_fma(r4, a, b);
}

/* Strategy 5: Inline assembly with many operands */
__attribute__((hot, noinline))
void test_multi_operand_asm() {
    /* Create 11 distinct variables to use as operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long op11 = 11;
    long result1, result2, result3;
    
    /* Inline assembly with 11 operands (3 outputs, 8 inputs) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %2, %8\n\t"
        "imul %0, %9\n\t"
        "imul %1, %10\n\t"
        "imul %2, %11"
        : "=&r"(result1), "=&r"(result2), "=&r"(result3)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), 
          "r"(op5), "r"(op6), "r"(op7), "r"(op8), "r"(op9)
        : "cc"
    );
    
    /* Use results to prevent elimination */
    volatile long sink = result1 + result2 + result3;
}

/* Main function that exercises all strategies */
int main() {
    /* Initialize data for vector operations */
    v8sf vec_array[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            vec_array[i][j] = (float)(i * 8 + j) * 0.1f;
        }
    }
    
    /* Execute all test patterns */
    test_vector_intrinsics();
    
    v8sf reduction_result = test_vector_reduction(vec_array, 4);
    volatile float sink1 = reduction_result[0];
    
    double fma_result = test_builtin_fma(1.1, 2.2, 3.3, 4.4);
    volatile double sink2 = fma_result;
    
    test_multi_operand_asm();
    
    printf("Test completed - check coverage of optabs.cc lines 8254-8263\n");
    return 0;
}
