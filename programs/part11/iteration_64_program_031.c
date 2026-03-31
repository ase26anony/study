/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the 10 and 11 operand cases in GCC's optabs.cc
 * by using various patterns that expand to multi-operand instructions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Prevent interprocedural optimizations */
#define NOIPA __attribute__((noipa, noinline))

/* ========== Pattern 1: Vector shuffles with many elements ========== */

NOIPA void test_vector_shuffle(volatile int* result) {
    /* Use GCC vector extensions with shuffle operations */
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v16si __attribute__((vector_size(64)));
    
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle with many indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7,
        16, 17, 18, 19, 20, 21, 22, 23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 11, 10, 9, 8,
        31, 30, 29, 28, 27, 26, 25, 24);
    
    v16si e = c + d;
    
    /* Store result to volatile memory */
    for (int i = 0; i < 16; i++) {
        result[i] = e[i];
    }
    
    use(&e);
}

/* ========== Pattern 2: Target-specific built-ins (x86 AVX-512) ========== */

#ifdef __AVX512F__
#include <x86intrin.h>

NOIPA void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        base[i] = i;
    }
    
    /* Gather operation with scale - expands to many operands */
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    
    use(&gathered);
}
#endif

/* ========== Pattern 3: Atomic operations with many parameters ========== */

NOIPA void test_atomic_operations(volatile int* result) {
    volatile int shared = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0; /* Use strong version */
    
    /* __atomic_compare_exchange has many parameters */
    int success = __atomic_compare_exchange_n(&shared, &expected, desired,
                                              weak, __ATOMIC_SEQ_CST, 
                                              __ATOMIC_RELAXED);
    
    /* Another atomic with different ordering */
    int val = __atomic_exchange_n(&shared, 200, __ATOMIC_ACQ_REL);
    
    result[0] = success;
    result[1] = val;
    result[2] = shared;
    
    use(&shared);
}

/* ========== Pattern 4: OpenMP SIMD with complex clauses ========== */

NOIPA void test_openmp_simd(volatile int* result) {
    #define N 1024
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32) \
                    reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
        result[0] += a[i];
    }
    
    use(a);
    use(b);
    use(c);
}

/* ========== Pattern 5: Inline assembly with many operands ========== */

NOIPA void test_multi_operand_asm(volatile int* result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm template */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), 
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand asm template */\n\t"
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
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
    use(&out2);
}

/* ========== Pattern 6: Complex built-in with many arguments ========== */

NOIPA void test_complex_builtin(volatile int* result) {
    /* __builtin_prefetch with many arguments */
    int* ptr = (int*)result;
    
    /* Prefetch with all parameters specified */
    __builtin_prefetch(ptr, 0, 3, 0);  /* rw=0, locality=3, type=0 */
    __builtin_prefetch(ptr + 64, 1, 2, 1);
    __builtin_prefetch(ptr + 128, 0, 1, 0);
    
    /* Complex math built-in */
    double x = 2.0;
    double y = __builtin_pow(x, 3.14159);
    
    /* Sync built-in */
    __sync_synchronize();
    
    result[0] = (int)y;
    
    use(&y);
}

/* ========== Main function with runtime dispatch ========== */

int main(int argc, char* argv[]) {
    /* Create volatile result arrays */
    volatile int results[6][64] = {0};
    
    /* Simple hash from program name for runtime decision */
    unsigned int seed = 0;
    if (argc > 0 && argv[0]) {
        for (char* p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Execute different patterns based on seed */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            test_vector_shuffle(results[0]);
            break;
        case 1:
            #ifdef __AVX512F__
            test_avx512_gather(results[1]);
            #else
            test_vector_shuffle(results[1]);
            #endif
            break;
        case 2:
            test_atomic_operations(results[2]);
            break;
        case 3:
            test_openmp_simd(results[3]);
            break;
        case 4:
            test_multi_operand_asm(results[4]);
            break;
        case 5:
            test_complex_builtin(results[5]);
            break;
    }
    
    /* Compute checksum to ensure all code has effect */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 64; j++) {
            checksum ^= results[i][j];
        }
    }
    
    printf("Checksum: %d (pattern: %d)\n", checksum, pattern);
    
    return 0;
}

/* Dummy implementation of use() to satisfy linker */
void use(void* p) {
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(p) : "memory");
}
