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
    /* Use FMA operations that can generate multi-operand RTL */
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    result += (int)a[0];
#endif
    
    /* Use shuffle operations with large constant masks */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with immediate mask - generates immediate operands */
    v4sf shuffled = __builtin_shuffle(v1, v2, 
        (typeof(v1)){0, 4, 1, 5, 2, 6, 3, 7});
    result += (int)shuffled[0];
    
    (void)result;
}

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
void test_inline_asm() {
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int o1 = 0, o2 = 0;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r" (o1)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9)
        : "cc"
    );
    
    /* Exactly 11 operands: 1 output + 10 inputs */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (o2)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9),
          [in10] "r" (i10)
        : "cc"
    );
    
    printf("ASM results: %d, %d\n", o1, o2);
}

/* Strategy 3: Target-specific builtins for multi-operand instructions */
void test_target_builtins() {
#if defined(__AVX512F__)
    /* AVX-512 gather instructions have many operands */
    __m512i index = _mm512_set1_epi32(0);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    void* base = NULL;
    
    /* _mm512_mask_i32gather_ps has: src, mask, index, base, scale */
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    (void)result;
#endif
    
#if defined(__ARM_NEON) || defined(__aarch64__)
    /* ARM NEON structured loads can have many operands */
    float32x4x4_t data;
    const float* ptr = NULL;
    
    /* vld4q_f32 loads 4 registers - potentially many operands in RTL */
    data = vld4q_f32(ptr);
    (void)data;
#endif
}

/* Strategy 4: Complex constant expressions */
int test_const_expressions() {
    /* Force compiler to handle complex constant expression */
    int x = 0;
    
    /* Large constant expression that might not fold immediately */
    if (__builtin_constant_p(1)) {
        x = 1 + 2 * 3 - 4 / 5 + 6 % 7 + 8 << 9 >> 10 | 11 & 12 ^ 13;
    } else {
        /* This branch forces RTL generation for the constant expression */
        x = (1 << 0) + (2 << 1) + (3 << 2) + (4 << 3) + (5 << 4) +
            (6 << 5) + (7 << 6) + (8 << 7) + (9 << 8) + (10 << 9);
    }
    
    /* Another complex expression with exactly 11 terms */
    int y = (1 * 2) + (3 * 4) + (5 * 6) + (7 * 8) + (9 * 10) + 
            (11 * 12) + (13 * 14) + (15 * 16) + (17 * 18) + 
            (19 * 20) + (21 * 22);
    
    return x + y;
}

/* Strategy 5: C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that varies with template parameters */
    return a + b + static_cast<T>(N) + 
           (a * b) - (a / static_cast<T>(N + 1)) +
           (b % static_cast<T>(N + 2)) + (a << (N % 8)) +
           (b >> ((N + 1) % 8)) + (a & static_cast<T>(N * 2)) +
           (b | static_cast<T>(N * 3)) + (a ^ static_cast<T>(N * 4));
}

void test_templates() {
    /* Instantiate with different types and constants */
    int r1 = template_operation<int, 1>(10, 20);
    int r2 = template_operation<int, 2>(30, 40);
    int r3 = template_operation<int, 3>(50, 60);
    int r4 = template_operation<int, 4>(70, 80);
    int r5 = template_operation<int, 5>(90, 100);
    
    float f1 = template_operation<float, 1>(1.5f, 2.5f);
    float f2 = template_operation<float, 2>(3.5f, 4.5f);
    
    printf("Template results: %d, %d, %d, %d, %d, %f, %f\n", 
           r1, r2, r3, r4, r5, f1, f2);
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    test_vector_operations();
    test_inline_asm();
    test_target_builtins();
    
    int const_result = test_const_expressions();
    printf("Constant expression result: %d\n", const_result);
    
#ifdef __cplusplus
    test_templates();
#endif
    
    /* Ensure everything is used */
    volatile int dummy = const_result;
    
    return 0;
}
