/* test_optabs_10_11_operands.c
 * Test program to cover 10/11 operand cases in optabs.cc
 * Compile with: gcc -O2 -mavx512f -mavx512vl -fopenmp -ftree-vectorize -fno-tree-slp-vectorize test.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Strategy 1: Large vector shuffle with many operands */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with explicit indices - requires many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    use((void*)result);
}
#endif

/* Strategy 2: AVX-512 gather intrinsic (likely needs many operands) */
#ifdef __AVX512F__
__attribute__((noipa, noinline))
void test_gather_intrinsic(volatile int* result) {
    double base[64];
    int32_t indices[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    v8df src = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    __mmask8 mask = 0xFF;
    
    /* Initialize base array */
    for (int i = 0; i < 64; i++) {
        base[i] = i * 1.5;
    }
    
    /* Use gather intrinsic - expands to many operands */
    v8df res = __builtin_ia32_gathersiv8df(src, (const double*)base,
                                          (__v8si)indices, mask, 1);
    
    /* Store results */
    for (int i = 0; i < 8; i++) {
        result[i] = (int)res[i];
    }
    use((void*)result);
}
#endif

/* Strategy 3: Atomic compare-exchange with many parameters */
__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    int success = 0;
    
    /* __atomic_compare_exchange with many parameters */
    for (int i = 0; i < 4; i++) {
        expected = atomic_var;
        desired = expected + 10;
        success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
        result[i] = success ? atomic_var : -1;
    }
    use((void*)result);
}

/* Strategy 4: OpenMP SIMD with many clauses */
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result, int n) {
    int a[128], b[128], c[128];
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                    reduction(+:result[0]) if(n > 100)
    for (int i = 0; i < 128; i++) {
        a[i] = b[i] * c[i] + i;
        result[0] += a[i];
    }
    
    /* Additional computation to ensure expansion */
    #pragma omp simd collapse(2) simdlen(4)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            result[idx % 16] += a[idx] - b[idx];
        }
    }
    use((void*)result);
}

/* Strategy 5: Inline assembly with many operands */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2, out3, out4, out5;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand test %0 = (%1 + %2) * (%3 + %4) + (%5 * %6) - (%7 + %8 + %9) */\n\t"
        "addl %1, %2\n\t"
        "addl %3, %4\n\t"
        "imull %%eax, %%edx\n\t"
        "movl %5, %%eax\n\t"
        "imull %6, %%eax\n\t"
        "addl %%eax, %%edx\n\t"
        "movl %7, %%eax\n\t"
        "addl %8, %%eax\n\t"
        "addl %9, %%eax\n\t"
        "subl %%eax, %%edx\n\t"
        "movl %%edx, %0"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "eax", "edx", "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
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
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j)
        : "eax", "cc"
    );
    
    result[0] = out1 + out2;
    use((void*)result);
}

/* Strategy 6: Complex builtin with many arguments */
#ifdef __AVX512F__
__attribute__((noipa, noinline))
void test_complex_builtin(volatile int* result) {
    /* Use permute builtin which may need many operands */
    v16si v1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si v2 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex permutation pattern */
    __m512i idx = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    v16si permuted = _mm512_permutex2var_epi32(v1, idx, v2);
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = permuted[i];
    }
    use((void*)result);
}
#endif

/* Main function with volatile control flow */
int main(int argc, char *argv[]) {
    volatile int seed = 0;
    volatile int results[64] = {0};
    
    /* Create seed from argv[0] */
    if (argc > 0 && argv[0]) {
        for (int i = 0; argv[0][i]; i++) {
            seed ^= argv[0][i] << ((i % 4) * 8);
        }
    }
    
    /* Execute different test cases based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle(results);
            #endif
            break;
        case 1:
            #ifdef __AVX512F__
            test_gather_intrinsic(results);
            #endif
            break;
        case 2:
            test_atomic_ops(results);
            break;
        case 3:
            test_openmp_simd(results, 128);
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            #ifdef __AVX512F__
            test_complex_builtin(results);
            #endif
            break;
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
