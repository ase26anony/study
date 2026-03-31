/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector types for different architectures */
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

/* Pattern 1: Complex vector operations with chained FMA */
void test_vector_operations() {
    volatile int result = 0;
    
#if defined(__FMA__)
    /* Chained FMA operations that may generate many operands */
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Complex expression that might generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    result += (int)a[0];
#endif
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result));
}

/* Pattern 2: Inline assembly with exactly 10 operands */
void test_asm_10_operands() {
    int o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %1, %2, %0\n\t"
        "add %3, %4, %0\n\t"
        "add %5, %6, %0\n\t"
        "add %7, %8, %0\n\t"
        "add %9, %0, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int dummy = o0;
}

/* Pattern 3: Inline assembly with exactly 11 operands */
void test_asm_11_operands() {
    int o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
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
    
    volatile int dummy = o0;
}

/* Pattern 4: Complex shuffle/permute with many constant indices */
void test_vector_shuffle() {
#if defined(__SSE__) || defined(__AVX__)
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with many constant indices - may generate immediate operands */
    typedef int v4si __attribute__((vector_size(16)));
    v4si mask = {3, 2, 1, 0, 7, 6, 5, 4};  /* 8 indices for 2x4-element vectors */
    
    /* Complex shuffle that might use many operands */
    v4sf result = __builtin_shuffle(v1, v2, mask);
    
    volatile float dummy = result[0];
#endif
}

/* Pattern 5: Large constant expression that might not fold immediately */
int test_large_constant_expr() {
    /* Force compiler to consider all operands before folding */
    if (__builtin_constant_p(0)) {
        return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        volatile int a = 1, b = 2, c = 3, d = 4, e = 5, 
                    f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
        return a + b + c + d + e + f + g + h + i + j + k;
    }
}

/* Pattern 6: Target-specific builtins for multi-operand instructions */
void test_target_specific() {
#if defined(__AVX512F__)
    /* AVX-512 gather instruction with many parameters */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    
    float base[64] = {0};
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    volatile float dummy = _mm512_cvtss_f32(result);
#endif
}

/* Pattern 7: Mixed constraints in inline assembly */
void test_mixed_constraints() {
    int out1, out2;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int *mem = &in1;
    
    /* Mix of register, memory, and immediate constraints */
    asm volatile (
        "lea (%1,%2,4), %0\n\t"
        "add %3, %0\n\t"
        "imul $10, %4, %0\n\t"
        "add (%5), %0"
        : "=&r"(out1), "=r"(out2)
        : "r"(in1), "r"(in2), "r"(in3), "m"(*mem),
          "i"(10), "i"(4), "0"(out1), "1"(out2)
        : "cc"
    );
    
    volatile int dummy = out1 + out2;
}

/* Pattern 8: Template-like pattern using macros for C */
#define GEN_OPERATION(N) \
    int operation_##N(int a, int b) { \
        return a + b + N; \
    }

/* Generate multiple functions with different constant operands */
GEN_OPERATION(1)
GEN_OPERATION(2)
GEN_OPERATION(3)
GEN_OPERATION(4)
GEN_OPERATION(5)
GEN_OPERATION(6)
GEN_OPERATION(7)
GEN_OPERATION(8)
GEN_OPERATION(9)
GEN_OPERATION(10)
GEN_OPERATION(11)

/* Main function that exercises all patterns */
int main() {
    int result = 0;
    
    /* Test all patterns */
    test_vector_operations();
    test_asm_10_operands();
    test_asm_11_operands();
    test_vector_shuffle();
    result += test_large_constant_expr();
    test_target_specific();
    test_mixed_constraints();
    
    /* Use generated functions */
    result += operation_1(1, 2);
    result += operation_2(2, 3);
    result += operation_3(3, 4);
    result += operation_4(4, 5);
    result += operation_5(5, 6);
    result += operation_6(6, 7);
    result += operation_7(7, 8);
    result += operation_8(8, 9);
    result += operation_9(9, 10);
    result += operation_10(10, 11);
    result += operation_11(11, 12);
    
    /* Return non-zero to ensure all code is used */
    return result == 0 ? 1 : 0;
}
