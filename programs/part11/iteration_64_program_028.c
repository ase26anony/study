/* test_optabs_10_11.c - Test program for GCC optabs 10/11 operand cases */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

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
        15, 14, 13, 12, 31, 30, 29, 28, 11, 10, 9, 8, 27, 26, 25, 24);
    
    /* Store results to volatile memory */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    
    use(&c);
    use(&d);
}

/* ==================== Approach 2: x86 AVX-512 Gather Intrinsics ==================== */

#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather operations can require many operands */
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xAAAA;  /* Alternating pattern */
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    /* Gather with scale 4 - expands to many operands */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // indices
        (const void*)base,       // base
        4                        // scale
    );
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    
    use(&gathered);
}
#endif

/* ==================== Approach 3: Atomic Operations ==================== */

__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    volatile _Atomic int atomic_var = 0;
    int expected = 0;
    int desired = 42;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange_n(
        &atomic_var,        // ptr
        &expected,          // expected
        desired,            // desired
        0,                  // weak
        __ATOMIC_SEQ_CST,   // success_memorder
        __ATOMIC_RELAXED    // failure_memorder
    );
    
    /* Another atomic operation with fetch_add */
    int old = __atomic_fetch_add(&atomic_var, 10, __ATOMIC_SEQ_CST);
    
    result[0] = success;
    result[1] = old;
    result[2] = atomic_var;
    
    use(&atomic_var);
}

/* ==================== Approach 4: OpenMP SIMD with Many Clauses ==================== */

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
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                simdlen(8) safelen(16) reduction(+:control)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * 3;
    }
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i];
    }
    
    use(a);
    use(b);
    use(c);
}

/* ==================== Approach 5: Inline Assembly with Many Operands ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
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
        "/* 11-operand asm */\n\t"
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
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
    use(&out2);
}

/* ==================== Approach 6: Complex Vector Operations ==================== */

typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_complex_vector_ops(volatile int* result) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
                 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Complex sequence of vector operations */
    v16sf v3 = v1 + v2;
    v16sf v4 = v1 * v2;
    v16sf v5 = __builtin_shufflevector(v3, v4, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Convert to integer for storage */
    v16si vi = __builtin_convertvector(v5, v16si);
    
    for (int i = 0; i < 16; i++) {
        result[i] = vi[i];
    }
    
    use(&v3);
    use(&v4);
    use(&v5);
}

/* ==================== Main Function ==================== */

int main(int argc, char* argv[]) {
    /* Use argv[0] to create a simple hash for control flow */
    unsigned long seed = 0;
    if (argv[0]) {
        for (char* p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Allocate result buffers */
    volatile int results[6][32] = {{0}};
    
    /* Execute different test functions based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
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
            test_openmp_simd(results[3]);
            break;
        case 4:
            test_multi_operand_asm(results[4]);
            break;
        case 5:
            test_complex_vector_ops(results[5]);
            break;
    }
    
    /* Compute checksum to ensure all code executes */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 32; j++) {
            checksum += results[i][j];
        }
    }
    
    printf("Checksum: %d (test case: %d)\n", checksum, test_case);
    
    return 0;
}

/* Dummy implementation of use() to satisfy linker */
void use(void* ptr) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
