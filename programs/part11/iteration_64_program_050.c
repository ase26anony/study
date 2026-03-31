/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the 10 and 11 operand cases in optabs.cc
 * by using various GCC features that expand to multi-operand RTL patterns.
 */

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
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle with many indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 31, 30, 29, 28, 11, 10, 9, 8, 27, 26, 25, 24);
    
    /* Use both results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    
    use(&c);
    use(&d);
}

/* ========== Approach 2: AVX-512 gather intrinsics (if available) ========== */
#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather operations can have many operands */
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // indices
        (void*)base,             // base pointer
        4                        // scale
    );
    
    _mm512_store_epi32((void*)result, gathered);
    use(&gathered);
}
#endif

/* ========== Approach 3: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    _Atomic(int) atomic_var = 0;
    int expected = 0;
    int desired = 42;
    int weak = 0;  /* Use weak compare-exchange */
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange_n(
        &atomic_var,        // ptr
        &expected,          // expected
        desired,            // desired
        weak,               // weak
        __ATOMIC_SEQ_CST,   // success_memorder
        __ATOMIC_ACQUIRE    // failure_memorder
    );
    
    /* Another atomic operation with fetch_add and memory order */
    int old = __atomic_fetch_add(&atomic_var, 10, __ATOMIC_RELAXED);
    
    result[0] = success;
    result[1] = atomic_var;
    result[2] = old;
    
    use(&atomic_var);
}

/* ========== Approach 4: OpenMP SIMD with many clauses ========== */
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    #define N 128
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd aligned(a, b, c: 64) linear(i:1) safelen(32) simdlen(8)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * 3;
    }
    
    /* Complex reduction */
    int sum = 0;
    #pragma omp simd reduction(+:sum) aligned(c:64)
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    result[0] = sum;
    result[1] = c[N-1];
    
    use(c);
}

/* ========== Approach 5: Inline assembly with many operands ========== */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm: %0 = (%1 + %2) * (%3 + %4) - (%5 * %6) + (%7 - %8) / %9 */\n\t"
        "addl %1, %2\n\t"
        "addl %3, %4\n\t"
        "imull %2, %4\n\t"
        "movl %4, %0\n\t"
        "imull %5, %6\n\t"
        "subl %6, %0\n\t"
        "subl %8, %7\n\t"
        "cltd\n\t"
        "idivl %9\n\t"
        "addl %7, %0"
        : "=&r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc", "eax", "edx"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand asm */\n\t"
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
    
    use(&out1);
    use(&out2);
}

/* ========== Approach 6: Complex builtin with many arguments ========== */
__attribute__((noipa, noinline))
void test_complex_builtin(volatile int* result) {
    /* __builtin_add_overflow with multiple accumulations */
    int a = 1000, b = 2000, c = 3000, d = 4000;
    int sum1, sum2, sum3, sum4;
    int of1, of2, of3, of4;
    
    /* Chain of overflow checks - may expand to complex pattern */
    of1 = __builtin_add_overflow(a, b, &sum1);
    of2 = __builtin_add_overflow(sum1, c, &sum2);
    of3 = __builtin_add_overflow(sum2, d, &sum3);
    of4 = __builtin_add_overflow(sum3, a, &sum4);
    
    /* __builtin_mul_overflow with many operands */
    int m1, m2, m3;
    int mof1, mof2, mof3;
    
    mof1 = __builtin_mul_overflow(a, b, &m1);
    mof2 = __builtin_mul_overflow(m1, c, &m2);
    mof3 = __builtin_mul_overflow(m2, d, &m3);
    
    result[0] = sum4;
    result[1] = m3;
    result[2] = of1 | of2 | of3 | of4;
    result[3] = mof1 | mof2 | mof3;
    
    use(&sum4);
    use(&m3);
}

/* ========== Main function with runtime dispatch ========== */
int main(int argc, char *argv[]) {
    /* Create volatile result arrays */
    volatile int results[6][64] = {{0}};
    
    /* Simple hash from program name for control flow */
    unsigned int seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Execute different tests based on seed */
    int test_num = seed % 6;
    
    switch (test_num) {
        case 0:
            test_vector_shuffle(results[0]);
            printf("Vector shuffle test: %d\n", results[0][0]);
            break;
            
        case 1:
#ifdef __AVX512F__
            test_avx512_gather(results[1]);
            printf("AVX-512 gather test: %d\n", results[1][0]);
#else
            printf("AVX-512 not available, skipping\n");
#endif
            break;
            
        case 2:
            test_atomic_ops(results[2]);
            printf("Atomic ops test: %d %d %d\n", 
                   results[2][0], results[2][1], results[2][2]);
            break;
            
        case 3:
            test_openmp_simd(results[3]);
            printf("OpenMP SIMD test: %d %d\n", results[3][0], results[3][1]);
            break;
            
        case 4:
            test_multi_operand_asm(results[4]);
            printf("Multi-operand asm test: %d %d\n", results[4][0], results[4][1]);
            break;
            
        case 5:
            test_complex_builtin(results[5]);
            printf("Complex builtin test: %d %d %d %d\n",
                   results[5][0], results[5][1], results[5][2], results[5][3]);
            break;
    }
    
    /* Compute checksum of all results to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 64; j++) {
            checksum ^= results[i][j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}

/* Dummy implementation of use() to satisfy linker */
void use(void *ptr) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
