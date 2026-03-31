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
    /* Use volatile to prevent optimization */
    volatile v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    volatile v4sf c = {9.0f, 10.0f, 11.0f, 12.0f};
    volatile v4sf d = {13.0f, 14.0f, 15.0f, 16.0f};
    volatile v4sf e = {17.0f, 18.0f, 19.0f, 20.0f};
    volatile v4sf f = {21.0f, 22.0f, 23.0f, 24.0f};
    
    /* Complex expression that might generate multi-operand RTL */
    v4sf result = a + b * c + d * e + f;
    
    /* Use result to prevent dead code elimination */
    asm volatile ("" : "+x" (result));
}

/* Strategy 2: Inline assembly with many operands */
void test_multi_operand_asm() {
    int64_t o0, o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    int64_t i0 = 0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11-operand inline assembly */
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
        : "=r" (o0)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3), "r" (i4),
          "r" (i5), "r" (i6), "r" (i7), "r" (i8), "r" (i9)
        : "cc"
    );
    
    /* 10-operand inline assembly with mixed constraints */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "lea (%1,%2,1), %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r" (o1)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3), "r" (i4),
          "r" (i5), "r" (i6), "r" (i7), "i" (100)
        : "cc"
    );
}

/* Strategy 3: Complex constant expressions */
int test_complex_const_expr() {
    /* Force compiler to consider complex constant expression */
    int x = 0;
    
    /* This might generate RTL with many immediate operands */
    if (__builtin_constant_p(1)) {
        x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        x = 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20 + 21;
    }
    
    /* Nested conditional with many constants */
    int y = (x > 0) ? 
            (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) |
            (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) :
            (2 << 0) | (2 << 1) | (2 << 2) | (2 << 3) | (2 << 4) |
            (2 << 5) | (2 << 6) | (2 << 7) | (2 << 8) | (2 << 9) | (2 << 10);
    
    return x + y;
}

/* Strategy 4: Vector shuffle with large mask */
void test_vector_shuffle() {
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Complex shuffle with immediate mask */
    typedef int v4si __attribute__((vector_size(16)));
    v4si mask = {3, 2, 1, 0};
    
    /* This might generate RTL with vector and mask operands */
    v4sf shuffled = __builtin_shuffle(a, b, mask);
    
    asm volatile ("" : "+x" (shuffled));
}

/* Strategy 5: Use target-specific builtins when available */
#ifdef __AVX512F__
#include <immintrin.h>
void test_avx512_gather() {
    /* AVX-512 gather can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    float base[64] = {0};
    
    __m512 result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(),
                                            mask,
                                            index,
                                            base,
                                            4);
    asm volatile ("" : "+x" (result));
}
#endif

/* Template/Generic approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that might expand differently */
    return a + b + (a * b) + (a / (b + 1)) + 
           (a << 1) + (b >> 1) + (a & b) + (a | b) +
           (a ^ b) + (N * a) + (N * b);
}

void test_template_instantiations() {
    /* Instantiate with different types and constants */
    int r1 = template_operation<int, 10>(1, 2);
    float r2 = template_operation<float, 11>(1.0f, 2.0f);
    double r3 = template_operation<double, 12>(1.0, 2.0);
    
    asm volatile ("" : "+r" (r1), "+r" (r2), "+r" (r3));
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing multi-operand expansion coverage...\n");
    
    /* Execute all test strategies */
    test_vector_operations();
    test_multi_operand_asm();
    
    int const_result = test_complex_const_expr();
    printf("Constant expression result: %d\n", const_result);
    
    test_vector_shuffle();
    
    #ifdef __AVX512F__
    test_avx512_gather();
    #endif
    
    #ifdef __cplusplus
    test_template_instantiations();
    #endif
    
    /* Ensure results are used */
    volatile int dummy = const_result;
    
    return 0;
}
