/* test_optabs_coverage.c - Test program to cover 10/11 operand expansion cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Vector types for AVX/AVX-512 operations */
typedef double v4df __attribute__((vector_size(32)));  /* 4 doubles = 256 bits */
typedef float v8sf __attribute__((vector_size(32)));   /* 8 floats = 256 bits */
typedef float v16sf __attribute__((vector_size(64)));  /* 16 floats = 512 bits */
typedef int v8si __attribute__((vector_size(32)));     /* 8 ints = 256 bits */

/* Function to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test 1: Complex vector operations with FMA chaining */
void test_vector_operations() {
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Complex FMA chain - may generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    a = __builtin_fma(a, b, __builtin_fma(c, d, e));
    
    use(&a);
}

/* Test 2: Vector shuffle with large constant mask */
void test_vector_shuffle() {
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Large shuffle mask - 8 immediate operands */
    v8sf result = __builtin_shuffle(v1, v2, 
        (v8si){7, 6, 5, 4, 3, 2, 1, 0});
    
    /* Another shuffle with different mask */
    result = __builtin_shuffle(result, v1,
        (v8si){0, 1, 2, 3, 4, 5, 6, 7});
    
    use(&result);
}

/* Test 3: Inline assembly with exactly 10 operands */
void test_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
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
    
    use(&o0);
}

/* Test 4: Inline assembly with exactly 11 operands */
void test_asm_11_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
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
    
    use(&o0);
}

/* Test 5: Mixed constraints in inline assembly */
void test_asm_mixed_constraints() {
    int o0, o1, o2;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    const char *str = "test";
    
    /* Mix of register, memory, and immediate constraints */
    asm volatile (
        "movl $1, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "addl %4, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %5, %1\n\t"
        "movl %6, %2"
        : "=r"(o0), "=m"(o1), "=r"(o2)
        : "r"(i1), "i"(10), "m"(i2), "r"(i3)
        : "eax", "memory", "cc"
    );
    
    use(&o0); use(&o1); use(&o2); use(&str);
}

/* Test 6: Complex constant expression */
int test_complex_constant() {
    /* Large constant expression - may generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    x += 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20;
    
    /* Use __builtin_constant_p to force evaluation */
    if (__builtin_constant_p(x)) {
        x = x * 2;
    } else {
        x = x / 2;
    }
    
    return x;
}

/* Test 7: AVX-512 style operations (if supported) */
#ifdef __AVX512F__
void test_avx512_operations() {
    v16sf a = {0}, b = {0}, c = {0};
    v16sf mask = {0};
    
    /* Simulated gather operation - conceptually many operands */
    for (int i = 0; i < 16; i++) {
        a[i] = b[i] * c[i] + mask[i];
    }
    
    /* Complex permutation */
    v16sf result = __builtin_shuffle(a, b, 
        (__m512i){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
    
    use(&result);
}
#endif

/* Test 8: Template-like pattern using macros */
#define GEN_OPERATION(TYPE, N) \
    TYPE operation_##TYPE##_##N(TYPE a, TYPE b) { \
        return a + b + N + (N*2) + (N*3) + (N*4) + \
               (N*5) + (N*6) + (N*7) + (N*8) + (N*9); \
    }

/* Generate multiple instantiations */
GEN_OPERATION(int, 1)
GEN_OPERATION(int, 2)
GEN_OPERATION(int, 3)
GEN_OPERATION(float, 1)
GEN_OPERATION(float, 2)
GEN_OPERATION(double, 1)

/* Test 9: Memory operations with many addressing components */
void test_memory_operations() {
    struct Large {
        int data[20];
    } l1, l2;
    
    /* Complex memory copy with many field accesses */
    for (int i = 0; i < 10; i++) {
        l1.data[i] = l2.data[i] + l2.data[i+1] + 
                     l2.data[i+2] + l2.data[i+3];
    }
    
    use(&l1); use(&l2);
}

/* Main function that runs all tests */
int main() {
    printf("Testing optabs coverage for 10/11 operand expansions\n");
    
    /* Execute all tests */
    test_vector_operations();
    test_vector_shuffle();
    test_asm_10_operands();
    test_asm_11_operands();
    test_asm_mixed_constraints();
    
    int constant_result = test_complex_constant();
    printf("Constant test result: %d\n", constant_result);
    
    /* Test generated operations */
    int r1 = operation_int_1(1, 2);
    int r2 = operation_int_2(2, 3);
    int r3 = operation_int_3(3, 4);
    float r4 = operation_float_1(1.0f, 2.0f);
    float r5 = operation_float_2(2.0f, 3.0f);
    double r6 = operation_double_1(1.0, 2.0);
    
    printf("Template results: %d %d %d %.2f %.2f %.2f\n", 
           r1, r2, r3, r4, r5, r6);
    
    test_memory_operations();
    
#ifdef __AVX512F__
    test_avx512_operations();
    printf("AVX-512 tests executed\n");
#endif
    
    return 0;
}
