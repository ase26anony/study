/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
    
    /* Use vector shuffles with large constant masks */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    int mask = 0x1B3A;  /* Complex shuffle mask */
    v4sf shuffled = __builtin_shuffle(v1, v2, mask);
    result += (int)shuffled[0];
}

/* Strategy 2: Inline assembly with many operands */
void test_inline_asm() {
    /* 10-operand inline assembly */
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
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
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    /* 11-operand inline assembly with mixed constraints */
    int64_t o1;
    int64_t i10 = 10;
    const char *mem = "test";
    
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
        : "=r"(o1)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), 
          "r"(i9), "r"(i10), "m"(*mem)
        : "cc", "memory"
    );
    
    volatile int dummy = o0 + o1;
}

/* Strategy 3: Target-specific builtins */
void test_target_builtins() {
#if defined(__AVX512F__)
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    void *base = malloc(64);
    
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    free(base);
    
    volatile float f = _mm512_cvtss_f32(result);
#endif
    
#if defined(__ARM_NEON) || defined(__aarch64__)
    /* ARM NEON/SVE multi-register loads */
    float32x4x4_t data;
    float *ptr = malloc(64);
    
    data = vld4q_f32(ptr);  /* Loads 4 registers - may generate multi-operand RTL */
    free(ptr);
#endif
}

/* Strategy 4: Complex constant expressions */
int test_const_expressions() {
    /* Force compiler to handle large constant expressions */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to prevent early folding */
    if (__builtin_constant_p(x)) {
        x = x * 2;
    } else {
        x = x / 2;
    }
    
    /* Nested conditional expressions with many constants */
    int y = (x > 0) ? 
            (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10) :
            (11 + 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20);
    
    return x + y;
}

/* Strategy 5: Template/generic patterns (C++ compatible) */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that might generate multi-operand RTL */
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) + 
           static_cast<T>(N+3) + static_cast<T>(N+4) + 
           static_cast<T>(N+5) + static_cast<T>(N+6) + 
           static_cast<T>(N+7);
}

void test_templates() {
    int r1 = template_operation<int, 1>(10, 20);
    float r2 = template_operation<float, 2>(10.5f, 20.5f);
    double r3 = template_operation<double, 3>(10.5, 20.5);
    
    volatile int dummy = r1 + (int)r2 + (int)r3;
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Call all test functions to ensure code is not eliminated */
    test_vector_operations();
    test_inline_asm();
    test_target_builtins();
    int const_result = test_const_expressions();
    
#ifdef __cplusplus
    test_templates();
#endif
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = const_result;
    
    printf("Test completed.\n");
    return 0;
}
