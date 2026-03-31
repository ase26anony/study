/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdint.h>
#include <stdio.h>

/* Vector types for different architectures */
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
__attribute__((noinline))
static void test_vector_operations() {
    /* Use volatile to prevent optimization */
    volatile v4df a = {1.0, 2.0, 3.0, 4.0};
    volatile v4df b = {5.0, 6.0, 7.0, 8.0};
    volatile v4df c = {9.0, 10.0, 11.0, 12.0};
    volatile v4df d = {13.0, 14.0, 15.0, 16.0};
    volatile v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    v4df result = __builtin_fma(a, b, __builtin_fma(c, d, e));
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+x"(result));
}

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
__attribute__((noinline))
static void test_inline_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
    asm volatile(
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r"(o0)
        : [in1] "r"(i1), [in2] "r"(i2), [in3] "r"(i3),
          [in4] "r"(i4), [in5] "r"(i5), [in6] "r"(i6),
          [in7] "r"(i7), [in8] "r"(i8), [in9] "r"(i9)
        : "cc"
    );
    
    printf("10-operand asm result: %ld\n", (long)o0);
}

__attribute__((noinline))
static void test_inline_asm_11_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, 
            i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* Exactly 11 operands: 1 output + 10 inputs */
    asm volatile(
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r"(o0)
        : [in1] "r"(i1), [in2] "r"(i2), [in3] "r"(i3),
          [in4] "r"(i4), [in5] "r"(i5), [in6] "r"(i6),
          [in7] "r"(i7), [in8] "r"(i8), [in9] "r"(i9),
          [in10] "r"(i10)
        : "cc"
    );
    
    printf("11-operand asm result: %ld\n", (long)o0);
}

/* Strategy 3: Complex shuffle/permute operations */
__attribute__((noinline))
static void test_vector_shuffle() {
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with a complex mask - may generate RTL with many immediate operands */
    typedef int v4si __attribute__((vector_size(16)));
    v4si mask = {3, 2, 1, 0, 7, 6, 5, 4};  /* 8-element mask for concatenated vectors */
    
    /* Use __builtin_shuffle with many mask elements */
    v4sf result = __builtin_shuffle(a, b, mask);
    
    asm volatile("" : "+x"(result));
}

/* Strategy 4: Complex constant expressions */
__attribute__((noinline))
static int test_complex_const_expr() {
    /* Force compiler to generate RTL for complex constant computation */
    int x = 
        1 * 2 + 3 * 4 + 5 * 6 + 7 * 8 + 9 * 10 +
        11 * 12 + 13 * 14 + 15 * 16 + 17 * 18 + 19 * 20;
    
    /* Use __builtin_constant_p to prevent early folding */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x - 1;
    }
}

/* Strategy 5: Template/generic approach (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
__attribute__((noinline))
T template_operation(T a, T b) {
    /* Complex expression that might generate multi-operand RTL */
    return (a + b) * N + (a - b) * (N + 1) + (a * b) * (N + 2) + 
           (a / (b + 1)) * (N + 3) + (a % (b + 2)) * (N + 4);
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test vector operations */
    test_vector_operations();
    
    /* Test inline assembly with exactly 10 and 11 operands */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test vector shuffle */
    test_vector_shuffle();
    
    /* Test complex constant expressions */
    int const_result = test_complex_const_expr();
    printf("Complex constant result: %d\n", const_result);
    
#ifdef __cplusplus
    /* Test template operations if compiling as C++ */
    int template_result = template_operation<int, 10>(5, 3);
    printf("Template operation result: %d\n", template_result);
    
    double template_double = template_operation<double, 11>(5.5, 2.2);
    printf("Template double result: %f\n", template_double);
#endif
    
    return 0;
}
