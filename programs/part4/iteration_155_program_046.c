/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
    
#if defined(__FMA__) || defined(__AVX512F__)
    /* Use FMA operations that may generate multi-operand RTL */
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    
    /* Use result to prevent optimization */
    result += (int)a[0];
#endif
    
    /* Complex shuffle with many constant indices */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with many constant operands */
    v4sf shuffled = __builtin_shuffle(v1, v2, 
        (typeof(v1)){0, 4, 1, 5, 2, 6, 3, 7});
    
    result += (int)shuffled[0];
    
    printf("Vector result: %d\n", result);
}

/* Strategy 2: Inline assembly with exactly 10/11 operands */
void test_inline_asm_10_operands() {
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int o0 = 0;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
    asm volatile (
        "# 10-operand asm\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        : "=r"(o0) 
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    printf("10-operand asm result: %d\n", o0);
}

void test_inline_asm_11_operands() {
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10, i11 = 11;
    int o0 = 0;
    
    /* Exactly 11 operands: 1 output + 10 inputs */
    asm volatile (
        "# 11-operand asm\n\t"
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        "add %9, %10, %0\n\t"
        : "=r"(o0) 
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), 
          "r"(i9), "r"(i10), "r"(i11)
        : "cc"
    );
    
    printf("11-operand asm result: %d\n", o0);
}

/* Strategy 3: Mixed constraints in inline assembly */
void test_mixed_constraints_asm() {
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int result = 0;
    
    /* Mix of register, memory, and immediate constraints */
    asm volatile (
        "# Mixed constraints\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0\n\t"
        : "=r"(result)
        : "r"(arr[0]), "r"(arr[1]), "m"(arr[2]), "i"(4),
          "r"(arr[4]), "m"(arr[5]), "i"(7), "r"(arr[7]),
          "m"(arr[8]), "r"(arr[9])
        : "cc"
    );
    
    printf("Mixed constraints result: %d\n", result);
}

/* Strategy 4: Target-specific builtins */
#ifdef __AVX512F__
#include <immintrin.h>
void test_avx512_gather() {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    __m512 result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, index, base, 4);
    
    /* Use result to prevent optimization */
    volatile float f = result[0];
    (void)f;
}
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
void test_neon_ld4() {
    /* ARM NEON LD4 loads 4 registers - may generate multi-operand RTL */
    float32x4x4_t result;
    float32_t* ptr = malloc(16 * sizeof(float32_t));
    
    result = vld4q_f32(ptr);
    
    /* Use result */
    volatile float f = vgetq_lane_f32(result.val[0], 0);
    (void)f;
    free(ptr);
}
#endif

/* Strategy 5: Complex constant expressions */
int test_complex_const_expr() {
    /* Force compiler to handle complex constant expression */
    int x = 0;
    
    /* Large constant expression that might not fold immediately */
    if (__builtin_constant_p(0)) {
        x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        x = 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20 + 21 + 22;
    }
    
    /* Another complex expression */
    int y = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | 
            (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) |
            (1 << 8) | (1 << 9) | (1 << 10);
    
    return x + y;
}

/* Strategy 6: C++ templates for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex operation that might generate multi-operand RTL */
    return a + b + (T)N + (T)(N+1) + (T)(N+2) + (T)(N+3) + 
           (T)(N+4) + (T)(N+5) + (T)(N+6) + (T)(N+7);
}

void test_templates() {
    int r1 = template_operation<int, 1>(10, 20);
    float r2 = template_operation<float, 2>(10.5f, 20.5f);
    double r3 = template_operation<double, 3>(10.5, 20.5);
    
    printf("Template results: %d %f %f\n", r1, r2, r3);
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test vector operations */
    test_vector_operations();
    
    /* Test inline assembly with exact operand counts */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    test_mixed_constraints_asm();
    
    /* Test target-specific builtins if available */
#ifdef __AVX512F__
    test_avx512_gather();
#endif
    
#ifdef __ARM_NEON
    test_neon_ld4();
#endif
    
    /* Test complex constant expressions */
    int const_result = test_complex_const_expr();
    printf("Constant expression result: %d\n", const_result);
    
#ifdef __cplusplus
    /* Test C++ templates if compiling as C++ */
    test_templates();
#endif
    
    return 0;
}
