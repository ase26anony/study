/* test_optabs_10_11.c - Test program to cover 10/11 operand cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Approach 1: Vector shuffles with many operands ========== */

typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices = 2 vectors + 16 indices = 18 operands total */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Store results */
    memcpy((void*)result, &c, sizeof(c));
    memcpy((void*)(result + 16), &d, sizeof(d));
    
    use((void*)result);
}

/* ========== Approach 2: x86-specific gather intrinsics ========== */

#ifdef __x86_64__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsics(volatile double* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512d src = _mm512_set1_pd(2.0);
    __m512i index = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __mmask8 mask = 0xFF;
    
    double base[64] __attribute__((aligned(64)));
    for (int i = 0; i < 64; i++) base[i] = i * 0.5;
    
    /* Gather with scale 8 - expands to many operands */
    __m512d gathered = _mm512_i64gather_pd(index, base, 8);
    
    /* Store result */
    _mm512_store_pd((void*)result, gathered);
    
    use((void*)result);
}
#endif

/* ========== Approach 3: Atomic operations with many parameters ========== */

__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange has many parameters that expand to operands */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Another atomic with different ordering */
    int val = __atomic_fetch_add(ptr, 10, __ATOMIC_ACQ_REL);
    
    /* Store results */
    result[1] = success;
    result[2] = val;
    result[3] = expected;
    
    use((void*)result);
}

/* ========== Approach 4: OpenMP SIMD with complex clauses ========== */

#define N 128
__attribute__((noipa, noinline))
void test_openmp_simd(volatile float* result) {
    float a[N] __attribute__((aligned(64)));
    float b[N] __attribute__((aligned(64)));
    float c[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
    }
    
    /* OpenMP SIMD with multiple clauses - expands to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * 2.0f;
    }
    
    /* Copy to volatile result */
    for (int i = 0; i < N; i++) {
        result[i] = c[i];
    }
    
    use((void*)result);
}

/* ========== Approach 5: Inline assembly with many operands ========== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 = (%1 + %2) * (%3 + %4) + (%5 * %6) - (%7 + %8 + %9) */\n\t"
        "addl %3, %4\n\t"
        "addl %1, %2\n\t"
        "imull %4, %2\n\t"
        "movl %2, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "subl %7, %0\n\t"
        "subl %8, %0\n\t"
        "subl %9, %0"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand test */\n\t"
        "leal (%1,%2), %%eax\n\t"
        "addl %3, %%eax\n\t"
        "addl %4, %%eax\n\t"
        "addl %5, %%eax\n\t"
        "addl %6, %%eax\n\t"
        "addl %7, %%eax\n\t"
        "addl %8, %%eax\n\t"
        "addl %9, %%eax\n\t"
        "addl %10, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "eax", "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use((void*)result);
}

/* ========== Approach 6: Complex built-in with many arguments ========== */

__attribute__((noipa, noinline))
void test_complex_builtin(volatile int* result) {
    /* __builtin_add_overflow can generate multi-operand patterns */
    int a = 1000, b = 2000, c = 3000, d = 4000;
    int sum1, sum2, sum3, sum4;
    int ov1, ov2, ov3, ov4;
    
    ov1 = __builtin_add_overflow(a, b, &sum1);
    ov2 = __builtin_add_overflow(sum1, c, &sum2);
    ov3 = __builtin_add_overflow(sum2, d, &sum3);
    ov4 = __builtin_add_overflow(sum3, a, &sum4);
    
    /* Chain of operations that might expand to multi-operand pattern */
    result[0] = sum1;
    result[1] = sum2;
    result[2] = sum3;
    result[3] = sum4;
    result[4] = ov1 | ov2 | ov3 | ov4;
    
    use((void*)result);
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[]) {
    /* Create a hash from argv[0] for volatile control */
    unsigned int seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Allocate result buffers */
    volatile int* int_results = malloc(256 * sizeof(int));
    volatile float* float_results = malloc(256 * sizeof(float));
    volatile double* double_results = malloc(256 * sizeof(double));
    
    if (!int_results || !float_results || !double_results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some values */
    for (int i = 0; i < 256; i++) {
        if (i < 256) int_results[i] = i;
        if (i < 256) float_results[i] = i * 0.5f;
        if (i < 256) double_results[i] = i * 0.25;
    }
    
    /* Execute tests based on seed to ensure all paths are considered */
    int test_selector = seed % 6;
    
    switch (test_selector) {
        case 0:
            test_vector_shuffle(int_results);
            break;
        case 1:
#ifdef __x86_64__
            test_gather_intrinsics(double_results);
#endif
            break;
        case 2:
            test_atomic_operations(int_results);
            break;
        case 3:
            test_openmp_simd(float_results);
            break;
        case 4:
            test_multi_operand_asm(int_results);
            break;
        case 5:
            test_complex_builtin(int_results);
            break;
    }
    
    /* Force execution of all tests in optimized builds by using control */
    if (control == 0) {
        test_vector_shuffle(int_results + 64);
#ifdef __x86_64__
        test_gather_intrinsics(double_results + 64);
#endif
        test_atomic_operations(int_results + 128);
        test_openmp_simd(float_results + 128);
        test_multi_operand_asm(int_results + 192);
        test_complex_builtin(int_results + 224);
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= int_results[i];
        checksum += (int)(float_results[i] * 1000);
    }
    
    printf("Checksum: %d\n", checksum);
    
    free((void*)int_results);
    free((void*)float_results);
    free((void*)double_results);
    
    return 0;
}
