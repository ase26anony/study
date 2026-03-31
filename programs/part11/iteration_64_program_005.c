/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inter-procedural optimization */
#define NOIPA __attribute__((noipa, noinline, noclone))

/* External function to keep values live */
extern void use(void *);

/* ==================== PATTERN 1: Vector Shuffle with Many Elements ==================== */
NOIPA
void test_vector_shuffle(volatile int *result) {
    /* Use GCC vector extensions with 32-byte vectors (8 ints) */
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v16si __attribute__((vector_size(64)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Create 16-element vector from two 8-element vectors */
    v16si c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7,    /* First 8 from a */
        0, 1, 2, 3, 4, 5, 6, 7);   /* Next 8 from a (duplicate) */
    
    /* Another shuffle with different pattern - may require many operands */
    v16si d = __builtin_shufflevector(c, c,
        15, 14, 13, 12, 11, 10, 9, 8,
        7, 6, 5, 4, 3, 2, 1, 0);
    
    /* Store result to volatile memory */
    for (int i = 0; i < 16; i++) {
        result[i] = d[i];
    }
    use(&d);
}

/* ==================== PATTERN 2: x86 Gather Intrinsics (if available) ==================== */
#ifdef __x86_64__
#include <x86intrin.h>
NOIPA
void test_gather_intrinsic(volatile int *result) {
    /* Use AVX512 gather if available - these have many operands */
    #ifdef __AVX512F__
    __m512i index = _mm512_set_epi32(0, 8, 16, 24, 32, 40, 48, 56,
                                     64, 72, 80, 88, 96, 104, 112, 120);
    __mmask16 mask = 0xFFFF;
    int base[128];
    for (int i = 0; i < 128; i++) base[i] = i;
    
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    _mm512_store_epi32((void *)result, gathered);
    use(&gathered);
    #endif
}
#endif

/* ==================== PATTERN 3: Atomic Compare Exchange with Many Parameters ==================== */
NOIPA
void test_atomic_ops(volatile int *result) {
    int *ptr = (int *)result;
    int expected = 42;
    int desired = 100;
    
    /* __atomic_compare_exchange has many parameters:
       ptr, expected, desired, weak, success_memorder, failure_memorder */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, /* weak */
                                            __ATOMIC_SEQ_CST, 
                                            __ATOMIC_ACQUIRE);
    
    /* Another atomic with many operands */
    int val = 50;
    __atomic_add_fetch(ptr, val, __ATOMIC_RELAXED);
    
    result[1] = success;
    result[2] = expected;
    use(&success);
}

/* ==================== PATTERN 4: OpenMP SIMD with Multiple Clauses ==================== */
NOIPA
void test_openmp_simd(volatile int *result, int n) {
    int a[128], b[128], c[128];
    for (int i = 0; i < 128; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with many clauses - may expand to multi-operand operations */
    #pragma omp simd linear(i:1) aligned(a,b,c:32) simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i] * 3;
    }
    
    for (int i = 0; i < n; i++) {
        result[i] = c[i];
    }
    use(c);
}

/* ==================== PATTERN 5: Inline Assembly with Many Operands ==================== */
NOIPA
void test_multi_operand_asm(volatile int *result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* 10-operand asm */\n\t"
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "mov %[out1], %[i]\n\t"
        "mov %[out2], %[j]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand asm */\n\t"
        "imul %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "sub %[e], %[f]\n\t"
        "and %[g], %[h]\n\t"
        "or %[i], %[j]\n\t"
        "mov %[out1], %[k]"
        : [out1] "=r" (out1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    result[0] = out1 + out2;
    use(&out1);
}

/* ==================== PATTERN 6: Complex Vector Operations ==================== */
NOIPA
void test_complex_vector_ops(volatile int *result) {
    /* Use complex vector operations that might expand to many operands */
    typedef float v8sf __attribute__((vector_size(32)));
    typedef float v16sf __attribute__((vector_size(64)));
    
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Fused multiply-add style operations */
    v8sf v3 = v1 * v2 + v1;
    v8sf v4 = v3 - v2 * v1;
    
    /* Permute with many indices */
    v8sf v5 = __builtin_shufflevector(v3, v4, 0, 2, 4, 6, 1, 3, 5, 7);
    
    for (int i = 0; i < 8; i++) {
        result[i] = (int)v5[i];
    }
    use(&v5);
}

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char *argv[]) {
    /* Create volatile result arrays */
    volatile int results[6][128];
    
    /* Simple hash from argv[0] for branch selection */
    unsigned seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize results */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 128; j++) {
            results[i][j] = 0;
        }
    }
    
    /* Execute different patterns based on seed */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            test_vector_shuffle(results[0]);
            break;
        case 1:
            #ifdef __x86_64__
            test_gather_intrinsic(results[1]);
            #else
            test_vector_shuffle(results[1]);
            #endif
            break;
        case 2:
            test_atomic_ops(results[2]);
            break;
        case 3:
            test_openmp_simd(results[3], 64);
            break;
        case 4:
            test_multi_operand_asm(results[4]);
            break;
        case 5:
            test_complex_vector_ops(results[5]);
            break;
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 64; j++) {
            checksum += results[i][j];
        }
    }
    
    printf("Checksum: %d (pattern: %d)\n", checksum, pattern);
    return 0;
}

/* Dummy function to prevent optimization */
void use(void *ptr) {
    /* Empty but referenced to keep values live */
    (void)ptr;
}
