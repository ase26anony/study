/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's optabs expansion for 10 and 11 operand
 * instruction patterns, covering the uncovered lines in optabs.cc.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* ==================== Approach 1: Vector Shuffle with Many Elements ==================== */

typedef int v16si __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle requiring many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 11, 10, 9, 8, 31, 30, 29, 28, 27, 26, 25, 24);
    
    /* Use results to prevent elimination */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    use((void*)result);
}

/* ==================== Approach 2: x86 AVX-512 Gather Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    /* Gather with scale 4 - expands to many operands */
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    use((void*)result);
}
#endif

/* ==================== Approach 3: Atomic Operations ==================== */

__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* Atomic compare exchange with multiple parameters */
    /* This expands to complex RTL with many operands on some targets */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, /* weak */
                                            __ATOMIC_SEQ_CST, 
                                            __ATOMIC_ACQUIRE);
    
    /* Another atomic operation with many parameters */
    int old = __atomic_fetch_add(ptr, 10, __ATOMIC_RELAXED);
    
    result[1] = success + old;
    use((void*)result);
}

/* ==================== Approach 4: OpenMP SIMD with Many Clauses ==================== */

__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    int N = 64;
    int a[64] __attribute__((aligned(64)));
    int b[64] __attribute__((aligned(64)));
    int c[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                simdlen(8) safelen(16) collapse(1) \
                private(i) lastprivate(i)
    for (int i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
    }
    
    /* Copy to volatile result */
    for (int i = 0; i < 16; i++) {
        result[i] = a[i];
    }
    use((void*)result);
}

/* ==================== Approach 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
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
        "/* 11-operand test */\n\t"
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
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    result[0] = out1 + out2;
    use((void*)result);
}

/* ==================== Approach 6: Complex Vector Operations ==================== */

typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_complex_vector_ops(volatile int* result) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f,
                 8.5f, 9.5f, 10.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f};
    v16sf v3 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f,
                 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    
    /* Complex FMA-like operation that might expand to many operands */
    v16sf res = v1 * v2 + v3;
    
    /* Convert to int and store */
    for (int i = 0; i < 16; i++) {
        result[i] = (int)res[i];
    }
    use((void*)result);
}

/* ==================== Main Function with Volatile Control Flow ==================== */

int main(int argc, char *argv[]) {
    volatile int seed = 0;
    volatile int result[64] = {0};
    
    /* Create a simple hash from argv[0] for seed */
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Execute different test functions based on seed */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle(result);
            break;
        case 1:
#ifdef __AVX512F__
            test_avx512_gather(result);
#else
            test_vector_shuffle(result);
#endif
            break;
        case 2:
            test_atomic_operations(result);
            break;
        case 3:
            test_openmp_simd(result);
            break;
        case 4:
            test_multi_operand_asm(result);
            break;
        case 5:
            test_complex_vector_ops(result);
            break;
    }
    
    /* Compute checksum to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += result[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
