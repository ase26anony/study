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

#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Strategy 1: Complex vector operations with FMA chaining */
#ifdef __FMA__
void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    
    volatile v8df* ptr = &a;
    (void)ptr; /* Prevent optimization */
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
void test_inline_asm_10_operands(void) {
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
    
    volatile int64_t* ptr = &o0;
    (void)ptr;
}

void test_inline_asm_11_operands(void) {
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
    
    volatile int64_t* ptr = &o0;
    (void)ptr;
}

/* Strategy 3: Complex shuffle/permute operations */
#ifdef __AVX512F__
void test_complex_shuffle(void) {
    v16sf a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16sf b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle with many constant indices */
    v16sf c = __builtin_shuffle(a, b, 
        (v16si){31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16});
    
    volatile v16sf* ptr = &c;
    (void)ptr;
}
#endif

/* Strategy 4: Complex constant expressions */
int test_complex_const_expr(void) {
    /* Force compiler to generate RTL for complex constant computation */
    int result = 
        __builtin_constant_p(1) ? 
        (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11) :
        (1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * 11);
    
    return result;
}

/* Strategy 5: Template/generic approach (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
T template_accumulate(T a) {
    return a + N + (N-1) + (N-2) + (N-3) + (N-4) + 
           (N-5) + (N-6) + (N-7) + (N-8) + (N-9);
}

void test_template_instances(void) {
    int i = template_accumulate<int, 10>(0);
    float f = template_accumulate<float, 11>(0.0f);
    double d = template_accumulate<double, 12>(0.0);
    
    volatile int* pi = &i;
    volatile float* pf = &f;
    volatile double* pd = &d;
    (void)pi; (void)pf; (void)pd;
}
#else
/* C version using macros */
#define ACCUMULATE(type, val, n) \
    ((val) + (n) + ((n)-1) + ((n)-2) + ((n)-3) + ((n)-4) + \
     ((n)-5) + ((n)-6) + ((n)-7) + ((n)-8) + ((n)-9))

void test_macro_instances(void) {
    int i = ACCUMULATE(int, 0, 10);
    float f = ACCUMULATE(float, 0.0f, 11);
    double d = ACCUMULATE(double, 0.0, 12);
    
    volatile int* pi = &i;
    volatile float* pf = &f;
    volatile double* pd = &d;
    (void)pi; (void)pf; (void)pd;
}
#endif

/* Strategy 6: Target-specific builtins */
#ifdef __AVX512F__
#include <immintrin.h>
void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    float* base = (float*)malloc(64 * sizeof(float));
    
    for (int i = 0; i < 64; i++) base[i] = (float)i;
    
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    
    volatile __m512* ptr = &result;
    (void)ptr;
    free(base);
}
#endif

/* Main function that calls all test patterns */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test inline assembly patterns */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test complex constant expression */
    int const_result = test_complex_const_expr();
    volatile int* pcr = &const_result;
    (void)pcr;
    
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_complex_shuffle();
    test_avx512_gather();
#endif
    
#ifdef __cplusplus
    test_template_instances();
#else
    test_macro_instances();
#endif
    
    printf("Test completed.\n");
    return 0;
}
