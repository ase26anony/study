/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Define vector types for different architectures */
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
    volatile v4df f = {21.0, 22.0, 23.0, 24.0};
    
    /* Complex expression that might generate many operands */
    v4df result = a + b * c + d * e + f;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+x"(result));
}

/* Strategy 2: Inline assembly with exactly 10 operands */
__attribute__((noinline))
static void test_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile(
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
    
    printf("ASM 10 operands result: %ld\n", o0);
}

/* Strategy 3: Inline assembly with exactly 11 operands */
__attribute__((noinline))
static void test_asm_11_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
                   i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile(
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
    
    printf("ASM 11 operands result: %ld\n", o0);
}

/* Strategy 4: Complex constant expression */
__attribute__((noinline))
static int test_complex_const_expression() {
    /* Force compiler to consider all operands separately */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Complex expression that might not fold immediately */
    int result = a + b + c + d + e + f + g + h + i + j;
    
    /* Use __builtin_constant_p to potentially generate RTL for both paths */
    if (__builtin_constant_p(result)) {
        return result;
    } else {
        return a + b + c + d + e + f + g + h + i + j;
    }
}

/* Strategy 5: Vector shuffle with large mask */
__attribute__((noinline))
static void test_vector_shuffle() {
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with explicit indices - each index is an operand */
    typedef int v4si __attribute__((vector_size(16)));
    v4si mask = {3, 2, 1, 0};
    
    /* Use __builtin_shuffle which might generate many operands */
    v4sf result = __builtin_shuffle(v1, v2, mask);
    
    asm volatile("" : "+x"(result));
}

/* Strategy 6: Mixed constraints in inline assembly */
__attribute__((noinline))
static void test_mixed_constraints() {
    int out1, out2;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int mem1 = 6, mem2 = 7, mem3 = 8;
    
    /* Mix of register, memory, and immediate constraints */
    asm volatile(
        "imul %[out1], %[in1], %[imm1]\n\t"
        "add %[out1], %[out1], %[in2]\n\t"
        "add %[out2], %[in3], %[mem1]\n\t"
        "imul %[out2], %[out2], %[imm2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [mem1] "m" (mem1), [mem2] "m" (mem2), [mem3] "m" (mem3),
          [imm1] "i" (10), [imm2] "i" (20)
        : "cc"
    );
    
    printf("Mixed constraints: %d, %d\n", out1, out2);
}

/* C++ template version for Strategy 5 */
#ifdef __cplusplus
template<typename T, int N>
__attribute__((noinline))
T template_operation(T a, T b) {
    /* Complex operation that might generate many operands */
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) +
           static_cast<T>(N+3) + static_cast<T>(N+4);
}

static void test_template_instantiations() {
    /* Instantiate with different types and constants */
    int int_result = template_operation<int, 10>(1, 2);
    float float_result = template_operation<float, 20>(1.0f, 2.0f);
    double double_result = template_operation<double, 30>(1.0, 2.0);
    
    printf("Template results: %d, %f, %f\n", 
           int_result, float_result, double_result);
}
#endif

int main() {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Execute all test strategies */
    test_vector_operations();
    test_asm_10_operands();
    test_asm_11_operands();
    
    int const_result = test_complex_const_expression();
    printf("Constant expression result: %d\n", const_result);
    
    test_vector_shuffle();
    test_mixed_constraints();
    
    #ifdef __cplusplus
    test_template_instantiations();
    #endif
    
    return 0;
}
