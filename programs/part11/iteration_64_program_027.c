/* Test program to cover 10/11 operand cases in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(void *);

/* Prevent interprocedural optimizations */
#define NOIPA __attribute__((noipa, noinline))

/* Vector types for shuffle operations */
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

/* Test 1: Large vector shuffle with many indices */
NOIPA void test_vector_shuffle(volatile int *result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Store result */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i];
    }
    use((void*)result);
}

/* Test 2: Atomic compare exchange with multiple parameters */
NOIPA void test_atomic_ops(volatile int *result) {
    int expected = 42;
    int desired = 100;
    int *ptr = (int*)result;
    
    /* __atomic_compare_exchange has many parameters */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                           0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    result[1] = success;
    result[2] = expected;
    use((void*)result);
}

/* Test 3: OpenMP SIMD with multiple clauses */
NOIPA void test_openmp_simd(volatile int *result, int n) {
    int a[64] __attribute__((aligned(64)));
    int b[64] __attribute__((aligned(64)));
    int c[64] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* Complex OpenMP SIMD pragma with multiple clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16)
    for (int i = 0; i < n && i < 64; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Copy to result */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i];
    }
    use((void*)result);
}

/* Test 4: Inline assembly with many operands */
NOIPA void test_many_operand_asm(volatile int *result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Custom 10-operand operation */\n\t"
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
        "/* Custom 11-operand operation */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    use((void*)result);
}

/* Test 5: AVX-512 gather intrinsic (if available) */
#ifdef __AVX512F__
#include <immintrin.h>
NOIPA void test_avx512_gather(volatile int *result) {
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        base[i] = i * 3;
    }
    
    /* __mm512_i32gather_epi32 has many implicit operands */
    __m512i gathered = _mm512_i32gather_epi32(index, (void*)base, 4);
    
    _mm512_store_epi32((void*)result, gathered);
    use((void*)result);
}
#endif

/* Test 6: Complex builtin with many arguments */
NOIPA void test_complex_builtin(volatile int *result) {
    /* __builtin___clear_cache takes two pointer arguments */
    char buffer[64];
    __builtin___clear_cache(buffer, buffer + 64);
    
    /* Complex math builtins can have many operands */
    double x = 1.5;
    double y = __builtin_pow(x, 3.0);
    
    /* Use sincos which returns two values */
    double sin_val, cos_val;
    __builtin_sincos(x, &sin_val, &cos_val);
    
    result[0] = (int)(y * 100);
    result[1] = (int)(sin_val * 100);
    result[2] = (int)(cos_val * 100);
    use((void*)result);
}

/* Main function with volatile control flow */
int main(int argc, char *argv[]) {
    volatile int seed = 0;
    volatile int results[128];
    
    /* Create a seed from argv[0] */
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize results array */
    memset((void*)results, 0, sizeof(results));
    
    /* Execute different tests based on seed */
    int test_choice = (seed & 0x7);  /* 0-7 */
    
    switch (test_choice) {
        case 0:
            test_vector_shuffle(results);
            break;
        case 1:
            test_atomic_ops(results);
            break;
        case 2:
            test_openmp_simd(results, 32);
            break;
        case 3:
            test_many_operand_asm(results);
            break;
        case 4:
            test_complex_builtin(results);
            break;
        #ifdef __AVX512F__
        case 5:
            test_avx512_gather(results);
            break;
        #endif
        default:
            /* Run multiple tests */
            test_vector_shuffle(results);
            test_atomic_ops(results + 16);
            test_many_operand_asm(results + 32);
            break;
    }
    
    /* Compute simple checksum */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
