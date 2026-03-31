/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for various architectures */
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

/* Approach 1: Complex vector operations that may generate many operands */
#ifdef __AVX512F__
static v8df complex_vector_operation(v8df a, v8df b, v8df c, v8df d, v8df e) {
    /* This complex expression might generate RTL with many operands */
    return __builtin_fma(a, b, __builtin_fma(c, d, __builtin_fma(e, a, 
           __builtin_fma(b, c, __builtin_fma(d, e, a)))));
}
#endif

/* Approach 2: Inline assembly with exactly 10 and 11 operands */
static void inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
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
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5), 
          "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t sink = o0; /* Prevent optimization */
}

static void inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* Exactly 11 operands: 1 output + 10 inputs */
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
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5), 
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int64_t sink = o0; /* Prevent optimization */
}

/* Approach 3: Complex constant expression with many terms */
static int complex_constant_expression(void) {
    /* Force compiler to handle many constants in one expression */
    int result = 
        __builtin_constant_p(1) ? 1 : 0 +
        __builtin_constant_p(2) ? 2 : 0 +
        __builtin_constant_p(3) ? 3 : 0 +
        __builtin_constant_p(4) ? 4 : 0 +
        __builtin_constant_p(5) ? 5 : 0 +
        __builtin_constant_p(6) ? 6 : 0 +
        __builtin_constant_p(7) ? 7 : 0 +
        __builtin_constant_p(8) ? 8 : 0 +
        __builtin_constant_p(9) ? 9 : 0 +
        __builtin_constant_p(10) ? 10 : 0 +
        __builtin_constant_p(11) ? 11 : 0;
    
    return result;
}

/* Approach 4: Vector shuffle with large constant mask */
#ifdef __AVX512F__
static v16sf vector_shuffle_large_mask(v16sf a, v16sf b) {
    /* Shuffle with a 16-element constant mask - may generate many immediate operands */
    const int mask[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    
    /* Use __builtin_shuffle with all constant indices */
    return __builtin_shuffle(a, b, 
        mask[0], mask[1], mask[2], mask[3], mask[4], mask[5], mask[6], mask[7],
        mask[8], mask[9], mask[10], mask[11], mask[12], mask[13], mask[14], mask[15]);
}
#endif

/* Approach 5: Mixed constraints in inline assembly */
static void mixed_constraint_asm(void) {
    int o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int *i6 = &i1, *i7 = &i2;
    const int i8 = 8, i9 = 9, i10 = 10;
    
    /* Mix register, memory, and immediate constraints */
    asm volatile (
        "/* mixed constraints asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add (%6), %0\n\t"
        "add (%7), %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "i"(i8), "i"(i9), "i"(i10)
        : "memory", "cc"
    );
    
    volatile int sink = o0;
}

/* Template approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex template operation that might expand differently */
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) + 
           static_cast<T>(N+3) + static_cast<T>(N+4) +
           static_cast<T>(N+5) + static_cast<T>(N+6) +
           static_cast<T>(N+7) + static_cast<T>(N+8);
}
#endif

int main(void) {
    printf("Testing 10/11 operand expansion coverage\n");
    
    /* Test inline assembly with exactly 10 and 11 operands */
    inline_asm_10_operands();
    inline_asm_11_operands();
    
    /* Test mixed constraint assembly */
    mixed_constraint_asm();
    
    /* Test complex constant expression */
    volatile int const_result = complex_constant_expression();
    
#ifdef __AVX512F__
    /* Test vector operations if AVX-512 is available */
    v8df vec1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df vec2 = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df vec3 = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df vec4 = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df vec5 = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    v8df vec_result = complex_vector_operation(vec1, vec2, vec3, vec4, vec5);
    volatile double sink = vec_result[0];
    
    /* Test vector shuffle */
    v16sf vecf1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                   9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf vecf2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                   25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    v16sf shuffle_result = vector_shuffle_large_mask(vecf1, vecf2);
    volatile float sinkf = shuffle_result[0];
#endif

#ifdef __cplusplus
    /* Test template instantiations */
    int template_int = template_operation<int, 1>(10, 20);
    double template_double = template_operation<double, 2>(10.5, 20.5);
    volatile int template_sink = template_int;
    volatile double template_sink_d = template_double;
#endif
    
    return 0;
}
