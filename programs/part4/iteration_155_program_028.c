/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for different architectures */
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
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    a = __builtin_fma(a, b, __builtin_fma(c, d, __builtin_fma(e, a, b)));
    
    /* Use result to prevent optimization */
    volatile v8df result = a;
    (void)result;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out1, out2;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* 10-operand dummy instruction */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r" (out1)
        : "r" (in1), "r" (in2), "r" (in3), "r" (in4),
          "r" (in5), "r" (in6), "r" (in7), "r" (in8), "r" (in9)
        : "cc"
    );
    
    /* 11 operands: 2 outputs + 9 inputs */
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %2, %0\n\t"
        "mov %3, %1\n\t"
        "add %4, %0\n\t"
        "add %5, %1\n\t"
        "add %6, %0\n\t"
        "add %7, %1\n\t"
        "add %8, %0\n\t"
        "add %9, %1\n\t"
        "add %10, %0"
        : "=r" (out1), "=r" (out2)
        : "r" (in1), "r" (in2), "r" (in3), "r" (in4), "r" (in5),
          "r" (in6), "r" (in7), "r" (in8), "r" (in9)
        : "cc"
    );
    
    /* Use results */
    volatile int64_t res1 = out1 + out2;
    (void)res1;
}

/* Strategy 3: Complex constant expression with many terms */
static int test_complex_const_expression(void) {
    /* Force compiler to generate RTL for complex constant computation */
    int x = 
        (1 << 0) + (2 << 1) + (3 << 2) + (4 << 3) + (5 << 4) +
        (6 << 5) + (7 << 6) + (8 << 7) + (9 << 8) + (10 << 9) +
        (11 << 10) + (12 << 11) + (13 << 12) + (14 << 13) + (15 << 14);
    
    /* Use __builtin_constant_p to potentially generate both code paths */
    if (__builtin_constant_p(x)) {
        return x + 100;
    } else {
        return x + 200;
    }
}

/* Strategy 4: Vector shuffle/permute with large masks */
#ifdef __AVX512F__
static void test_vector_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Complex shuffle with many immediate constants */
    int mask[16] = {0, 16, 1, 17, 2, 18, 3, 19, 
                    4, 20, 5, 21, 6, 22, 7, 23};
    
    /* Create shuffle - may generate RTL with many immediate operands */
    v16sf result = __builtin_shuffle(v1, v2, 
        mask[0], mask[1], mask[2], mask[3], mask[4], mask[5], mask[6], mask[7],
        mask[8], mask[9], mask[10], mask[11], mask[12], mask[13], mask[14], mask[15]);
    
    volatile v16sf res = result;
    (void)res;
}
#endif

/* Strategy 5: C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex operation that might generate multi-operand RTL */
    return (a + b) * N + (a - b) / N + (a * b) % (N + 1) +
           (a & b) | (N << 2) + (a ^ b) & ~N;
}

static void test_template_instantiations(void) {
    /* Instantiate with different types and constants */
    int r1 = template_operation<int, 10>(1, 2);
    int r2 = template_operation<int, 11>(3, 4);
    int r3 = template_operation<int, 12>(5, 6);
    
    volatile int res = r1 + r2 + r3;
    (void)res;
}
#endif

/* Strategy 6: Memory operations with many addressing components */
static void test_complex_memory_ops(void) {
    struct LargeStruct {
        int64_t a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    };
    
    struct LargeStruct s1 = {0};
    struct LargeStruct s2 = {0};
    
    /* Complex memory copy that might use many operands */
    asm volatile (
        "/* Complex memory operation */\n\t"
        "movq 0(%1), %%rax\n\t"
        "movq %%rax, 0(%0)\n\t"
        "movq 8(%1), %%rax\n\t"
        "movq %%rax, 8(%0)\n\t"
        "movq 16(%1), %%rax\n\t"
        "movq %%rax, 16(%0)\n\t"
        "movq 24(%1), %%rax\n\t"
        "movq %%rax, 24(%0)\n\t"
        "movq 32(%1), %%rax\n\t"
        "movq %%rax, 32(%0)\n\t"
        "movq 40(%1), %%rax\n\t"
        "movq %%rax, 40(%0)\n\t"
        "movq 48(%1), %%rax\n\t"
        "movq %%rax, 48(%0)\n\t"
        "movq 56(%1), %%rax\n\t"
        "movq %%rax, 56(%0)"
        : 
        : "r" (&s1), "r" (&s2)
        : "rax", "memory"
    );
}

int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Execute all test strategies */
    
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
    test_inline_asm_10_operands();
    
    int const_result = test_complex_const_expression();
    printf("Constant expression result: %d\n", const_result);
    
#ifdef __AVX512F__
    test_vector_shuffle();
#endif
    
#ifdef __cplusplus
    test_template_instantiations();
#endif
    
    test_complex_memory_ops();
    
    return 0;
}
