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
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Complex FMA chain that might generate multi-operand RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    volatile v8df sink = a;
    (void)sink;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9)
        : "cc"
    );
    
    volatile int64_t sink = o0;
    (void)sink;
}

static void test_inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "mov %[out], %[in1]\n\t"
        "add %[out], %[out], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9),
          [in10] "r" (i10)
        : "cc"
    );
    
    volatile int64_t sink = o0;
    (void)sink;
}

/* Strategy 3: Complex shuffle/permute operations */
#ifdef __AVX__
static void test_vector_shuffle(void) {
    v8sf a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf b = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Large shuffle mask - 8 elements each from 2 vectors = 16 possible indices */
    v8sf c = __builtin_shuffle(a, b, 
        (typeof(a)){0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15});
    
    volatile v8sf sink = c;
    (void)sink;
}
#endif

/* Strategy 4: Complex constant expression that might not fold immediately */
static int test_complex_const_expr(void) {
    /* Use __builtin_constant_p to potentially prevent folding */
    int x = 1;
    if (__builtin_constant_p(0)) {
        /* This creates a complex constant expression */
        x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        x = 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10;
    }
    
    /* Force use of the result */
    volatile int sink = x;
    return sink;
}

/* Strategy 5: Template-like approach using macros for C */
#define GEN_OPERATION(TYPE, N) \
    static TYPE operation_##TYPE##_##N(TYPE a, TYPE b) { \
        return a + b + N; \
    }

/* Generate multiple instantiations */
GEN_OPERATION(int, 1)
GEN_OPERATION(int, 2)
GEN_OPERATION(int, 3)
GEN_OPERATION(int, 4)
GEN_OPERATION(int, 5)
GEN_OPERATION(int, 6)
GEN_OPERATION(int, 7)
GEN_OPERATION(int, 8)
GEN_OPERATION(int, 9)
GEN_OPERATION(int, 10)
GEN_OPERATION(int, 11)

static void test_multiple_instantiations(void) {
    int results[11];
    for (int i = 0; i < 11; i++) {
        switch (i) {
            case 0: results[i] = operation_int_1(i, i+1); break;
            case 1: results[i] = operation_int_2(i, i+1); break;
            case 2: results[i] = operation_int_3(i, i+1); break;
            case 3: results[i] = operation_int_4(i, i+1); break;
            case 4: results[i] = operation_int_5(i, i+1); break;
            case 5: results[i] = operation_int_6(i, i+1); break;
            case 6: results[i] = operation_int_7(i, i+1); break;
            case 7: results[i] = operation_int_8(i, i+1); break;
            case 8: results[i] = operation_int_9(i, i+1); break;
            case 9: results[i] = operation_int_10(i, i+1); break;
            case 10: results[i] = operation_int_11(i, i+1); break;
        }
    }
    
    volatile int sink = results[0];
    (void)sink;
}

/* Main function that exercises all strategies */
int main(void) {
    /* Test inline assembly with exactly 10 and 11 operands */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test complex constant expressions */
    test_complex_const_expr();
    
    /* Test multiple instantiations */
    test_multiple_instantiations();
    
#ifdef __FMA__
    /* Test vector FMA chains if supported */
    test_vector_fma_chain();
#endif
    
#ifdef __AVX__
    /* Test vector shuffles if supported */
    test_vector_shuffle();
#endif
    
    return 0;
}
