/* test_optabs_10_11_operands.c
 * Test program to cover 10/11 operand cases in optabs.cc
 * Compile with: gcc -O3 -mavx512f -mavx512vl -fopenmp -ftree-vectorize -fno-tree-slp-vectorize test.c -o test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* Strategy 1: Large vector shuffle operations */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another complex shuffle pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    
    use(&c);
    use(&d);
}
#endif

/* Strategy 2: AVX-512 gather intrinsics (if available) */
#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsic(volatile int* result) {
    float base[64] __attribute__((aligned(64)));
    int indices[16] __attribute__((aligned(64)));
    __m512i vindex;
    __m512 src, mask;
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) base[i] = i * 1.5f;
    for (int i = 0; i < 16; i++) indices[i] = i * 4;
    
    vindex = _mm512_load_si512(indices);
    mask = _mm512_set1_ps(1.0f);
    
    /* Gather operation with many implicit operands */
    __m512 gathered = _mm512_i32gather_ps(vindex, base, 4);
    
    /* Store result */
    _mm512_store_ps((void*)result, gathered);
    
    use(&gathered);
}
#endif

/* Strategy 3: Atomic operations with multiple parameters */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    intptr_t ptr_val = (intptr_t)result;
    volatile int* atomic_ptr = (volatile int*)&ptr_val;
    int expected = 0;
    int desired = 42;
    int weak = 0; /* Use strong by default */
    
    /* __atomic_compare_exchange with many parameters */
    /* Parameters: ptr, expected, desired, weak, success_memorder, failure_memorder */
    __atomic_compare_exchange(atomic_ptr, &expected, &desired, 
                              weak, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Another atomic with fetch_add and memory order */
    int old = __atomic_fetch_add(atomic_ptr, 1, __ATOMIC_ACQ_REL);
    
    result[0] = old;
    result[1] = expected;
    
    use(atomic_ptr);
}

/* Strategy 4: OpenMP SIMD with complex clauses */
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    const int N = 128;
    float a[N] __attribute__((aligned(64)));
    float b[N] __attribute__((aligned(64)));
    float c[N] __attribute__((aligned(64)));
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
    }
    
    /* Complex OpenMP SIMD pragma with multiple clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * b[i] + (float)i;
    }
    
    /* Store results */
    for (int i = 0; i < N && i < 16; i++) {
        result[i] = (int)c[i];
    }
    
    use(a);
    use(b);
    use(c);
}

/* Strategy 5: Multi-operand inline assembly */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        /* Fake multi-operand operation - compiler will parse as 10 operands */
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "mov %0, %1\n\t"
        "imul %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9\n\t"
        "add %0, %10"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
    use(&out2);
}

/* Strategy 6: Complex builtin with many arguments */
__attribute__((noipa, noinline))
void test_complex_builtin(volatile int* result) {
    /* __builtin_add_overflow with multiple accumulations */
    int a = 1000, b = 2000, c = 3000, d = 4000;
    int sum1, sum2, sum3, sum4;
    int ov1, ov2, ov3, ov4;
    
    /* Chain of overflow checks - may create complex RTL */
    ov1 = __builtin_add_overflow(a, b, &sum1);
    ov2 = __builtin_add_overflow(sum1, c, &sum2);
    ov3 = __builtin_add_overflow(sum2, d, &sum3);
    ov4 = __builtin_add_overflow(sum3, control, &sum4);
    
    /* Complex expression with many operands */
    result[0] = (sum1 * sum2) / (sum3 + 1) - sum4;
    result[1] = ov1 | ov2 | ov3 | ov4;
    
    use(&sum1);
    use(&sum2);
    use(&sum3);
    use(&sum4);
}

/* Main function with volatile control flow */
int main(int argc, char *argv[]) {
    /* Create volatile seed from argv[0] */
    unsigned int seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Allocate result buffers */
    volatile int results[6][32] = {{0}};
    
    /* Execute different test functions based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle(results[0]);
            #endif
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
            test_complex_builtin(results[5]);
            break;
    }
    
    /* Force execution of all functions if control is set */
    if (control) {
        #ifdef __AVX512F__
        test_vector_shuffle(results[0]);
        test_gather_intrinsic(results[1]);
        #endif
        test_atomic_operations(results[2]);
        test_openmp_simd(results[3]);
        test_multi_operand_asm(results[4]);
        test_complex_builtin(results[5]);
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 32; j++) {
            checksum ^= results[i][j];
        }
    }
    
    printf("Checksum: %d (test case: %d)\n", checksum, test_case);
    
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void *ptr) {
    /* Empty but referenced to keep variables alive */
    (void)ptr;
}
