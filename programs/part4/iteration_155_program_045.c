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
    
    /* Complex FMA chain that might generate multi-operand RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    volatile v8df result = a;
    (void)result;
#endif
}
#endif

/* Strategy 2: Vector permutations with large masks */
static void test_vector_permutations(void) {
#ifdef __AVX__
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex shuffle with 16-element mask (8 from each vector) */
    v8sf shuffled = __builtin_shufflevector(v1, v2, 
        0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);
    volatile v8sf result = shuffled;
    (void)result;
#endif
}

/* Strategy 3: Inline assembly with exactly 10 and 11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "# 10-operand asm\n\t"
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "mov %9, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9)
        : "cc"
    );
    
    volatile int64_t result = o0;
    (void)result;
}

static void test_inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "# 11-operand asm\n\t"
        "lea (%1,%2), %%rax\n\t"
        "add %3, %%rax\n\t"
        "add %4, %%rax\n\t"
        "add %5, %%rax\n\t"
        "add %6, %%rax\n\t"
        "add %7, %%rax\n\t"
        "add %8, %%rax\n\t"
        "add %9, %%rax\n\t"
        "add %10, %%rax\n\t"
        "mov %%rax, %0"
        : "=r"(o0)
        : "r"(i1), "r"(i2), "r"(i3), "r"(i4), 
          "r"(i5), "r"(i6), "r"(i7), "r"(i8), 
          "r"(i9), "r"(i10)
        : "rax", "cc"
    );
    
    volatile int64_t result = o0;
    (void)result;
}

/* Strategy 4: Complex constant expressions */
static void test_complex_const_expressions(void) {
    /* Large constant expression that might not fold immediately */
    const int x = 1 + (2 * 3) + (4 << 5) + (6 & 7) + (8 | 9) + 
                  (10 ^ 11) + (12 / 13) + (14 % 15) + (16 - 17) + 
                  (18 == 19) + (20 != 21);
    
    /* Force compiler to consider both branches */
    if (__builtin_constant_p(x)) {
        volatile int result = x + 100;
        (void)result;
    } else {
        volatile int result = x + 200;
        (void)result;
    }
}

/* Strategy 5: Target-specific builtins for x86 */
#ifdef __x86_64__
#include <x86intrin.h>

static void test_avx512_gather(void) {
#ifdef __AVX512F__
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    float base[64] = {0};
    
    /* AVX512 gather has multiple implicit operands */
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    volatile __m512 vresult = result;
    (void)vresult;
#endif
}
#endif

/* Strategy 6: Mixed register/memory constraints in inline asm */
static void test_mixed_constraint_asm(void) {
    int64_t o0, o1, o2;
    int64_t i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    int64_t mem1 = 100, mem2 = 200;
    
    /* Mix of register and memory constraints */
    asm volatile (
        "# Mixed constraints asm\n\t"
        "mov %5, %%rax\n\t"
        "add %6, %%rax\n\t"
        "add %7, %%rax\n\t"
        "add %8, %%rax\n\t"
        "mov %%rax, %0\n\t"
        "mov %%rax, %1\n\t"
        "mov %%rax, %2"
        : "=r"(o0), "=m"(*(int64_t*)&mem1), "=r"(o2)
        : "r"(i1), "r"(i2), "r"(i3), "m"(mem2), "r"(i4)
        : "rax", "cc", "memory"
    );
    
    volatile int64_t result = o0 + o1 + o2;
    (void)result;
}

/* Strategy 7: Template-like approach using macros */
#define GENERATE_VECTOR_OP(TYPE, SUFFIX) \
    static void test_vector_##SUFFIX(void) { \
        TYPE v1, v2, v3, v4; \
        for (int i = 0; i < sizeof(v1)/sizeof(v1[0]); i++) { \
            v1[i] = i + 1; \
            v2[i] = i + 2; \
            v3[i] = i + 3; \
            v4[i] = i + 4; \
        } \
        /* Complex expression */ \
        v1 = v1 + v2 * v3 - v4 / (TYPE){2}; \
        volatile TYPE result = v1; \
        (void)result; \
    }

#ifdef __SSE2__
GENERATE_VECTOR_OP(v4sf, v4sf)
GENERATE_VECTOR_OP(v2df, v2df)
#endif

#ifdef __AVX__
GENERATE_VECTOR_OP(v8sf, v8sf)
GENERATE_VECTOR_OP(v4df, v4df)
#endif

/* Main function that calls all tests */
int main(void) {
    printf("Testing 10/11 operand expansion coverage...\n");
    
    /* Test inline assembly with exact operand counts */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test mixed constraint assembly */
    test_mixed_constraint_asm();
    
    /* Test complex constant expressions */
    test_complex_const_expressions();
    
    /* Test vector operations if supported */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
    test_vector_permutations();
    
#ifdef __SSE2__
    test_vector_v4sf();
    test_vector_v2df();
#endif
    
#ifdef __AVX__
    test_vector_v8sf();
    test_vector_v4df();
#endif
    
#ifdef __x86_64__
#ifdef __AVX512F__
    test_avx512_gather();
#endif
#endif
    
    printf("Tests completed.\n");
    return 0;
}
