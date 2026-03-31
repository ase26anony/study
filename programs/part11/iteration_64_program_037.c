/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent interprocedural optimizations */
#define NOIPA __attribute__((noipa, noinline))

/* External function to keep values live */
extern void use(void*);

/* ========== Pattern 1: Vector shuffles with many operands ========== */
NOIPA void test_vector_shuffle(volatile int* result) {
    /* Large vector types that may require many operands during expansion */
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v16si __attribute__((vector_size(64)));
    
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle with many indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Another shuffle pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 31, 30, 29, 28, 11, 10, 9, 8, 27, 26, 25, 24);
    
    /* Use results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += c[i] + d[i];
    }
    *result = sum;
    use(&c);
    use(&d);
}

/* ========== Pattern 2: x86-specific gather intrinsics ========== */
#ifdef __x86_64__
#include <x86intrin.h>

NOIPA void test_gather_intrinsics(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    #ifdef __AVX512F__
    __m512i index = _mm512_set_epi32(0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60);
    __mmask16 mask = 0xAAAA;
    int base[64];
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        index,                   // index
        (const void*)base,       // base
        4                        // scale
    );
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += gathered[i];
    }
    *result = sum;
    use(&gathered);
    #endif
}
#endif

/* ========== Pattern 3: Atomic operations with many parameters ========== */
NOIPA void test_atomic_operations(volatile int* result) {
    intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(
        &atomic_var,           // ptr
        &expected,             // expected
        &desired,              // desired
        0,                     // weak
        __ATOMIC_SEQ_CST,      // success_memorder
        __ATOMIC_ACQUIRE       // failure_memorder
    );
    
    /* Another atomic with multiple ordering parameters */
    intptr_t val = 100;
    __atomic_store(&atomic_var, &val, __ATOMIC_RELEASE);
    intptr_t loaded = __atomic_load(&atomic_var, __ATOMIC_ACQUIRE);
    
    *result = success + loaded;
    use(&atomic_var);
}

/* ========== Pattern 4: OpenMP SIMD with complex clauses ========== */
NOIPA void test_openmp_simd(volatile int* result) {
    #define N 1024
    static int a[N] __attribute__((aligned(64)));
    static int b[N] __attribute__((aligned(64)));
    static int c[N] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    int sum = 0;
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                     reduction(+:sum) if(simd:1)
    for (int i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
        sum += a[i];
    }
    
    *result = sum;
    use(a);
    use(b);
    use(c);
}

/* ========== Pattern 5: Many-operand inline assembly ========== */
NOIPA void test_many_operand_asm(volatile int* result) {
    int out;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand assembly statement */
    asm volatile (
        "/* Custom 10-operand operation */\n\t"
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0"
        : "=r"(out)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand assembly statement */
    int out2;
    int k = 11;
    asm volatile (
        "/* Custom 11-operand operation */\n\t"
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    
    *result = out + out2;
    use(&out);
    use(&out2);
}

/* ========== Pattern 6: Complex builtin with many arguments ========== */
NOIPA void test_complex_builtin(volatile int* result) {
    /* __builtin_constant_p with many arguments in a complex expression */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* Complex expression that may expand to many operands */
    int complex_expr = __builtin_constant_p(a) ? 
                      (__builtin_constant_p(b) ? 
                       (__builtin_constant_p(c) ? 
                        (__builtin_constant_p(d) ? 
                         (__builtin_constant_p(e) ? 
                          (__builtin_constant_p(f) ? 
                           (__builtin_constant_p(g) ? 
                            (__builtin_constant_p(h) ? 
                             (__builtin_constant_p(i) ? 
                              (__builtin_constant_p(j) ? 1 : 0) : 0) : 0) : 0) : 0) : 0) : 0) : 0) : 0) : 0;
    
    /* Another complex builtin usage */
    long long ll_result = __builtin_add_overflow_p(a, b, (int){0}) +
                         __builtin_sub_overflow_p(c, d, (int){0}) +
                         __builtin_mul_overflow_p(e, f, (int){0}) +
                         __builtin_add_overflow_p(g, h, (int){0}) +
                         __builtin_sub_overflow_p(i, j, (int){0});
    
    *result = complex_expr + (int)ll_result;
    use(&complex_expr);
    use(&ll_result);
}

/* ========== Main execution flow ========== */
int main(int argc, char *argv[]) {
    volatile int seed = 0;
    volatile int results[6] = {0};
    
    /* Create a simple hash from argv[0] for seed */
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Execute different patterns based on seed */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            test_vector_shuffle(&results[0]);
            break;
        case 1:
            #ifdef __x86_64__
            test_gather_intrinsics(&results[1]);
            #endif
            break;
        case 2:
            test_atomic_operations(&results[2]);
            break;
        case 3:
            test_openmp_simd(&results[3]);
            break;
        case 4:
            test_many_operand_asm(&results[4]);
            break;
        case 5:
            test_complex_builtin(&results[5]);
            break;
    }
    
    /* Compute checksum to ensure all code contributes */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d (seed: %d, pattern: %d)\n", checksum, seed, pattern);
    
    return 0;
}

/* Dummy definition to satisfy external reference */
void use(void *ptr) {
    asm volatile ("" : : "r"(ptr) : "memory");
}
