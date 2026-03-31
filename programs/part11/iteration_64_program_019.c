/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's optabs expansion for 10 and 11 operand cases.
 * It uses various techniques to create RTL patterns requiring many operands.
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
typedef float v16sf __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle requiring many operands during expansion */
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

/* ========== Approach 2: x86-specific gather intrinsics ========== */

#ifdef __x86_64__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsics(volatile int* result) {
    /* AVX-512 gather operations can have many operands */
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xFFFF;
    
    /* Simulate gather from memory */
    int base[64] __attribute__((aligned(64)));
    for (int i = 0; i < 64; i++) base[i] = i;
    
    /* Complex operation that might expand to many operands */
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

/* ========== Approach 3: Atomic operations with many parameters ========== */

__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int obj = 42;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(
        &obj,                    // ptr
        &expected,               // expected
        &desired,                // desired
        0,                       // weak
        __ATOMIC_SEQ_CST,        // success_memorder
        __ATOMIC_RELAXED         // failure_memorder
    );
    
    /* Another atomic operation */
    int old = __atomic_fetch_add(&obj, 10, __ATOMIC_SEQ_CST);
    
    result[0] = success;
    result[1] = old;
    result[2] = obj;
    
    use(&obj);
}

/* ========== Approach 4: OpenMP SIMD with complex clauses ========== */

#define N 1024

__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    int d[N] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = 0;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                     simdlen(16) safelen(32) \
                     reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        d[i] = a[i] + b[i] * c[i];
        result[0] += d[i];
    }
    
    use(d);
}

/* ========== Approach 5: Inline assembly with many operands ========== */

__attribute__((noipa, noinline))
void test_many_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand assembly statement */
    asm volatile (
        "/* Multi-operand test %0 = (%1 + %2) * (%3 + %4) + (%5 * %6) - (%7 + %8 + %9) */\n\t"
        "addl %1, %2\n\t"
        "addl %3, %4\n\t"
        "imull %%eax, %%edx\n\t"
        "movl %5, %%eax\n\t"
        "imull %6, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "movl %7, %%edx\n\t"
        "addl %8, %%edx\n\t"
        "addl %9, %%edx\n\t"
        "subl %%edx, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), 
          "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "eax", "edx", "cc"
    );
    
    /* 11-operand assembly statement */
    int k = 11;
    asm volatile (
        "/* 11-operand test */\n\t"
        "leal (%1,%2), %%eax\n\t"
        "leal (%3,%4), %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "addl %5, %%eax\n\t"
        "addl %6, %%eax\n\t"
        "addl %7, %%eax\n\t"
        "addl %8, %%eax\n\t"
        "addl %9, %%eax\n\t"
        "addl %10, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d),
          "r"(e), "r"(f), "r"(g), "r"(h),
          "r"(i), "r"(j)
        : "eax", "edx", "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use(&out1);
    use(&out2);
}

/* ========== Approach 6: Complex builtin with many arguments ========== */

__attribute__((noipa, noinline))
void test_complex_builtin(volatile int* result) {
    /* __builtin_constant_p with many arguments in a complex expression */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Complex expression that might expand to many operands */
    int complex_result = 
        (__builtin_constant_p(a) ? a : 0) +
        (__builtin_constant_p(b) ? b : 0) +
        (__builtin_constant_p(c) ? c : 0) +
        (__builtin_constant_p(d) ? d : 0) +
        (__builtin_constant_p(e) ? e : 0) +
        (__builtin_constant_p(f) ? f : 0) +
        (__builtin_constant_p(g) ? g : 0) +
        (__builtin_constant_p(h) ? h : 0) +
        (__builtin_constant_p(i) ? i : 0) +
        (__builtin_constant_p(j) ? j : 0);
    
    /* Another complex builtin usage */
    long long ll_result = __builtin_add_overflow_p(
        (long long)a * b * c * d * e,
        (long long)f * g * h * i * j,
        (long long)0
    ) ? 0 : (a * b * c * d * e + f * g * h * i * j);
    
    result[0] = complex_result;
    result[1] = (int)ll_result;
    
    use(&complex_result);
    use(&ll_result);
}

/* ========== Main function with volatile control flow ========== */

int main(int argc, char *argv[]) {
    /* Create volatile seed from argv[0] */
    unsigned seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Allocate result arrays */
    volatile int results[6][64] = {0};
    
    /* Execute different test cases based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0:
            printf("Testing vector shuffle...\n");
            test_vector_shuffle(results[0]);
            break;
            
        case 1:
#ifdef __x86_64__
            printf("Testing gather intrinsics...\n");
            test_gather_intrinsics(results[1]);
#else
            printf("Gather intrinsics not available on this arch, using fallback...\n");
            test_vector_shuffle(results[1]);
#endif
            break;
            
        case 2:
            printf("Testing atomic operations...\n");
            test_atomic_operations(results[2]);
            break;
            
        case 3:
            printf("Testing OpenMP SIMD...\n");
            test_openmp_simd(results[3]);
            break;
            
        case 4:
            printf("Testing many-operand assembly...\n");
            test_many_operand_asm(results[4]);
            break;
            
        case 5:
            printf("Testing complex builtins...\n");
            test_complex_builtin(results[5]);
            break;
    }
    
    /* Compute checksum from all results to ensure execution */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 64; j++) {
            checksum += results[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test case executed: %d\n", test_case);
    
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void *ptr) {
    /* Empty but referenced to keep variables alive */
    (void)ptr;
}
