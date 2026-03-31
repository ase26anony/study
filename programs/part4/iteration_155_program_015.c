/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
void test_vector_operations() {
    volatile int result = 0;
    
#if defined(__FMA__) || defined(__AVX512F__)
    /* Use FMA operations that may generate multi-operand RTL */
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    result += (int)a[0];
#endif
    
    /* Use vector shuffle with large constant mask */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    int mask[] = {3, 2, 1, 0, 7, 6, 5, 4};
    
    /* Complex shuffle operation */
    v4sf shuffled = __builtin_shuffle(v1, v2, mask[0], mask[1], mask[2], mask[3]);
    result += (int)shuffled[0];
}

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
void test_inline_asm() {
    int i0 = 0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int o0, o1;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5), 
          "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    /* Exactly 11 operands: 1 output + 10 inputs */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(o1)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5), 
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int dummy = o0 + o1;
    (void)dummy;
}

/* Strategy 3: Complex constant expression */
int test_const_expression() {
    /* Large constant expression that might generate multi-operand RTL */
    int x = 1 + (2 * 3) + (4 << 5) + (6 & 7) + (8 | 9) + (10 ^ 11) + 
             (12 - 13) + (14 / 15) + (16 % 17) + (18 * 19) + (20 + 21);
    
    /* Force evaluation of both branches with __builtin_constant_p */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x - 1;
    }
}

/* Strategy 4: Target-specific builtins */
void test_target_builtins() {
#if defined(__AVX512F__)
    /* AVX-512 gather instruction with many parameters */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    float base[64] = {0};
    
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    volatile float dummy = _mm512_cvtss_f32(result);
    (void)dummy;
#endif
}

/* Strategy 5: Template/generic approach (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex operation that might generate multi-operand RTL */
    return a + b + (T)N + (T)(N*2) + (T)(N*3) + (T)(N*4) + 
           (T)(N*5) + (T)(N*6) + (T)(N*7) + (T)(N*8) + (T)(N*9);
}

void test_template() {
    int r1 = template_operation<int, 1>(10, 20);
    float r2 = template_operation<float, 2>(10.5f, 20.5f);
    double r3 = template_operation<double, 3>(10.5, 20.5);
    
    volatile int dummy = r1 + (int)r2 + (int)r3;
    (void)dummy;
}
#endif

/* Main function that exercises all strategies */
int main() {
    test_vector_operations();
    test_inline_asm();
    
    int const_result = test_const_expression();
    
    test_target_builtins();
    
#ifdef __cplusplus
    test_template();
#endif
    
    /* Ensure results are used */
    volatile int final_result = const_result;
    
    return final_result == 0 ? 0 : 1;
}
