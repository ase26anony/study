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

/* Approach 1: Complex vector operations with FMA chaining */
#ifdef __FMA__
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    volatile v8df result = a;
    (void)result;
#endif
}
#endif

/* Approach 2: Inline assembly with many operands */
static void test_inline_asm_many_operands(void) {
    int64_t o0, o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    int64_t i0 = 0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operand inline assembly */
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
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    /* 10 operand inline assembly with mixed constraints */
    asm volatile (
        "/* dummy 10-operand asm with mixed constraints */\n\t"
        "lea (%1,%2,1), %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r"(o1)
        : "r"(i1), "r"(i2), "i"(3), "r"(i4), "m"(i5),
          "r"(i6), "i"(7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t dummy = o0 + o1;
    (void)dummy;
}

/* Approach 3: Complex constant expression that may not fold immediately */
static int test_complex_const_expression(void) {
    /* Large constant expression - may generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to potentially prevent folding */
    if (__builtin_constant_p(x)) {
        return x + 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20;
    } else {
        return x + 21 + 22 + 23 + 24 + 25 + 26 + 27 + 28 + 29 + 30 + 31;
    }
}

/* Approach 4: Vector permutation with large mask */
#ifdef __AVX512F__
static void test_vector_permutation(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Complex shuffle with many immediate indices */
    int mask[16] = {0, 16, 1, 17, 2, 18, 3, 19, 
                    4, 20, 5, 21, 6, 22, 7, 23};
    
    /* This may generate RTL with many immediate operands */
    v16sf result = __builtin_shufflevector(v1, v2, 
        mask[0], mask[1], mask[2], mask[3], mask[4], mask[5], 
        mask[6], mask[7], mask[8], mask[9], mask[10], mask[11],
        mask[12], mask[13], mask[14], mask[15]);
    
    volatile v16sf res = result;
    (void)res;
}
#endif

/* Approach 5: C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N1, int N2, int N3, int N4, int N5,
         int N6, int N7, int N8, int N9, int N10>
T template_many_params(T a, T b) {
    /* Complex expression with many template parameters */
    return a + b + N1 + N2 + N3 + N4 + N5 + N6 + N7 + N8 + N9 + N10;
}

static void test_template_instantiations(void) {
    /* Instantiate with many different types and values */
    int r1 = template_many_params<int, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(1, 2);
    float r2 = template_many_params<float, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20>(1.0f, 2.0f);
    double r3 = template_many_params<double, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30>(1.0, 2.0);
    
    volatile int dummy = r1 + (int)r2 + (int)r3;
    (void)dummy;
}
#endif

/* Main function that exercises all approaches */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test 1: Inline assembly with many operands */
    test_inline_asm_many_operands();
    
    /* Test 2: Complex constant expressions */
    int const_result = test_complex_const_expression();
    volatile int v1 = const_result;
    (void)v1;
    
#ifdef __FMA__
    /* Test 3: Vector FMA chaining */
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    /* Test 4: Vector permutation with large mask */
    test_vector_permutation();
#endif
    
#ifdef __cplusplus
    /* Test 5: Template instantiations (C++ only) */
    test_template_instantiations();
#endif
    
    /* Additional: Direct 11-operand expression */
    {
        int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5, a6 = 6, 
            a7 = 7, a8 = 8, a9 = 9, a10 = 10, a11 = 11;
        
        /* Complex expression that uses all 11 variables */
        int complex_result = ((((((((((a1 + a2) * a3) - a4) / a5) + a6) 
                                * a7) - a8) / a9) + a10) * a11);
        
        /* Prevent optimization */
        asm volatile ("" : "+r"(complex_result));
        
        volatile int v2 = complex_result;
        (void)v2;
    }
    
    printf("Test completed.\n");
    return 0;
}
