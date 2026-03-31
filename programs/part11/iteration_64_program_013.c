/* test_optabs_coverage.c - Cover 10/11-operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== PATTERN 1: Vector Shuffle with Many Elements ==================== */
__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    /* Large vector types that may require many operands during expansion */
    typedef int v16si __attribute__((vector_size(64)));
    typedef long long v8di __attribute__((vector_size(64)));
    
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle with many indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7,     /* First 8 from a */
        16, 17, 18, 19, 20, 21, 22, 23  /* Next 8 from b */
    );
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 11, 10, 9, 8,
        31, 30, 29, 28, 27, 26, 25, 24
    );
    
    /* Combine results */
    v16si e = c + d;
    
    /* Store to volatile memory to force computation */
    for (int i = 0; i < 16; i++) {
        result[i] = e[i];
    }
    
    use(&e);
}

/* ==================== PATTERN 2: x86 AVX-512 Gather Intrinsics ==================== */
#ifdef __AVX512F__
#include <immintrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    /* __builtin_ia32_gathersiv16si may expand to many operands */
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Store result */
    _mm512_store_epi32((void*)result, gathered);
    
    use(&gathered);
}
#endif

/* ==================== PATTERN 3: Atomic Operations with Many Parameters ==================== */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, /* weak */
                                            __ATOMIC_SEQ_CST, 
                                            __ATOMIC_ACQUIRE);
    
    /* Another atomic with multiple memory orders */
    int val = __atomic_load_n(ptr, __ATOMIC_RELAXED);
    __atomic_store_n(ptr, val * 2, __ATOMIC_RELEASE);
    
    /* Atomic exchange with memory order */
    int old = __atomic_exchange_n(ptr, 100, __ATOMIC_ACQ_REL);
    
    result[1] = success + old;
    use(ptr);
}

/* ==================== PATTERN 4: OpenMP SIMD with Complex Clauses ==================== */
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    const int N = 128;
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32) \
                    reduction(+:control)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * control;
    }
    
    /* Complex SIMD operation with multiple arrays */
    #pragma omp simd aligned(a,b,c:64) simdlen(8)
    for (int i = 0; i < N; i++) {
        a[i] = (b[i] << 2) | (c[i] & 0xFF);
    }
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] + c[i];
    }
    
    use(a); use(b); use(c);
}

/* ==================== PATTERN 5: Inline Assembly with Many Operands ==================== */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand assembly statement */
    asm volatile (
        "/* Multi-operand template %0 = %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9 */\n\t"
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
    
    /* 11-operand assembly statement */
    asm volatile (
        "/* 11-operand template */\n\t"
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
    use(&out1); use(&out2);
}

/* ==================== PATTERN 6: Complex Built-in with Many Arguments ==================== */
__attribute__((noipa, noinline))
void test_complex_builtin(volatile int* result) {
    /* __builtin_constant_p with many arguments (though it may not expand to RTL) */
    /* Use __sync builtins which may have complex expansions */
    long sync_var = 0;
    
    /* __sync_fetch_and_add with memory barrier */
    long old = __sync_fetch_and_add(&sync_var, 42);
    
    /* __sync_val_compare_and_swap */
    long cmp = __sync_val_compare_and_swap(&sync_var, old, old + 100);
    
    /* __sync_lock_test_and_set */
    long locked = __sync_lock_test_and_set(&sync_var, 999);
    
    /* __sync_synchronize */
    __sync_synchronize();
    
    result[0] = (int)(old + cmp + locked);
    use(&sync_var);
}

/* ==================== MAIN EXECUTION FLOW ==================== */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed from program name */
    unsigned seed = 0;
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    control = seed;
    
    /* Allocate aligned volatile memory for results */
    volatile int *results = aligned_alloc(64, 64 * sizeof(int));
    if (!results) return 1;
    
    /* Clear results */
    for (int i = 0; i < 64; i++) results[i] = 0;
    
    /* Execute different patterns based on seed */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            test_vector_shuffle(results);
            break;
        case 1:
#ifdef __AVX512F__
            test_avx512_gather(results);
#else
            test_vector_shuffle(results);
#endif
            break;
        case 2:
            test_atomic_operations(results);
            break;
        case 3:
            test_openmp_simd(results);
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            test_complex_builtin(results);
            break;
    }
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Result checksum: %d (pattern %d, seed %u)\n", checksum, pattern, seed);
    
    free((void*)results);
    return 0;
}

/* Dummy implementation of external function */
void use(void *ptr) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(ptr) : "memory");
}
