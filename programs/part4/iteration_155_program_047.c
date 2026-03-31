/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    volatile v8df result = a; /* Prevent optimization */
    (void)result;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
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

static void test_inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* Exactly 11 operands: 1 output + 10 inputs */
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
    
    volatile int64_t res = o0;
    (void)res;
}

/* Strategy 3: Complex shuffle/permute operations with large masks */
#ifdef __AVX512F__
static void test_vector_shuffle_many_operands(void) {
    v16sf v1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16sf v2 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with a complex mask - may generate many immediate operands */
    int mask[16] = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    v16sf result;
    
    /* Manually create shuffle-like operation using inline asm 
       to ensure many operands */
    asm volatile (
        "vmovaps %[res], %[v1]\n\t"
        "vblendps $0xAA, %[v2], %[res], %[res]"
        : [res] "=x" (result)
        : [v1] "x" (v1), [v2] "x" (v2)
        : 
    );
    
    volatile v16sf res = result;
    (void)res;
}
#endif

/* Strategy 4: Complex constant expressions */
static int test_complex_const_expression(void) {
    /* Force compiler to handle complex constant expression */
    int x = 
        (1 << 0) + (2 << 1) + (3 << 2) + (4 << 3) + (5 << 4) +
        (6 << 5) + (7 << 6) + (8 << 7) + (9 << 8) + (10 << 9) +
        (11 << 10);
    
    /* Use __builtin_constant_p to potentially generate RTL for both paths */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x - 1;
    }
}

/* Strategy 5: Template/generic approach (using macros for C) */
#define GENERATE_MANY_OPERAND_OP(TYPE, SUFFIX) \
static TYPE test_many_operand_##SUFFIX(TYPE a, TYPE b, TYPE c, TYPE d, TYPE e, \
                                       TYPE f, TYPE g, TYPE h, TYPE i, TYPE j) { \
    /* Complex expression with many operands */ \
    return ((a + b) * (c - d)) / ((e + f) * (g - h)) + (i * j) + \
           ((a * b) + (c * d) + (e * f) + (g * h) + (i * j)); \
}

GENERATE_MANY_OPERAND_OP(int, int)
GENERATE_MANY_OPERAND_OP(float, float)
GENERATE_MANY_OPERAND_OP(double, double)

/* Strategy 6: Memory operations with many addressing components */
static void test_memory_operand_chain(void) {
    struct LargeStruct {
        int a[10];
        double b[10];
        char c[100];
    } data[10];
    
    /* Complex memory access pattern that might generate many operands */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex addressing expression */
            sum += data[i].a[j] + data[(i + j) % 10].a[(i * j) % 10];
        }
    }
    
    volatile int result = sum;
    (void)result;
}

/* Strategy 7: Use target-specific builtins when available */
#ifdef __AVX512F__
#include <immintrin.h>
static void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    float base[100] = {0};
    __mmask16 mask = 0xFFFF;
    
    /* AVX512 gather has many operands: source, mask, index, scale, base */
    __m512 result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, 
                                            index, base, 4);
    volatile __m512 res = result;
    (void)res;
}
#endif

/* Main function that exercises all strategies */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test inline assembly with exact operand counts */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test vector operations if supported */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_vector_shuffle_many_operands();
    test_avx512_gather();
#endif
    
    /* Test complex expressions */
    int const_result = test_complex_const_expression();
    volatile int v1 = const_result;
    (void)v1;
    
    /* Test generic many-operand functions */
    int int_result = test_many_operand_int(1,2,3,4,5,6,7,8,9,10);
    float float_result = test_many_operand_float(1.0f,2.0f,3.0f,4.0f,5.0f,
                                                 6.0f,7.0f,8.0f,9.0f,10.0f);
    double double_result = test_many_operand_double(1.0,2.0,3.0,4.0,5.0,
                                                    6.0,7.0,8.0,9.0,10.0);
    
    volatile int v2 = int_result;
    volatile float v3 = float_result;
    volatile double v4 = double_result;
    (void)v2; (void)v3; (void)v4;
    
    /* Test memory operand chains */
    test_memory_operand_chain();
    
    printf("Test completed.\n");
    return 0;
}
