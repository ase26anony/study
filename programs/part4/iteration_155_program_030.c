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

/* Strategy 1: Complex vector operations that might generate multi-operand RTL */
static void test_vector_operations(void) {
    /* Use volatile to prevent optimization */
    volatile v4df a = {1.0, 2.0, 3.0, 4.0};
    volatile v4df b = {5.0, 6.0, 7.0, 8.0};
    volatile v4df c = {9.0, 10.0, 11.0, 12.0};
    volatile v4df d = {13.0, 14.0, 15.0, 16.0};
    volatile v4df e = {17.0, 18.0, 19.0, 20.0};
    volatile v4df f = {21.0, 22.0, 23.0, 24.0};
    
    /* Complex expression that might generate RTL with many operands */
    v4df result = a + b * c + d * e + f;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+x"(result));
}

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* Exactly 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
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
    
    /* Use the result */
    asm volatile("" : "+r"(o0));
}

static void test_inline_asm_11_operands(void) {
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
    
    /* Use the result */
    asm volatile("" : "+r"(o0));
}

/* Strategy 3: Complex constant expression with many terms */
static int test_complex_constant_expr(void) {
    /* Large constant expression - might generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 +
            12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20;
    
    /* Use __builtin_constant_p to potentially generate RTL for both paths */
    if (__builtin_constant_p(x)) {
        return x + 100;
    } else {
        return x + 200;
    }
}

/* Strategy 4: Vector shuffle with large constant mask */
static void test_vector_shuffle(void) {
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with constant mask - might generate RTL with immediate operands */
    int mask[8] = {0, 4, 1, 5, 2, 6, 3, 7};
    v4sf result;
    
    /* Complex shuffle operation */
    result = __builtin_shuffle(a, b, (int[]){mask[0], mask[1], mask[2], mask[3]});
    
    /* Use result */
    asm volatile("" : "+x"(result));
}

/* Strategy 5: Target-specific builtins when available */
#ifdef __AVX512F__
#include <immintrin.h>
static void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float src[64];
    __m512 result;
    
    /* Initialize source array */
    for (int i = 0; i < 64; i++) {
        src[i] = (float)i;
    }
    
    /* AVX-512 gather has many operands: base, scale, index, mask, etc. */
    result = _mm512_i32gather_ps(index, src, 4);
    
    /* Use result */
    asm volatile("" : "+x"(result));
}
#endif

/* Strategy 6: Mixed constraints in inline assembly */
static void test_mixed_constraints_asm(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t *mem_ptr = &i1;
    
    /* Mixed register, memory, and immediate constraints */
    asm volatile (
        "/* mixed constraints asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add (%3), %0\n\t"
        "add $10, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(mem_ptr), "r"(i4), "r"(i5)
        : "memory"
    );
    
    asm volatile("" : "+r"(o0));
}

/* Main function that calls all test patterns */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Execute all test patterns */
    test_vector_operations();
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    int constant_result = test_complex_constant_expr();
    printf("Constant expr result: %d\n", constant_result);
    
    test_vector_shuffle();
    test_mixed_constraints_asm();
    
    #ifdef __AVX512F__
    test_avx512_gather();
    printf("AVX-512 gather tested\n");
    #endif
    
    return 0;
}
