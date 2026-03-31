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
__attribute__((noinline))
static void test_vector_operations() {
    /* Use volatile to prevent optimization */
    volatile v4df a = {1.0, 2.0, 3.0, 4.0};
    volatile v4df b = {5.0, 6.0, 7.0, 8.0};
    volatile v4df c = {9.0, 10.0, 11.0, 12.0};
    volatile v4df d = {13.0, 14.0, 15.0, 16.0};
    volatile v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Complex expression that might generate many operands */
    v4df result = __builtin_fma(a, b, __builtin_fma(c, d, e));
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+x"(result));
}

/* Strategy 2: Inline assembly with exactly 10 operands */
__attribute__((noinline))
static void test_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        "add %9, %0, %0"
        : "=r"(o0) 
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    /* Use the result */
    asm volatile("" : "+r"(o0));
}

/* Strategy 3: Inline assembly with exactly 11 operands */
__attribute__((noinline))
static void test_asm_11_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    /* Use the result */
    asm volatile("" : "+r"(o0));
}

/* Strategy 4: Complex constant expression */
__attribute__((noinline))
static int test_complex_const_expr() {
    /* Force compiler to generate RTL for complex constant computation */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    int y = x * 2 - 3 + 4 - 5 + 6 - 7 + 8 - 9 + 10 - 11 + 12;
    
    /* Use __builtin_constant_p to potentially generate more RTL */
    if (__builtin_constant_p(x)) {
        return y + 100;
    } else {
        return y + 200;
    }
}

/* Strategy 5: Vector shuffle with large mask */
__attribute__((noinline))
static void test_vector_shuffle() {
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with complex mask - might generate many immediate operands */
    int mask1[] = {0, 4, 1, 5, 2, 6, 3, 7};
    v4sf result;
    
    /* Multiple shuffle operations in one expression */
    result = __builtin_shuffle(v1, v2, 
              __builtin_shufflevector(v1, v2, 0, 4, 1, 5));
    
    /* Use result */
    asm volatile("" : "+x"(result));
}

/* Strategy 6: Target-specific builtins if available */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
static void test_avx512_gather() {
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __m512 src = _mm512_set1_ps(1.0f);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    /* AVX512 gather has many operands */
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    
    /* Use result */
    asm volatile("" : "+x"(result));
}
#endif

/* Strategy 7: Template/generic approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
__attribute__((noinline))
T template_operation(T a, T b) {
    /* Complex expression that might expand to many operands */
    return a + b + (T)N + (T)(N+1) + (T)(N+2) + (T)(N+3) + 
           (T)(N+4) + (T)(N+5) + (T)(N+6) + (T)(N+7) + (T)(N+8);
}

static void test_template_operations() {
    /* Instantiate with different types */
    int int_result = template_operation<int, 10>(1, 2);
    float float_result = template_operation<float, 20>(1.0f, 2.0f);
    double double_result = template_operation<double, 30>(1.0, 2.0);
    
    /* Use results */
    asm volatile("" : "+r"(int_result));
    asm volatile("" : "+x"(float_result));
    asm volatile("" : "+x"(double_result));
}
#endif

/* Main function that calls all test patterns */
int main() {
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Call all test functions */
    test_vector_operations();
    test_asm_10_operands();
    test_asm_11_operands();
    
    int const_result = test_complex_const_expr();
    printf("Constant expression result: %d\n", const_result);
    
    test_vector_shuffle();
    
    #ifdef __AVX512F__
    test_avx512_gather();
    #endif
    
    #ifdef __cplusplus
    test_template_operations();
    #endif
    
    /* Ensure all results are used */
    volatile int dummy = const_result;
    
    return 0;
}
