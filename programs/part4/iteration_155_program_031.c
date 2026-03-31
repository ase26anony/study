/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for various architectures */
#if defined(__AVX512F__)
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#elif defined(__AVX__)
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#else
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Strategy 1: Complex vector operations with FMA chaining */
void test_vector_operations() {
    volatile int result = 0;
    
#if defined(__FMA__)
    /* Use FMA operations which can generate multi-operand RTL */
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    result += (int)a[0];
#endif
    
    /* Use shuffle operations with large masks */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    int mask = 0x1B3A;  /* Complex shuffle mask */
    v4sf shuffled = __builtin_shuffle(v1, v2, mask);
    result += (int)shuffled[0];
}

/* Strategy 2: Inline assembly with many operands */
void test_inline_asm_many_operands() {
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int o0, o1;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "add %1, %2, %0\n\t"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    /* 11 operands: 2 outputs + 9 inputs */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %4, %10\n\t"
        : "=r"(o0), "=r"(o1)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
}

/* Strategy 3: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
#include <immintrin.h>
void test_avx512_multi_operand() {
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xAAAA;
    float base[64] = {0};
    float scale = 4.0f;
    
    /* AVX512 gather instruction with multiple parameters */
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, scale);
    volatile float v = result[0];
}
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
void test_neon_multi_operand() {
    /* NEON load multiple structures can use many operands */
    float32x4x4_t result;
    float32_t* ptr = (float32_t*)malloc(64);
    
    /* Load 4 registers from memory - may generate multi-operand RTL */
    result = vld4q_f32(ptr);
    free(ptr);
}
#endif

/* Strategy 4: Complex constant expressions */
int test_complex_const_expression() {
    /* Force compiler to handle complex constant expression */
    int x = 1 + 2 * 3 - 4 / 5 + 6 % 7 << 8 >> 9 & 10 | 11 ^ 12;
    
    /* Use __builtin_constant_p to prevent early folding */
    if (__builtin_constant_p(x)) {
        return x + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20;
    } else {
        return x - 21 - 22 - 23 - 24 - 25 - 26 - 27 - 28;
    }
}

/* Strategy 5: Template/Generic context (C++ version available) */
/* For C, we use macros to simulate template-like behavior */
#define GENERATE_OPERATION(TYPE, N) \
    TYPE operation_##TYPE##_##N(TYPE a, TYPE b) { \
        return a + b + (TYPE)N + (TYPE)(N*2) + (TYPE)(N*3) + \
               (TYPE)(N*4) + (TYPE)(N*5) + (TYPE)(N*6) + \
               (TYPE)(N*7) + (TYPE)(N*8) + (TYPE)(N*9); \
    }

/* Generate multiple instantiations */
GENERATE_OPERATION(int, 1)
GENERATE_OPERATION(int, 2)
GENERATE_OPERATION(int, 3)
GENERATE_OPERATION(float, 1)
GENERATE_OPERATION(float, 2)
GENERATE_OPERATION(float, 3)

/* Main function that exercises all strategies */
int main() {
    volatile int result = 0;
    
    /* Test vector operations */
    test_vector_operations();
    
    /* Test inline assembly with many operands */
    test_inline_asm_many_operands();
    
    /* Test target-specific builtins if available */
#ifdef __AVX512F__
    test_avx512_multi_operand();
#endif
    
#ifdef __ARM_NEON
    test_neon_multi_operand();
#endif
    
    /* Test complex constant expressions */
    result += test_complex_const_expression();
    
    /* Test generated operations */
    result += operation_int_1(1, 2);
    result += operation_int_2(3, 4);
    result += operation_int_3(5, 6);
    result += (int)operation_float_1(1.0f, 2.0f);
    result += (int)operation_float_2(3.0f, 4.0f);
    result += (int)operation_float_3(5.0f, 6.0f);
    
    /* Additional complex expression that might generate 10+ operand RTL */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int complex_result = a + b - c * d / e % f | g & h ^ i << j >> k;
    result += complex_result;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
