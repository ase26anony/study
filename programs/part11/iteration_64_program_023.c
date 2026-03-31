/* test_optabs_10_11.c - Test program to cover 10/11 operand cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Approach 1: Vector shuffles with many elements ========== */
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices = 2 vectors + 16 indices = 18 operands in expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    use(&c);
    use(&d);
}

/* ========== Approach 2: x86-specific gather intrinsics ========== */
#ifdef __x86_64__
#include <x86intrin.h>

__attribute__((noipa, noinline, target("avx512f")))
void test_gather_intrinsics(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i;
    
    /* Gather with scale 4 - expands to many operands */
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    use(&gathered);
}

/* AVX-512 masked scatter */
__attribute__((noipa, noinline, target("avx512f")))
void test_scatter_intrinsics(volatile int* result) {
    __m512i index = _mm512_set_epi32(7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8);
    __m512i values = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __mmask16 mask = 0xAAAA;
    
    /* Scatter with many operands */
    _mm512_mask_i32scatter_epi32(result, mask, index, values, 4);
    use(&values);
}
#endif

/* ========== Approach 3: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange has many parameters that expand to multiple operands */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Another atomic with different memory orders */
    __atomic_store_n(ptr + 1, desired, __ATOMIC_RELEASE);
    
    /* Atomic exchange with many parameters in expansion */
    int old = __atomic_exchange_n(ptr + 2, desired * 2, __ATOMIC_ACQ_REL);
    
    result[3] = success + old;
    use(&success);
}

/* ========== Approach 4: OpenMP SIMD with many clauses ========== */
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result, int n) {
    int a[256] __attribute__((aligned(64)));
    int b[256] __attribute__((aligned(64)));
    int c[256] __attribute__((aligned(64)));
    
    for (int i = 0; i < 256; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses - expands to many operands */
    #pragma omp simd aligned(a,b,c:64) linear(i:1) safelen(64) simdlen(16)
    for (int i = 0; i < n && i < 256; i++) {
        c[i] = a[i] + b[i] * 3;
    }
    
    /* Another SIMD loop with reduction */
    int sum = 0;
    #pragma omp simd reduction(+:sum) aligned(c:64)
    for (int i = 0; i < n && i < 256; i++) {
        sum += c[i];
    }
    
    result[0] = sum;
    use(&c);
}

/* ========== Approach 5: Inline assembly with many operands ========== */
__attribute__((noipa, noinline))
void test_many_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand template %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* Another multi-operand template */\n\t"
        "mov %0, %1\n\t"
        "imul %0, %2\n\t"
        "add %0, %3\n\t"
        "sub %0, %4\n\t"
        "and %0, %5\n\t"
        "or %0, %6\n\t"
        "xor %0, %7\n\t"
        "add %0, %8\n\t"
        "sub %0, %9\n\t"
        "add %0, %10"
        : "=r"(out2)
        : "r"(out1), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    result[0] = out1 + out2;
    use(&out1);
    use(&out2);
}

/* ========== Approach 6: Complex builtin combinations ========== */
__attribute__((noipa, noinline))
void test_complex_builtins(volatile int* result) {
    /* __builtin_constant_p with many arguments in a complex expression */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Complex expression that might expand to many operands */
    int complex_result = 
        __builtin_constant_p(a) * b +
        __builtin_constant_p(c) * d +
        __builtin_constant_p(e) * f +
        __builtin_constant_p(g) * h +
        __builtin_constant_p(i) * j;
    
    /* __builtin_expect with many branches */
    if (__builtin_expect(complex_result > 100, 0)) {
        result[0] = __builtin_popcount(complex_result);
    } else {
        result[0] = __builtin_clz(complex_result);
    }
    
    /* Memory builtins with many parameters */
    char src[64] = "Test string for memory operations";
    char dst[64];
    
    __builtin_memcpy(dst, src, sizeof(src));
    __builtin_memset(dst + 32, 0, 16);
    
    result[1] = __builtin_strlen(dst);
    use(&complex_result);
    use(dst);
}

/* ========== Main test driver ========== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a simple hash for control flow */
    unsigned seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Allocate result arrays */
    volatile int results[6][256] = {0};
    
    /* Execute different test patterns based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0:
            test_vector_shuffle(results[0]);
            break;
        case 1:
#ifdef __x86_64__
            test_gather_intrinsics(results[1]);
#else
            test_vector_shuffle(results[1]);
#endif
            break;
        case 2:
            test_atomic_operations(results[2]);
            break;
        case 3:
            test_openmp_simd(results[3], 128);
            break;
        case 4:
            test_many_operand_asm(results[4]);
            break;
        case 5:
            test_complex_builtins(results[5]);
            break;
    }
    
    /* Also call some tests unconditionally to ensure coverage */
    if (control == 0) {
        test_vector_shuffle(results[0]);
#ifdef __x86_64__
        test_scatter_intrinsics(results[1]);
#endif
        test_atomic_operations(results[2]);
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 256; j++) {
            checksum ^= results[i][j];
        }
    }
    
    printf("Checksum: %d (seed: %u, test_case: %d)\n", checksum, seed, test_case);
    
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void* ptr) {
    /* Empty but referenced to keep variables alive */
    (void)ptr;
}
