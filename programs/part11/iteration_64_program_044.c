/* test_optabs_10_11.c - Test program to cover 10/11 operand cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* ==================== Approach 1: Vector Shuffle with Many Elements ==================== */

/* Large vector types for shuffle operations */
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle with many indices - may expand to multi-operand pattern */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Store results to volatile memory */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    use((void*)result);
}

/* ==================== Approach 2: AVX-512 Gather Intrinsics ==================== */

#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    /* __mm512_i32gather_epi32 has many parameters during expansion */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // index
        (const void*)base,       // base
        4                        // scale
    );
    
    _mm512_store_epi32((void*)result, gathered);
    use((void*)result);
}
#endif

/* ==================== Approach 3: Atomic Operations ==================== */

__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(
        ptr,                     // ptr
        &expected,               // expected
        &desired,                // desired
        0,                       // weak
        __ATOMIC_SEQ_CST,        // success_memorder
        __ATOMIC_ACQUIRE         // failure_memorder
    );
    
    /* Another atomic with many operands */
    __atomic_fetch_add(ptr, success ? 10 : 20, __ATOMIC_RELAXED);
    
    /* Complex atomic exchange */
    int old = __atomic_exchange_n(ptr, 100, __ATOMIC_SEQ_CST);
    result[1] = old;
    use((void*)result);
}

/* ==================== Approach 4: OpenMP SIMD with Many Clauses ==================== */

__attribute__((noipa, noinline))
void test_omp_simd(volatile int* result) {
    const int N = 128;
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to multi-operand pattern */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32)
    for (int i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
    }
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = a[i];
    }
    use((void*)result);
}

/* ==================== Approach 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm: %0 = (%1 + %2) * (%3 + %4) + (%5 + %6) - (%7 + %8 + %9) */\n\t"
        "add %1, %2, %10\n\t"
        "add %3, %4, %11\n\t"
        "add %5, %6, %12\n\t"
        "add %7, %8, %13\n\t"
        "add %13, %9, %13\n\t"
        "mul %10, %11, %10\n\t"
        "add %10, %12, %10\n\t"
        "sub %10, %13, %0"
        : "=r"(out1), "+r"(a), "+r"(b), "+r"(c), "+r"(d), 
          "+r"(e), "+r"(f), "+r"(g), "+r"(h), "+r"(i)
        : "r"(j)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand asm */\n\t"
        "add %1, %2, %0\n\t"
        "add %0, %3, %0\n\t"
        "add %0, %4, %0\n\t"
        "add %0, %5, %0\n\t"
        "add %0, %6, %0\n\t"
        "add %0, %7, %0\n\t"
        "add %0, %8, %0\n\t"
        "add %0, %9, %0\n\t"
        "add %0, %10, %0\n\t"
        "add %0, %11, %0"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    
    result[0] = out1 + out2;
    use((void*)result);
}

/* ==================== Approach 6: Complex Vector Operations ==================== */

__attribute__((noipa, noinline))
void test_complex_vector_ops(volatile int* result) {
    /* Using GCC vector extensions with complex operations */
    typedef int v8si __attribute__((vector_size(32)));
    v8si v1 = {1,2,3,4,5,6,7,8};
    v8si v2 = {8,7,6,5,4,3,2,1};
    v8si v3 = {2,2,2,2,2,2,2,2};
    v8si v4 = {3,3,3,3,3,3,3,3};
    v8si v5 = {4,4,4,4,4,4,4,4};
    
    /* Complex expression that may expand to multi-operand pattern */
    v8si r = (v1 * v2) + (v3 * v4) - (v5 * v1) / v2 + (v1 << v3) | (v2 >> v4);
    
    /* Store results */
    for (int i = 0; i < 8; i++) {
        result[i] = r[i];
    }
    use((void*)result);
}

/* ==================== Main Function with Volatile Control Flow ==================== */

int main(int argc, char *argv[]) {
    /* Create volatile result arrays */
    volatile int results[6][32] = {{0}};
    
    /* Simple hash from argv[0] for volatile control */
    unsigned seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Call different test functions based on seed */
    switch (seed % 6) {
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
            test_atomic_ops(results[2]);
            break;
        case 3:
            test_omp_simd(results[3]);
            break;
        case 4:
            test_multi_operand_asm(results[4]);
            break;
        case 5:
            test_complex_vector_ops(results[5]);
            break;
    }
    
    /* Compute checksum to ensure all code contributes */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 32; j++) {
            checksum += results[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void *ptr) {
    /* Empty but referenced to keep data alive */
    (void)ptr;
}
