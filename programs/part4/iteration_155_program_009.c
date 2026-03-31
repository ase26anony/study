/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Define vector types for different architectures */
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
void test_vector_operations() {
    volatile int result = 0;
    
#if defined(__FMA__)
    /* Use FMA operations which can generate multi-operand RTL */
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    result += (int)a[0];
#endif
    
    /* Use shuffle with large constant mask */
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex shuffle with many immediate operands */
    v8sf shuffled = __builtin_shuffle(v1, v2, 
        (typeof(v1)){0, 8, 1, 9, 2, 10, 3, 11});
    result += (int)shuffled[0];
    
    (void)result; /* Prevent unused variable warning */
}

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
void test_inline_asm() {
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int o0, o1;
    
    /* Initialize variables to prevent undefined behavior */
    i0 = 0; i1 = 1; i2 = 2; i3 = 3; i4 = 4;
    i5 = 5; i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    
    /* 10 operand inline assembly */
    asm volatile (
        "/* 10-operand asm */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    /* 11 operand inline assembly with mixed constraints */
    asm volatile (
        "/* 11-operand asm */\n\t"
        "mov %0, #0\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=&r"(o1)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3),
          "r"(i4), "r"(i5), "r"(i6), "r"(i7),
          "r"(i8), "i"(42)  /* Mixed register and immediate */
        : "cc"
    );
    
    /* Use results to prevent dead code elimination */
    volatile int dummy = o0 + o1;
    (void)dummy;
}

/* Strategy 3: Target-specific builtins */
void test_target_builtins() {
#if defined(__AVX512F__)
    /* AVX-512 gather instructions have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    float* base = (float*)0x1000;
    
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    volatile float f = _mm512_cvtss_f32(result);
    (void)f;
#endif
    
#if defined(__ARM_NEON) || defined(__aarch64__)
    /* ARM NEON structured loads can have multiple operands */
    float32x4x4_t data;
    float* ptr = (float*)0x1000;
    
    /* Load 4 registers with post-increment */
    data = vld4q_f32(ptr);
    volatile float f0 = data.val[0][0];
    (void)f0;
#endif
}

/* Strategy 4: Complex constant expressions */
int test_const_expressions() {
    /* Large constant expression that may not fold immediately */
    int x = 
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
    
    return x;
}

/* Strategy 5: Template/Generic patterns (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that may generate multi-operand RTL */
    return a + b + (T)N + (T)(N+1) + (T)(N+2) + 
           (T)(N+3) + (T)(N+4) + (T)(N+5) +
           (T)(N+6) + (T)(N+7) + (T)(N+8);
}

void test_templates() {
    /* Instantiate with multiple types */
    int r1 = template_operation<int, 1>(1, 2);
    float r2 = template_operation<float, 2>(1.0f, 2.0f);
    double r3 = template_operation<double, 3>(1.0, 2.0);
    
    volatile int dummy = r1 + (int)r2 + (int)r3;
    (void)dummy;
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Execute all test strategies */
    test_vector_operations();
    test_inline_asm();
    test_target_builtins();
    
    int const_result = test_const_expressions();
    printf("Constant expression result: %d\n", const_result);
    
#ifdef __cplusplus
    test_templates();
#endif
    
    return 0;
}
