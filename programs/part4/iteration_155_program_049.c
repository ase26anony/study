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

/* Strategy 1: Complex vector operations with chained FMAs */
#ifdef __FMA__
__attribute__((noinline))
static void test_vector_fma_chain(void) {
#ifdef __AVX__
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    
    /* Use volatile to prevent optimization */
    volatile v4df result = a;
    (void)result;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10/11 operands */
__attribute__((noinline))
static void test_10_operand_asm(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
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
    
    volatile int64_t res = o0;
    (void)res;
}

__attribute__((noinline))
static void test_11_operand_asm(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10, i11 = 11;
    
    /* 11 operands: 1 output + 10 inputs */
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
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9),
          [in10] "r" (i10)
        : "cc"
    );
    
    volatile int64_t res = o0;
    (void)res;
}

/* Strategy 3: Complex shuffle/permute operations */
#ifdef __AVX__
__attribute__((noinline))
static void test_complex_shuffle(void) {
    v8sf a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf b = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex shuffle with many constant indices */
    v8sf c = __builtin_shuffle(a, b, 
        (typeof(a)){0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15});
    
    volatile v8sf result = c;
    (void)result;
}
#endif

/* Strategy 4: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
__attribute__((noinline))
static void test_avx512_gather(void) {
    v16sf src = {0};
    int indices[16] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60};
    float base[64];
    
    for (int i = 0; i < 64; i++) {
        base[i] = (float)i;
    }
    
    /* AVX512 gather - conceptually has many operands */
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    __mmask16 mask = 0xFFFF;
    v16sf result = _mm512_mask_i32gather_ps(src, mask, vindex, base, 4);
    
    volatile v16sf res = result;
    (void)res;
}
#endif

/* Strategy 5: Complex constant expressions */
__attribute__((noinline))
static int test_complex_const_expr(void) {
    /* Force compiler to handle complex constant expression */
    int x = 1 + (2 * 3) + (4 << 2) + (5 & 6) + (7 | 8) + (9 ^ 10) + 
             (11 - 12) + (13 / 14) + (15 % 16) + (17 == 18) + (19 != 20);
    
    /* Use __builtin_constant_p to force evaluation */
    if (__builtin_constant_p(x)) {
        return x + 100;
    } else {
        return x + 200;
    }
}

/* C++ template version for more instantiations */
#ifdef __cplusplus
template<typename T, int N>
__attribute__((noinline))
T template_operation(T a, T b) {
    /* Complex operation that might generate many operands */
    return a + b + (a * b) + (a / (b + 1)) + (a % (N + 1)) + 
           (a << (N & 3)) + (b >> (N & 3)) + (a & b) + (a | b) + (a ^ b);
}

static void test_template_instances(void) {
    /* Instantiate with different types and constants */
    int r1 = template_operation<int, 1>(10, 20);
    int r2 = template_operation<int, 2>(30, 40);
    int r3 = template_operation<int, 3>(50, 60);
    int r4 = template_operation<int, 4>(70, 80);
    int r5 = template_operation<int, 5>(90, 100);
    
    volatile int sum = r1 + r2 + r3 + r4 + r5;
    (void)sum;
}
#endif

int main(void) {
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Test all strategies */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
    test_10_operand_asm();
    test_11_operand_asm();
    
#ifdef __AVX__
    test_complex_shuffle();
#endif
    
#ifdef __AVX512F__
    test_avx512_gather();
#endif
    
    int const_result = test_complex_const_expr();
    volatile int v = const_result;
    (void)v;
    
#ifdef __cplusplus
    test_template_instances();
#endif
    
    printf("Test completed.\n");
    return 0;
}
