/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Vector operations with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* Strategy 2: Complex inline assembly with many operands */
void test_asm_10_operands(void) {
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9;
    int o0;
    
    /* Initialize to prevent undefined behavior */
    i0 = 0; i1 = 1; i2 = 2; i3 = 3; i4 = 4;
    i5 = 5; i6 = 6; i7 = 7; i8 = 8; i9 = 9;
    
    /* Inline assembly with exactly 10 operands */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "add %9, %10\n\t"
        "mov %%eax, %0"
        : "=r" (o0)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3),
          "r" (i4), "r" (i5), "r" (i6), "r" (i7),
          "r" (i8), "r" (i9)
        : "%eax", "memory"
    );
    
    volatile int sink = o0; /* Prevent dead code elimination */
}

void test_asm_11_operands(void) {
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int o0;
    
    /* Initialize */
    i0 = 0; i1 = 1; i2 = 2; i3 = 3; i4 = 4;
    i5 = 5; i6 = 6; i7 = 7; i8 = 8; i9 = 9;
    i10 = 10;
    
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
        "imul %1, %2\n\t"
        "imul %3, %4\n\t"
        "imul %5, %6\n\t"
        "imul %7, %8\n\t"
        "imul %9, %10\n\t"
        "add %11, %%eax\n\t"
        "mov %%eax, %0"
        : "=r" (o0)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3),
          "r" (i4), "r" (i5), "r" (i6), "r" (i7),
          "r" (i8), "r" (i9), "r" (i10)
        : "%eax", "memory"
    );
    
    volatile int sink = o0;
}

/* Strategy 3: Vector extensions for complex operations */
#ifdef __VECTOR_EXTENSIONS__
typedef float v8sf __attribute__((vector_size(32))); /* 8 floats = 256-bit */
typedef double v4df __attribute__((vector_size(32))); /* 4 doubles = 256-bit */

v4df test_vector_fma(v4df a, v4df b, v4df c, v4df d, v4df e) {
    /* Complex FMA chain that might generate many operands */
    v4df t1 = __builtin_fma(a, b, c);
    v4df t2 = __builtin_fma(d, e, t1);
    v4df t3 = __builtin_fma(t1, t2, a);
    v4df t4 = __builtin_fma(b, c, d);
    v4df t5 = __builtin_fma(e, t1, t2);
    
    /* Combine all - this complex expression might generate many operands */
    return __builtin_fma(t3, t4, __builtin_fma(t5, a, __builtin_fma(b, c, d)));
}

v8sf test_vector_shuffle(v8sf a, v8sf b) {
    /* Shuffle with large constant mask - might generate many immediate operands */
    const int mask[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    v8sf result = __builtin_shuffle(a, b, mask);
    
    /* Chain multiple shuffles */
    const int mask2[8] = {0, 2, 4, 6, 1, 3, 5, 7};
    result = __builtin_shuffle(result, a, mask2);
    
    const int mask3[8] = {3, 7, 2, 6, 1, 5, 0, 4};
    return __builtin_shuffle(result, b, mask3);
}
#endif

/* Strategy 4: Complex constant expressions */
int test_const_expr(void) {
    /* Large constant expression that might not fold immediately */
    int x = 1 + (2 * 3) + (4 / 2) + (5 << 1) + (6 >> 1) + 
            (7 & 0xF) + (8 | 0x1) + (9 ^ 0xF) + (10 % 3) + 
            (11 * 2) + (12 - 5);
    
    /* Use __builtin_constant_p to potentially prevent folding */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x - 1;
    }
}

/* Strategy 5: Template/Generic context (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex template operation that might instantiate differently */
    T result = a;
    for (int i = 0; i < N; i++) {
        result = result + b + T(i);
    }
    return result * T(N) + a - b;
}

/* Instantiate with multiple types */
void test_templates(void) {
    int int_result = template_operation<int, 10>(1, 2);
    float float_result = template_operation<float, 11>(1.0f, 2.0f);
    double double_result = template_operation<double, 12>(1.0, 2.0);
    
    volatile int sink1 = int_result;
    volatile float sink2 = float_result;
    volatile double sink3 = double_result;
}
#endif

/* Strategy 6: Target-specific builtins */
#ifdef __AVX512F__
__m512 test_avx512_gather(void) {
    /* AVX-512 gather with many parameters */
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    return _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, index, base, 4);
}
#endif

/* Main function that exercises all strategies */
int main(void) {
    printf("Testing 10/11 operand expansion coverage\n");
    
    /* Test inline assembly strategies */
    test_asm_10_operands();
    test_asm_11_operands();
    
    /* Test vector operations if supported */
#ifdef __VECTOR_EXTENSIONS__
    {
        v4df a = {1.0, 2.0, 3.0, 4.0};
        v4df b = {5.0, 6.0, 7.0, 8.0};
        v4df c = {9.0, 10.0, 11.0, 12.0};
        v4df d = {13.0, 14.0, 15.0, 16.0};
        v4df e = {17.0, 18.0, 19.0, 20.0};
        
        v4df vec_result = test_vector_fma(a, b, c, d, e);
        volatile v4df sink = vec_result;
        
        v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf v2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
        v8sf shuffle_result = test_vector_shuffle(v1, v2);
        volatile v8sf sink2 = shuffle_result;
    }
#endif
    
    /* Test constant expressions */
    int const_result = test_const_expr();
    volatile int const_sink = const_result;
    
    /* Test templates if C++ */
#ifdef __cplusplus
    test_templates();
#endif
    
    /* Test AVX-512 if available */
#ifdef __AVX512F__
    {
        __m512 avx_result = test_avx512_gather();
        volatile __m512 avx_sink = avx_result;
    }
#endif
    
    return 0;
}
