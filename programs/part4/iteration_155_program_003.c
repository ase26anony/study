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
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    a = __builtin_fma(a, b, __builtin_fma(c, d, __builtin_fma(e, a, b)));
    
    volatile v8df sink = a;
    (void)sink;
#endif
}
#endif

/* Strategy 2: Inline assembly with many operands */
static void test_many_operand_asm(void) {
    int64_t o0, o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    int64_t i0 = 0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 10 operand asm */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
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
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4),
          "r"(i5), "r"(i6), "r"(i7), "r"(i8)
        : "cc"
    );
    
    /* 11 operand asm with mixed constraints */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
        "lea (%1,%2,1), %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r"(o1)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4),
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t sink = o0 + o1;
    (void)sink;
}

/* Strategy 3: Complex constant expression that may not fold immediately */
static int test_complex_const_expr(void) {
    /* Large constant expression - may generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to potentially prevent folding */
    if (__builtin_constant_p(x)) {
        x = x * 2;
    } else {
        x = x + 1;
    }
    
    /* Chain more operations */
    x = x + (1 << 0) + (1 << 1) + (1 << 2) + (1 << 3) + 
            (1 << 4) + (1 << 5) + (1 << 6) + (1 << 7) + 
            (1 << 8) + (1 << 9);
    
    return x;
}

/* Strategy 4: Vector shuffle with large mask */
#ifdef __AVX512F__
static void test_large_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Shuffle with 16-element mask - may generate many immediate operands */
    int mask[16] = {0, 16, 1, 17, 2, 18, 3, 19, 
                    4, 20, 5, 21, 6, 22, 7, 23};
    
    /* Use __builtin_shuffle with explicit indices */
    v16sf result = __builtin_shuffle(v1, v2, 
        (typeof(v1)){mask[0], mask[1], mask[2], mask[3],
                     mask[4], mask[5], mask[6], mask[7],
                     mask[8], mask[9], mask[10], mask[11],
                     mask[12], mask[13], mask[14], mask[15]});
    
    volatile v16sf sink = result;
    (void)sink;
}
#endif

/* Strategy 5: Template-like pattern using macros (C version) */
#define DECLARE_AND_COMPUTE(TYPE, SUFFIX) \
    TYPE a##SUFFIX = 1, b##SUFFIX = 2, c##SUFFIX = 3, d##SUFFIX = 4, \
         e##SUFFIX = 5, f##SUFFIX = 6, g##SUFFIX = 7, h##SUFFIX = 8, \
         i##SUFFIX = 9, j##SUFFIX = 10; \
    TYPE result##SUFFIX = a##SUFFIX + b##SUFFIX + c##SUFFIX + d##SUFFIX + \
                         e##SUFFIX + f##SUFFIX + g##SUFFIX + h##SUFFIX + \
                         i##SUFFIX + j##SUFFIX; \
    volatile TYPE sink##SUFFIX = result##SUFFIX; \
    (void)sink##SUFFIX

static void test_multi_type_computation(void) {
    /* Generate computations with different types */
    DECLARE_AND_COMPUTE(int, _int);
    DECLARE_AND_COMPUTE(long, _long);
    DECLARE_AND_COMPUTE(float, _float);
    DECLARE_AND_COMPUTE(double, _double);
    
    /* Complex expression mixing types */
    int x_int = 1, y_int = 2;
    float x_float = 3.0f, y_float = 4.0f;
    double x_double = 5.0, y_double = 6.0;
    
    /* Mixed operation that might generate complex RTL */
    double mixed = (double)(x_int + y_int) + 
                   (double)x_float + (double)y_float + 
                   x_double + y_double + 7.0 + 8.0 + 9.0 + 10.0 + 11.0;
    
    volatile double sink_mixed = mixed;
    (void)sink_mixed;
}

/* Strategy 6: Memory operations with many addressing components */
static void test_complex_addressing(void) {
    struct LargeStruct {
        int data[256];
    };
    
    struct LargeStruct ls[10];
    int indices[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int scales[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Complex addressing calculation - may generate RTL with many operands */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        /* Each array access involves base, index, scale, offset */
        total += ls[i].data[indices[i] * scales[i] + i];
    }
    
    volatile int sink_total = total;
    (void)sink_total;
}

int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Execute all test strategies */
    test_many_operand_asm();
    
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_large_shuffle();
#endif
    
    test_multi_type_computation();
    test_complex_addressing();
    
    int result = test_complex_const_expr();
    
    printf("Result: %d\n", result);
    return 0;
}
