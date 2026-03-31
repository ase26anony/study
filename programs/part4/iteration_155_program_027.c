/* test_optabs_coverage.c - Test program to cover 10/11 operand expansion cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Vector operations with many operands */
#ifdef __GNUC__

/* Define vector types */
typedef float v8sf __attribute__((vector_size(32)));  /* 8 floats = 256 bits */
typedef double v4df __attribute__((vector_size(32))); /* 4 doubles = 256 bits */
typedef int v8si __attribute__((vector_size(32)));    /* 8 ints = 256 bits */

/* Complex vector expression that might generate many operands */
static v8sf vector_operation_10_operands(v8sf a, v8sf b, v8sf c, v8sf d, v8sf e) {
    /* Nested FMA-like operations that could expand to many operands */
    v8sf result = a * b + c * d + e;
    result = result * a + b * c + d * e;
    result = __builtin_shuffle(result, result, 
        (v8si){7, 6, 5, 4, 3, 2, 1, 0});  /* Reverse shuffle */
    return result;
}

/* Another complex vector operation */
static v4df vector_operation_11_operands(v4df a, v4df b, v4df c, v4df d, v4df e, v4df f) {
    /* Complex expression that might generate 11 operands */
    v4df t1 = a * b + c;
    v4df t2 = d * e + f;
    v4df t3 = t1 * t2 + a;
    v4df t4 = b * c + d;
    v4df result = t3 * t4 + e * f;
    
    /* Add a complex shuffle with immediate indices */
    result = __builtin_shuffle(result, result, 
        (v4df){3.0, 2.0, 1.0, 0.0});  /* This creates immediate operands */
    
    return result;
}

#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
static void inline_asm_10_operands(void) {
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9;
    int o0;
    
    /* Initialize variables to prevent undefined behavior */
    i0 = 0; i1 = 1; i2 = 2; i3 = 3; i4 = 4;
    i5 = 5; i6 = 6; i7 = 7; i8 = 8; i9 = 9;
    
    /* Inline assembly with exactly 10 operands */
    asm volatile (
        "/* 10-operand dummy instruction %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9"
        : "=r" (o0)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3), 
          "r" (i4), "r" (i5), "r" (i6), "r" (i7), "r" (i8)
        : "cc"
    );
    
    /* Use the result to prevent optimization */
    volatile int use_result = o0;
    (void)use_result;
}

static void inline_asm_11_operands(void) {
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int o0;
    
    /* Initialize variables */
    i0 = 0; i1 = 1; i2 = 2; i3 = 3; i4 = 4;
    i5 = 5; i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9\n\t"
        "add %0, %10"
        : "=r" (o0)
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3), 
          "r" (i4), "r" (i5), "r" (i6), "r" (i7), 
          "r" (i8), "r" (i9), "r" (i10)
        : "cc"
    );
    
    /* Use the result */
    volatile int use_result = o0;
    (void)use_result;
}

/* Strategy 3: Complex constant expressions */
static int complex_constant_expression(void) {
    /* Force compiler to handle many constants in one expression */
    int result = 
        (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) |
        (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) |
        (1 << 10);
    
    /* Use __builtin_constant_p to force evaluation */
    if (__builtin_constant_p(result)) {
        return result + 1;
    } else {
        return result;
    }
}

/* Strategy 4: Target-specific builtins (x86 AVX-512 example) */
#ifdef __AVX512F__
#include <immintrin.h>

static void avx512_multi_operand_test(void) {
    /* AVX-512 gather instruction with many parameters */
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    __m512 result = _mm512_mask_i32gather_ps(
        _mm512_setzero_ps(),  // src
        mask,                 // mask
        index,                // vindex
        (void*)base,          // base
        4                     // scale
    );
    
    /* Use result to prevent optimization */
    volatile __m512 use_result = result;
    (void)use_result;
}
#endif

/* Strategy 5: Template/generic approach (using macros for C) */
#define GENERATE_COMPLEX_OP(TYPE, NAME, VAL1, VAL2, VAL3, VAL4, VAL5, VAL6, VAL7, VAL8, VAL9, VAL10) \
    static TYPE NAME(void) { \
        TYPE a = (TYPE)VAL1; \
        TYPE b = (TYPE)VAL2; \
        TYPE c = (TYPE)VAL3; \
        TYPE d = (TYPE)VAL4; \
        TYPE e = (TYPE)VAL5; \
        TYPE f = (TYPE)VAL6; \
        TYPE g = (TYPE)VAL7; \
        TYPE h = (TYPE)VAL8; \
        TYPE i = (TYPE)VAL9; \
        TYPE j = (TYPE)VAL10; \
        \
        /* Complex expression with many operands */ \
        TYPE result = a + b * c - d / e + f % g + h & i | j; \
        result = result * a - b + c / d * e - f + g % h & i | j; \
        return result; \
    }

/* Generate multiple instantiations */
GENERATE_COMPLEX_OP(int, complex_int_op, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
GENERATE_COMPLEX_OP(long, complex_long_op, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100)
GENERATE_COMPLEX_OP(float, complex_float_op, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f)

/* Main function that exercises all strategies */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test inline assembly strategies */
    inline_asm_10_operands();
    inline_asm_11_operands();
    
    /* Test complex constant expressions */
    int const_result = complex_constant_expression();
    printf("Constant expression result: %d\n", const_result);
    
    /* Test generic/macro-generated functions */
    int int_result = complex_int_op();
    long long_result = complex_long_op();
    float float_result = complex_float_op();
    printf("Generic ops results: int=%d, long=%ld, float=%f\n", 
           int_result, long_result, float_result);
    
#ifdef __GNUC__
    /* Test vector operations if supported */
    v8sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vec2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf vec3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf vec4 = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    v8sf vec5 = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
    
    v8sf vec_result = vector_operation_10_operands(vec1, vec2, vec3, vec4, vec5);
    
    /* Use vector result */
    volatile float use_vec = ((float*)&vec_result)[0];
    (void)use_vec;
    
    printf("Vector operations completed\n");
#endif
    
#ifdef __AVX512F__
    /* Test AVX-512 specific operations */
    avx512_multi_operand_test();
    printf("AVX-512 operations completed\n");
#endif
    
    return 0;
}
