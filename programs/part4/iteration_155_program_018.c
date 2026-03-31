/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

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
    
    /* Complex expression that might generate multi-operand RTL */
    v4df result = __builtin_fma(a, b, __builtin_fma(c, d, e));
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+x"(result));
}

/* Strategy 2: Inline assembly with exactly 10 operands */
__attribute__((noinline))
static void test_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
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
    
    /* Use result */
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
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        "add %9, %10, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    /* Use result */
    asm volatile("" : "+r"(o0));
}

/* Strategy 4: Complex shuffle with large constant mask */
__attribute__((noinline))
static void test_vector_shuffle() {
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with complex mask - might generate multi-operand RTL */
    int mask[8] = {0, 4, 1, 5, 2, 6, 3, 7};
    v4sf result = __builtin_shuffle(a, b, mask[0], mask[1], mask[2], mask[3],
                                           mask[4], mask[5], mask[6], mask[7]);
    
    /* Use result */
    asm volatile("" : "+x"(result));
}

/* Strategy 5: Target-specific builtins for x86 AVX-512 */
#if defined(__AVX512F__)
__attribute__((noinline))
static void test_avx512_gather() {
    /* Simulate a gather operation with many parameters */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    
    /* This builtin has many implicit operands */
    float* base = (float*)0x1000;
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    
    /* Use result */
    asm volatile("" : "+x"(result));
}
#endif

/* Strategy 6: Complex constant expression */
__attribute__((noinline))
static int test_complex_constant() {
    /* Force compiler to consider a complex constant expression */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to prevent early folding */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x - 1;
    }
}

/* Strategy 7: Template-like approach using macros */
#define GEN_TEST(N) \
    __attribute__((noinline)) \
    static int test_macro_##N(int a, int b) { \
        return a + b + N; \
    }

/* Generate multiple functions with different constant operands */
GEN_TEST(10)
GEN_TEST(11)
GEN_TEST(12)
GEN_TEST(13)
GEN_TEST(14)
GEN_TEST(15)

/* Main function that exercises all strategies */
int main() {
    printf("Testing multi-operand expansion coverage...\n");
    
    /* Test vector operations */
    test_vector_operations();
    
    /* Test inline assembly with 10 and 11 operands */
    test_asm_10_operands();
    test_asm_11_operands();
    
    /* Test vector shuffle */
    test_vector_shuffle();
    
    /* Test AVX-512 if available */
#if defined(__AVX512F__)
    test_avx512_gather();
#endif
    
    /* Test complex constant expression */
    volatile int result = test_complex_constant();
    
    /* Test macro-generated functions */
    result += test_macro_10(1, 2);
    result += test_macro_11(2, 3);
    result += test_macro_12(3, 4);
    result += test_macro_13(4, 5);
    result += test_macro_14(5, 6);
    result += test_macro_15(6, 7);
    
    /* Use result to prevent optimization */
    asm volatile("" : "+r"(result));
    
    printf("Result: %d\n", result);
    return 0;
}
