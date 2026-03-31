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
    volatile v8df result = a;
    (void)result;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10/11 operands */
void test_inline_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        : "=r"(o0) 
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t res = o0;
    (void)res;
}

void test_inline_asm_11_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0\n\t"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int64_t res = o0;
    (void)res;
}

/* Strategy 3: Complex constant expression that may not fold immediately */
int test_complex_const_expression() {
    /* Large constant expression - might generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to potentially prevent early folding */
    if (__builtin_constant_p(x)) {
        return x;
    } else {
        /* Force compiler to consider this path */
        volatile int y = x;
        return y;
    }
}

/* Strategy 4: Vector shuffle with large constant mask */
#ifdef __AVX__
void test_vector_shuffle() {
    v8sf a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf b = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Shuffle with 8-element mask - each element is an immediate operand */
    v8sf c = __builtin_shufflevector(a, b, 
        0, 2, 4, 6, 8, 10, 12, 14);
    
    volatile v8sf result = c;
    (void)result;
}
#endif

/* Strategy 5: C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex operation that might generate many operands */
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) +
           static_cast<T>(N+3) + static_cast<T>(N+4);
}

void test_template_instantiations() {
    /* Instantiate with different types and constants */
    int r1 = template_operation<int, 1>(10, 20);
    float r2 = template_operation<float, 2>(10.5f, 20.5f);
    double r3 = template_operation<double, 3>(10.5, 20.5);
    
    volatile int vr1 = r1;
    volatile float vr2 = r2;
    volatile double vr3 = r3;
    (void)vr1; (void)vr2; (void)vr3;
}
#endif

/* Strategy 6: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
#include <immintrin.h>
void test_avx512_gather() {
    /* AVX-512 gather can have many operands: base, scale, index, mask, etc. */
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    __m512 result = _mm512_mask_i32gather_ps(
        _mm512_setzero_ps(),  // src
        mask,                 // mask
        index,                // index
        base,                 // base
        4                     // scale
    );
    
    volatile __m512 vresult = result;
    (void)vresult;
}
#endif

/* Main function that calls all test patterns */
int main() {
    /* Call all test functions to ensure they're not optimized away */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX__
    test_vector_shuffle();
#endif
    
#ifdef __AVX512F__
    test_avx512_gather();
#endif
    
#ifdef __cplusplus
    test_template_instantiations();
#endif
    
    int const_result = test_complex_const_expression();
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = const_result;
    
    return final_result > 0 ? 0 : 1;
}
