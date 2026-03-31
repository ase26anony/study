/* test_optabs_10_11_operands.c */
/* Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-expand -march=native -o test test_optabs_10_11_operands.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Vector types for AVX/AVX-512 operations */
typedef double v4df __attribute__((vector_size(32)));  /* 4 doubles = 256 bits */
typedef float v8sf __attribute__((vector_size(32)));   /* 8 floats = 256 bits */
typedef int v8si __attribute__((vector_size(32)));     /* 8 ints = 256 bits */

/* For AVX-512 if available */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));  /* 8 doubles = 512 bits */
typedef float v16sf __attribute__((vector_size(64)));  /* 16 floats = 512 bits */
#endif

/* Force usage of results to prevent optimization */
static volatile int use_result_int;
static volatile double use_result_double;

/* Pattern 1: Complex vector FMA chain that may generate many operands */
void test_vector_fma_chain(void) {
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Complex FMA chain - may generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    a = __builtin_fma(a, b, __builtin_fma(c, d, e));
    
    /* Use result to prevent dead code elimination */
    use_result_double = a[0] + a[1] + a[2] + a[3];
}

/* Pattern 2: Vector shuffle with large constant mask */
void test_vector_shuffle(void) {
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Large shuffle mask - 8 immediate operands */
    v8sf result = __builtin_shuffle(v1, v2, 
        (v8si){7, 6, 5, 4, 3, 2, 1, 0});
    
    /* Another shuffle with different mask */
    result = __builtin_shuffle(result, v1,
        (v8si){0, 2, 4, 6, 1, 3, 5, 7});
    
    use_result_double = result[0] + result[7];
}

/* Pattern 3: Inline assembly with exactly 10 operands */
void test_asm_10_operands(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9;
    int64_t out1, out2;
    
    /* 10 operands: 2 outputs, 8 inputs */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0"
        : "=r"(out1), "=r"(out2)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4),
          "r"(in5), "r"(in6), "r"(in7), "r"(in8)
        : "cc"
    );
    
    /* 10 operands with mixed constraints */
    int mem1 = 100, mem2 = 200;
    asm volatile (
        "/* mixed constraints */\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "imul %4, %0"
        : "+r"(out1), "=m"(mem1)
        : "r"(in9), "i"(10), "m"(mem2)
        : "cc"
    );
    
    use_result_int = out1 + out2 + mem1;
}

/* Pattern 4: Inline assembly with exactly 11 operands */
void test_asm_11_operands(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out1, out2, out3;
    
    /* 11 operands: 3 outputs, 8 inputs */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
        "mov %1, %0\n\t"
        "mov %2, %3\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %3"
        : "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8), "r"(in9)
        : "cc"
    );
    
    use_result_int = out1 + out2 + out3;
}

/* Pattern 5: Complex constant expression that may not fold immediately */
int test_complex_const_expr(void) {
    /* Large constant expression - may generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to potentially delay folding */
    if (__builtin_constant_p(x)) {
        x = x * 2;
    } else {
        x = x + 100;
    }
    
    /* Nested constant expressions */
    int y = (x << 1) | (x >> 3) & (x + 1) ^ (x - 1);
    
    return y;
}

/* Pattern 6: AVX-512 specific patterns if available */
#ifdef __AVX512F__
void test_avx512_gather(void) {
    /* Simulate gather operation with multiple parameters */
    v8si index = {0, 8, 16, 24, 32, 40, 48, 56};
    float base[64];
    for (int i = 0; i < 64; i++) base[i] = i * 1.0f;
    
    v16sf result;
    /* This may expand to multi-operand RTL on AVX-512 targets */
    for (int i = 0; i < 8; i++) {
        result[i] = base[index[i]];
        result[i+8] = base[index[i] + 1];
    }
    
    use_result_double = result[0] + result[15];
}
#endif

/* Pattern 7: Mixed operations in single expression */
void test_mixed_operations(void) {
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    
    /* Complex expression that may generate many operands */
    v4df result = a + b * c - d / a + b - c * d + a / b;
    
    /* Chain operations */
    result = result * a + b - c / d * a + b - c;
    
    use_result_double = result[0];
}

/* Pattern 8: Memory operations with many parameters */
void test_memory_ops(void) {
    struct LargeStruct {
        int a, b, c, d, e, f, g, h, i, j, k, l;
    } s1 = {0}, s2 = {0};
    
    /* Memory copy that may expand to multi-operand RTL */
    memcpy(&s1, &s2, sizeof(struct LargeStruct));
    
    /* Multiple field assignments */
    s1.a = 1; s1.b = 2; s1.c = 3; s1.d = 4; s1.e = 5;
    s1.f = 6; s1.g = 7; s1.h = 8; s1.i = 9; s1.j = 10;
    
    use_result_int = s1.a + s1.j;
}

/* Template pattern for C++ (if compiled as C++) */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    return a + b + N + (N << 1) + (N >> 2) + (N & 0xFF) + (N | 0x55);
}

void test_template_instances(void) {
    int r1 = template_operation<int, 1>(10, 20);
    int r2 = template_operation<int, 2>(30, 40);
    int r3 = template_operation<int, 3>(50, 60);
    int r4 = template_operation<int, 4>(70, 80);
    int r5 = template_operation<int, 5>(90, 100);
    
    use_result_int = r1 + r2 + r3 + r4 + r5;
}
#endif

/* Main function that exercises all patterns */
int main(void) {
    printf("Testing 10/11 operand expansion patterns...\n");
    
    /* Execute all test patterns */
    test_vector_fma_chain();
    test_vector_shuffle();
    test_asm_10_operands();
    test_asm_11_operands();
    
    int const_result = test_complex_const_expr();
    printf("Constant expression result: %d\n", const_result);
    
    test_mixed_operations();
    test_memory_ops();
    
#ifdef __AVX512F__
    test_avx512_gather();
    printf("AVX-512 patterns tested\n");
#endif
    
#ifdef __cplusplus
    test_template_instances();
    printf("C++ template patterns tested\n");
#endif
    
    printf("All tests completed.\n");
    return 0;
}
