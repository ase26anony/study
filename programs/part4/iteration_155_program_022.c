/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Vector operations with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* Strategy 2: Complex inline assembly with many operands */
static void test_inline_asm_10_operands(void) {
    int o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "add %9, %0"
        : "=r"(o0) 
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int dummy = o0; /* Prevent optimization */
}

static void test_inline_asm_11_operands(void) {
    int o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
        i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "imul %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "add %9, %10\n\t"
        "mov %0, %%eax"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "eax", "cc"
    );
    
    volatile int dummy = o0;
}

/* Strategy 3: Vector extensions with complex operations */
#ifdef __VECTOR_EXTENSIONS__
typedef float v8sf __attribute__((vector_size(32))); /* 8 floats = 256-bit */
typedef double v4df __attribute__((vector_size(32))); /* 4 doubles = 256-bit */

static v8sf test_vector_operations(v8sf a, v8sf b, v8sf c, v8sf d, v8sf e) {
    /* Complex expression that might generate many operands */
    v8sf result = a + b * c + d / e;
    result = result * a - b + c * d;
    
    /* Use shuffle with large constant mask (8 indices) */
    v8sf shuffled = __builtin_shuffle(a, b, 
        (v8sf){0, 2, 4, 6, 1, 3, 5, 7});
    
    return result + shuffled;
}
#endif

/* Strategy 4: Complex constant expressions */
static int test_complex_const_expr(void) {
    /* Force compiler to handle complex constant expression */
    int x = 1 + (2 * 3) + (4 / 2) + (5 << 1) + (6 >> 1) + 
             (7 & 3) + (8 | 1) + (9 ^ 2) + (10 % 3) + 11;
    
    /* Use __builtin_constant_p to force evaluation */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x + 2;
    }
}

/* Strategy 5: C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N1, int N2, int N3, int N4, int N5,
         int N6, int N7, int N8, int N9, int N10>
T template_operation(T a, T b) {
    return a * N1 + b * N2 + a * N3 + b * N4 + a * N5 +
           b * N6 + a * N7 + b * N8 + a * N9 + b * N10;
}

/* Instantiate with many parameters */
static void test_template_operations(void) {
    int r1 = template_operation<int, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(1, 2);
    float r2 = template_operation<float, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11>(1.0f, 2.0f);
    volatile int dummy1 = r1;
    volatile float dummy2 = r2;
}
#endif

/* Strategy 6: Target-specific builtins */
#ifdef __AVX512F__
static void test_avx512_operations(void) {
    /* AVX-512 gather operations have many parameters */
    __m512i index = _mm512_set1_epi32(1);
    __m512 src = _mm512_set1_ps(2.0f);
    __mmask16 mask = 0xFFFF;
    
    /* Simulate complex operation with many parameters */
    __m512 result = _mm512_mask_add_ps(src, mask, src, src);
    result = _mm512_mask_mul_ps(result, mask, result, result);
    
    volatile __m512 dummy = result;
}
#endif

/* Strategy 7: Nested function calls with many arguments */
static int nested_op(int a1, int a2, int a3, int a4, int a5,
                     int a6, int a7, int a8, int a9, int a10) {
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
}

static int complex_nested_call(void) {
    /* Function call with 10 arguments */
    return nested_op(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}

/* Main function that exercises all strategies */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test 1: Inline assembly with many operands */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test 2: Complex constant expressions */
    volatile int const_result = test_complex_const_expr();
    
    /* Test 3: Nested function calls */
    volatile int nested_result = complex_nested_call();
    
#ifdef __VECTOR_EXTENSIONS__
    /* Test 4: Vector operations */
    v8sf va = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vb = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf vc = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf vd = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    v8sf ve = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
    
    v8sf vresult = test_vector_operations(va, vb, vc, vd, ve);
    volatile float dummy_vec = vresult[0];
#endif

#ifdef __cplusplus
    /* Test 5: Template operations (C++ only) */
    test_template_operations();
#endif

#ifdef __AVX512F__
    /* Test 6: AVX-512 specific operations */
    test_avx512_operations();
#endif
    
    printf("Tests completed.\n");
    return 0;
}
