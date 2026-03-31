/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Vector types for different architectures */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Prevent optimization */
static volatile int sink;

/* Method 1: Complex vector operations with FMA chaining */
__attribute__((noinline))
v4df vector_fma_chain(v4df a, v4df b, v4df c, v4df d, v4df e) {
    /* Chain multiple FMA operations - may generate many operands */
    return __builtin_fma(a, b, __builtin_fma(c, d, e));
}

/* Method 2: Vector shuffle with large constant mask */
__attribute__((noinline))
v8sf vector_shuffle_complex(v8sf a, v8sf b) {
    /* Shuffle with 8-element constant mask */
    const int mask[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    return __builtin_shuffle(a, b, mask);
}

/* Method 3: Inline assembly with exactly 10 operands */
__attribute__((noinline))
void asm_10_operands(void) {
    int64_t o0, i1, i2, i3, i4, i5, i6, i7, i8, i9;
    
    /* Initialize to prevent undefined behavior */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9;
    
    /* Extended asm with 10 total operands (1 output + 9 inputs) */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9)
        : "cc"
    );
    
    sink = (int)o0;
}

/* Method 4: Inline assembly with exactly 11 operands */
__attribute__((noinline))
void asm_11_operands(void) {
    int64_t o0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    
    /* Extended asm with 11 total operands (1 output + 10 inputs) */
    asm volatile (
        "mov %[out], %[in1]\n\t"
        "add %[out], %[out], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9),
          [in10] "r" (i10)
        : "cc"
    );
    
    sink = (int)o0;
}

/* Method 5: Complex constant expression that may not fold immediately */
__attribute__((noinline))
int complex_const_expr(void) {
    /* Large constant expression - may generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to potentially prevent early folding */
    if (__builtin_constant_p(x)) {
        return x + 1;
    } else {
        return x - 1;
    }
}

/* Method 6: Target-specific builtins (x86 AVX-512 example) */
#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noinline))
void avx512_gather_test(void) {
    __m512i index = _mm512_set1_epi32(0);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    float* base = (float*)&sink;
    
    /* AVX-512 gather with multiple parameters */
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    
    /* Use result to prevent optimization */
    sink = _mm512_cvtss_f32(result);
}
#endif

/* Method 7: Mixed constraints in inline assembly */
__attribute__((noinline))
void mixed_constraint_asm(void) {
    int o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    int mem_var = 10;
    
    /* Mix of register, memory, and immediate constraints */
    asm volatile (
        "lea (%[m1], %[r1]), %[out]\n\t"
        "add %[imm1], %[out]\n\t"
        "add %[r2], %[out]\n\t"
        "add %[r3], %[out]\n\t"
        "add %[r4], %[out]\n\t"
        "add %[r5], %[out]\n\t"
        "add %[r6], %[out]\n\t"
        "add %[r7], %[out]\n\t"
        "add %[r8], %[out]"
        : [out] "=r" (o0)
        : [r1] "r" (i1), [r2] "r" (i2), [r3] "r" (i3),
          [r4] "r" (i4), [r5] "r" (i5), [r6] "r" (i6),
          [r7] "r" (i7), [r8] "r" (i8), [r9] "r" (i9),
          [m1] "m" (mem_var), [imm1] "i" (100)
        : "cc"
    );
    
    sink = o0;
}

/* Template approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that may generate many operands */
    return a + b + (T)N + (T)(N+1) + (T)(N+2) + (T)(N+3) + 
           (T)(N+4) + (T)(N+5) + (T)(N+6) + (T)(N+7);
}

void test_templates(void) {
    /* Instantiate with different types */
    int r1 = template_operation<int, 1>(1, 2);
    float r2 = template_operation<float, 2>(1.0f, 2.0f);
    double r3 = template_operation<double, 3>(1.0, 2.0);
    
    sink = r1 + (int)r2 + (int)r3;
}
#endif

int main(void) {
    /* Initialize vector variables */
    v4df v1 = {1.0, 2.0, 3.0, 4.0};
    v4df v2 = {5.0, 6.0, 7.0, 8.0};
    v4df v3 = {9.0, 10.0, 11.0, 12.0};
    v4df v4 = {13.0, 14.0, 15.0, 16.0};
    v4df v5 = {17.0, 18.0, 19.0, 20.0};
    
    v8sf vs1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vs2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Test all methods */
    v4df vresult = vector_fma_chain(v1, v2, v3, v4, v5);
    sink = (int)vresult[0];
    
    v8sf vsresult = vector_shuffle_complex(vs1, vs2);
    sink = (int)vsresult[0];
    
    asm_10_operands();
    asm_11_operands();
    
    int cresult = complex_const_expr();
    sink = cresult;
    
    mixed_constraint_asm();
    
    #ifdef __AVX512F__
    avx512_gather_test();
    #endif
    
    #ifdef __cplusplus
    test_templates();
    #endif
    
    /* Ensure all results are used */
    printf("Result: %d\n", sink);
    
    return 0;
}
