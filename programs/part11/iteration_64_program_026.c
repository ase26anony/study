/* test_optabs_coverage.c
 * Designed to trigger 10/11 operand cases in GCC's optabs.cc
 * Compile with: gcc -O2 -mavx512f -mavx512vl -ftree-vectorize -fopenmp -fno-tree-slp-vectorize test.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Pattern 1: Vector shuffle with many operands ========== */
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

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
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    use(&c);
    use(&d);
}

/* ========== Pattern 2: AVX-512 gather intrinsics ========== */
#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsic(volatile int* result) {
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    /* Gather operation with many implicit operands */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  /* src */
        mask,                    /* mask */
        index,                   /* index */
        (void*)base,             /* base */
        4                        /* scale */
    );
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    use(&gathered);
}
#endif

/* ========== Pattern 3: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    volatile int shared = 42;
    int expected = 42;
    int desired = 84;
    int weak = 0;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange_n(
        &shared,                 /* ptr */
        &expected,               /* expected */
        desired,                 /* desired */
        weak,                    /* weak */
        __ATOMIC_SEQ_CST,        /* success_memorder */
        __ATOMIC_ACQUIRE         /* failure_memorder */
    );
    
    /* Another atomic operation */
    int old = __atomic_fetch_add(&shared, 10, __ATOMIC_RELAXED);
    
    result[0] = success;
    result[1] = old;
    result[2] = shared;
    use(&shared);
}

/* ========== Pattern 4: OpenMP SIMD with complex clauses ========== */
#define N 1024
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    int d[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(16) safelen(32) \
                reduction(+:result[0]) private(d)
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] * c[i];
        result[0] += d[i];
    }
    
    use(a); use(b); use(c); use(d);
}

/* ========== Pattern 5: Inline assembly with many operands ========== */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand assembly statement */
    asm volatile (
        "/* Multi-operand test %0 = (%1+%2)*(%3+%4) + (%5+%6)*(%7+%8) - %9 */\n\t"
        "addl %1, %2\n\t"
        "addl %3, %4\n\t"
        "imull %%eax, %%edx\n\t"
        "movl %%ecx, %0\n\t"
        "addl %5, %6\n\t"
        "addl %7, %8\n\t"
        "imull %%eax, %%edx\n\t"
        "addl %%ecx, %0\n\t"
        "subl %9, %0"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), 
          "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "eax", "edx", "ecx", "cc", "memory"
    );
    
    /* 11-operand assembly statement */
    asm volatile (
        "/* 11-operand test */\n\t"
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
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
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "eax", "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    use(&out1); use(&out2);
}

/* ========== Pattern 6: Complex vector operations ========== */
__attribute__((noipa, noinline))
void test_complex_vector_ops(volatile int* result) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
                8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Fused multiply-add style operations */
    v16sf v3 = v1 * v2 + __builtin_shufflevector(v1, v2, 
        0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30);
    
    /* Store as integers */
    v16si vi = __builtin_convertvector(v3, v16si);
    
    for (int i = 0; i < 16; i++) {
        result[i] = vi[i];
    }
    use(&v3); use(&vi);
}

/* ========== Main test driver ========== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a pseudo-random seed */
    unsigned seed = 0;
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    control = seed;
    
    /* Allocate result buffers */
    volatile int *results[6];
    for (int i = 0; i < 6; i++) {
        results[i] = (volatile int*)aligned_alloc(64, 64 * sizeof(int));
        memset((void*)results[i], 0, 64 * sizeof(int));
    }
    
    int checksum = 0;
    
    /* Execute different patterns based on seed */
    switch (control % 6) {
        case 0:
            test_vector_shuffle(results[0]);
            break;
        case 1:
#ifdef __AVX512F__
            test_gather_intrinsic(results[1]);
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
            test_complex_vector_ops(results[5]);
            break;
    }
    
    /* Compute checksum from all results */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 16 && j < 64; j++) {
            checksum += results[i][j];
        }
        free((void*)results[i]);
    }
    
    printf("Checksum: %d (seed: %u)\n", checksum, seed);
    return 0;
}
