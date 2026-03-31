/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

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
static void test_vector_fma(void) {
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
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t res = o0;
    (void)res;
}

static void test_inline_asm_11_operands(void) {
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
        "add %10, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int64_t res = o0;
    (void)res;
}

/* Strategy 3: Complex shuffle/permute with large masks */
#ifdef __AVX512F__
static void test_vector_shuffle(void) {
    v16sf v1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16sf v2 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with a complex 16-element mask - may generate many immediate operands */
    int mask[16] = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    v16sf result = __builtin_shuffle(v1, v2, mask);
    
    volatile v16sf res = result;
    (void)res;
}
#endif

/* Strategy 4: Complex constant expression that may not fold immediately */
static int test_complex_const_expr(void) {
    /* Use __builtin_constant_p to potentially prevent early folding */
    if (__builtin_constant_p(0)) {
        return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        /* This branch creates a complex expression with 11 constants */
        int x = 1;
        x += 2;
        x += 3;
        x += 4;
        x += 5;
        x += 6;
        x += 7;
        x += 8;
        x += 9;
        x += 10;
        x += 11;
        return x;
    }
}

/* Strategy 5: Template/generic approach for C++ */
#ifdef __cplusplus
template<int N>
struct ComplexOp {
    template<typename T>
    static T compute(T a, T b) {
        /* Create a complex expression that may expand to many operands */
        return a + b + N + (N+1) + (N+2) + (N+3) + (N+4) + 
               (N+5) + (N+6) + (N+7) + (N+8) + (N+9);
    }
};

void test_template_instantiations(void) {
    /* Instantiate with multiple values to increase coverage chances */
    volatile int r1 = ComplexOp<1>::compute(1, 2);
    volatile int r2 = ComplexOp<2>::compute(3, 4);
    volatile int r3 = ComplexOp<3>::compute(5, 6);
    volatile int r4 = ComplexOp<4>::compute(7, 8);
    (void)r1; (void)r2; (void)r3; (void)r4;
}
#endif

/* Main function that exercises all strategies */
int main(void) {
    /* Test inline assembly with exactly 10 and 11 operands */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test vector operations if supported */
#ifdef __FMA__
    test_vector_fma();
#endif
    
#ifdef __AVX512F__
    test_vector_shuffle();
#endif
    
    /* Test complex constant expression */
    volatile int const_result = test_complex_const_expr();
    (void)const_result;
    
#ifdef __cplusplus
    test_template_instantiations();
#endif
    
    return 0;
}
