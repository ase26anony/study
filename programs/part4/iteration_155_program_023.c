/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector types for different architectures */
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

/* Strategy 1: Complex vector operations with FMA chaining */
#ifdef __FMA__
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    a = __builtin_fma(a, b, __builtin_fma(c, d, __builtin_fma(e, a, b)));
    
    volatile v8df sink = a;
    (void)sink;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10/11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 10 operand asm statement */
    asm volatile (
        "/* dummy 10-operand instruction %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    /* 11 operand asm statement */
    asm volatile (
        "/* dummy 11-operand instruction */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
          "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10)
        : "cc"
    );
    
    volatile int64_t sink = o0;
    (void)sink;
}

/* Strategy 3: Complex constant expression that may not fold immediately */
static int test_complex_const_expression(void) {
    /* Use __builtin_constant_p to potentially prevent early folding */
    int x = 0;
    
    if (__builtin_constant_p(0)) {
        /* This expression has 11 constants */
        x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        /* Force compiler to consider both paths */
        x = 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20 + 21 + 22;
    }
    
    /* Chain more operations to increase operand count */
    x = x + (1 << 2) + (3 << 4) + (5 << 6) + (7 << 8) + 
            (9 << 10) + (11 << 12) + (13 << 14) + (15 << 16) +
            (17 << 18) + (19 << 20);
    
    return x;
}

/* Strategy 4: Vector shuffle with large constant mask */
#ifdef __AVX512F__
static void test_vector_shuffle_large_mask(void) {
    v16sf v1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16sf v2 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with a 16-element mask - each element is an immediate */
    int mask[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    
    /* Use __builtin_shuffle with a large mask */
    v16sf result = __builtin_shuffle(v1, v2, mask[0], mask[1], mask[2], mask[3],
                                     mask[4], mask[5], mask[6], mask[7],
                                     mask[8], mask[9], mask[10], mask[11],
                                     mask[12], mask[13], mask[14], mask[15]);
    
    volatile v16sf sink = result;
    (void)sink;
}
#endif

/* Strategy 5: Mixed constraints in inline assembly */
static void test_mixed_constraints_asm(void) {
    int out1, out2;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int *mem_ptr = &in1;
    
    /* Mix register, memory, and immediate constraints */
    asm volatile (
        "/* Mixed constraints with 10 operands */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(out1), "=m"(*mem_ptr)
        : "r"(in1), "r"(in2), "r"(in3), "i"(100), 
          "r"(in5), "m"(in6), "r"(in7), "i"(200)
        : "cc", "memory"
    );
    
    volatile int sink = out1 + *mem_ptr;
    (void)sink;
}

/* Strategy 6: Use target-specific builtins when available */
#ifdef __AVX512F__
#include <immintrin.h>
static void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    __m512 result;
    
    /* AVX-512 gather has multiple operands: base, scale, index, mask */
    result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, index, 
                                     base, 4);
    
    volatile __m512 sink = result;
    (void)sink;
}
#endif

/* Main function that calls all test patterns */
int main(void) {
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Test 1: Vector FMA chaining */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
    /* Test 2: Inline assembly with exactly 10/11 operands */
    test_inline_asm_10_operands();
    
    /* Test 3: Complex constant expressions */
    int x = test_complex_const_expression();
    volatile int sink1 = x;
    (void)sink1;
    
    /* Test 4: Vector shuffle with large mask */
#ifdef __AVX512F__
    test_vector_shuffle_large_mask();
#endif
    
    /* Test 5: Mixed constraints inline assembly */
    test_mixed_constraints_asm();
    
    /* Test 6: Target-specific builtins */
#ifdef __AVX512F__
    test_avx512_gather();
#endif
    
    printf("Tests completed.\n");
    return 0;
}
