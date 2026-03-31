/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector types for different architectures */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

#ifdef __AVX__
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#endif

/* Strategy 1: Complex vector operations with FMA chaining */
#ifdef __FMA__
void test_vector_fma_chain() {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    volatile v8df result = a; /* Prevent optimization */
    (void)result;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10/11 operands */
void test_inline_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
    asm volatile (
        "# 10-operand dummy instruction\n\t"
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        : "=r"(o0) 
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t res = o0;
    (void)res;
}

void test_inline_asm_11_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* Exactly 11 operands: 1 output + 10 inputs */
    asm volatile (
        "# 11-operand dummy instruction\n\t"
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int64_t res = o0;
    (void)res;
}

/* Strategy 3: Complex constant expression with many terms */
int test_complex_const_expression() {
    /* Force compiler to handle many constants in one expression */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to potentially generate RTL for both paths */
    if (__builtin_constant_p(x)) {
        return x + 100;
    } else {
        return x + 200;
    }
}

/* Strategy 4: Vector shuffle with large constant mask */
#ifdef __AVX512F__
void test_vector_shuffle_large_mask() {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Large shuffle mask - 16 elements */
    v16sf result = __builtin_shuffle(v1, v2, 
        (v16si){0, 16, 1, 17, 2, 18, 3, 19, 
                4, 20, 5, 21, 6, 22, 7, 23});
    
    volatile v16sf res = result;
    (void)res;
}
#endif

/* Strategy 5: C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that might generate many operands */
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) +
           static_cast<T>(N+3) + static_cast<T>(N+4) +
           static_cast<T>(N+5) + static_cast<T>(N+6) +
           static_cast<T>(N+7);
}

void test_cpp_templates() {
    int int_result = template_operation<int, 10>(1, 2);
    float float_result = template_operation<float, 20>(1.5f, 2.5f);
    double double_result = template_operation<double, 30>(1.5, 2.5);
    
    volatile int vi = int_result;
    volatile float vf = float_result;
    volatile double vd = double_result;
    (void)vi; (void)vf; (void)vd;
}
#endif

/* Strategy 6: Memory operations with many addressing components */
void test_complex_memory_addressing() {
    struct LargeStruct {
        int data[256];
    };
    
    struct LargeStruct ls = {0};
    int indices[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Complex addressing with multiple components */
    for (int i = 0; i < 10; i++) {
        ls.data[indices[i] * 2 + 1] = i * 3 + 5;
    }
    
    volatile int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += ls.data[i];
    }
    (void)sum;
}

/* Main function that exercises all strategies */
int main() {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test inline assembly with exact operand counts */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test complex constant expressions */
    int const_result = test_complex_const_expression();
    printf("Constant expression result: %d\n", const_result);
    
    /* Test vector operations if supported */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_vector_shuffle_large_mask();
#endif
    
    /* Test complex memory addressing */
    test_complex_memory_addressing();
    
#ifdef __cplusplus
    /* Test C++ templates */
    test_cpp_templates();
#endif
    
    return 0;
}
