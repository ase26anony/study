/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Vector types for different architectures */
typedef float v8sf __attribute__((vector_size(32)));  /* 8 floats */
typedef double v4df __attribute__((vector_size(32))); /* 4 doubles */
typedef int v8si __attribute__((vector_size(32)));    /* 8 ints */

/* Strategy 1: Complex vector operations with FMA chaining */
__attribute__((noinline))
v4df vector_fma_chain(v4df a, v4df b, v4df c, v4df d, v4df e) {
    /* This may generate RTL with many operands when expanded */
    return __builtin_fma(a, b, __builtin_fma(c, d, e));
}

/* Strategy 2: Vector shuffle with large constant mask */
__attribute__((noinline))
v8sf vector_shuffle_complex(v8sf a, v8sf b) {
    /* Shuffle with 8-element constant mask */
    return __builtin_shuffle(a, b, 
        (v8si){7, 6, 5, 4, 3, 2, 1, 0});
}

/* Strategy 3: Multiple inline assembly statements with many operands */
__attribute__((noinline))
void inline_asm_10_operands(int *out, int in1, int in2, int in3, int in4,
                           int in5, int in6, int in7, int in8, int in9) {
    int result;
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        : "=r"(result)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4),
          "r"(in5), "r"(in6), "r"(in7), "r"(in8),
          "r"(in9)
        : "cc"
    );
    *out = result;
}

__attribute__((noinline))
void inline_asm_11_operands(int *out, int in1, int in2, int in3, int in4,
                           int in5, int in6, int in7, int in8, int in9,
                           int in10) {
    int result;
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        "add %9, %10, %0\n\t"
        : "=r"(result)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4),
          "r"(in5), "r"(in6), "r"(in7), "r"(in8),
          "r"(in9), "r"(in10)
        : "cc"
    );
    *out = result;
}

/* Strategy 4: Mixed constraints in inline assembly */
__attribute__((noinline))
void mixed_constraint_asm(int *mem, int reg1, int reg2) {
    /* Mix of register, memory, and immediate constraints */
    asm volatile (
        "/* mixed constraints */\n\t"
        "add %1, %2, %0\n\t"
        "add %0, %3, %0\n\t"
        "add %0, %4, %0\n\t"
        : "+r"(reg1), "+m"(*mem)
        : "r"(reg2), "i"(42), "i"(100)
        : "cc"
    );
}

/* Strategy 5: Complex constant expression */
__attribute__((noinline))
int complex_constant_expr(void) {
    /* Force compiler to handle many constants */
    if (__builtin_constant_p(1)) {
        return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        /* This branch forces RTL generation for the constant expression */
        volatile int x = 1;
        return x + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    }
}

/* Strategy 6: Target-specific builtins (x86 AVX-512 example) */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
__m512i avx512_gather_like(__m512i index, __m512i mask, int scale) {
    /* Simulate a gather-like operation with many parameters */
    __m512i result = _mm512_setzero_si512();
    /* Complex expression that might use many operands */
    result = _mm512_add_epi32(result, index);
    result = _mm512_add_epi32(result, mask);
    result = _mm512_add_epi32(result, _mm512_set1_epi32(scale));
    return result;
}
#endif

/* C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that might expand to many operands */
    return a + b + (T)N + (T)(N+1) + (T)(N+2) + (T)(N+3) + 
           (T)(N+4) + (T)(N+5) + (T)(N+6) + (T)(N+7);
}

/* Instantiate with different types */
template int template_operation<int, 1>(int, int);
template float template_operation<float, 2>(float, float);
template double template_operation<double, 3>(double, double);
#endif

/* Main function that uses all patterns */
int main(void) {
    volatile int result = 0;
    int out1, out2;
    
    /* Test vector operations */
    v4df v1 = {1.0, 2.0, 3.0, 4.0};
    v4df v2 = {5.0, 6.0, 7.0, 8.0};
    v4df v3 = {9.0, 10.0, 11.0, 12.0};
    v4df v4 = {13.0, 14.0, 15.0, 16.0};
    v4df v5 = {17.0, 18.0, 19.0, 20.0};
    
    v4df vresult = vector_fma_chain(v1, v2, v3, v4, v5);
    result += (int)vresult[0];
    
    /* Test inline assembly with many operands */
    inline_asm_10_operands(&out1, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    inline_asm_11_operands(&out2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += out1 + out2;
    
    /* Test mixed constraint assembly */
    int mem = 42;
    mixed_constraint_asm(&mem, 10, 20);
    result += mem;
    
    /* Test complex constant expression */
    result += complex_constant_expr();
    
    /* Test vector shuffle */
    v8sf sv1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf sv2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf svresult = vector_shuffle_complex(sv1, sv2);
    result += (int)svresult[0];
    
    #ifdef __cplusplus
    /* Test template instantiations */
    result += template_operation<int, 1>(10, 20);
    result += (int)template_operation<float, 2>(10.5f, 20.5f);
    result += (int)template_operation<double, 3>(10.5, 20.5);
    #endif
    
    printf("Result: %d\n", result);
    return 0;
}
