/* test_optabs_multioperand.c - Cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent interprocedural optimization */
#define NOIPA __attribute__((noipa, noinline))

/* External function to keep values live */
extern void use(void*);

/* ==================== PATTERN 1: Vector shuffles with many indices ==================== */
NOIPA void test_vector_shuffle(volatile int* result) {
    /* Use GCC vector extensions with explicit shuffle indices */
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v16si __attribute__((vector_size(64)));
    
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,    /* First 8 from a */
        16,17,18,19,20,21,22,23  /* Next 8 from b */
    );
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,
        31,30,29,28,27,26,25,24
    );
    
    /* Combine results */
    v16si e = c + d;
    
    /* Store to volatile memory to force computation */
    memcpy((void*)result, &e, sizeof(e));
    use((void*)result);
}

/* ==================== PATTERN 2: x86-specific gather intrinsics ==================== */
#ifdef __x86_64__
#include <x86intrin.h>

NOIPA void test_gather_intrinsic(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __mmask16 mask = 0xFFFF;
    int base[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = i;
    
    /* __builtin_ia32_gathersiv16si has many parameters:
       src, base, index, scale, mask */
    __m512i gathered = _mm512_i32gather_epi32(index, base, 4);
    
    /* Another operation with mask */
    __m512i scaled = _mm512_slli_epi32(gathered, 1);
    __m512i result_vec = _mm512_mask_add_epi32(gathered, mask, scaled, gathered);
    
    _mm512_store_epi32((void*)result, result_vec);
    use((void*)result);
}
#endif

/* ==================== PATTERN 3: Atomic operations with many parameters ==================== */
NOIPA void test_atomic_operations(volatile int* result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0; /* strong compare-exchange */
    
    /* __atomic_compare_exchange has many parameters:
       ptr, expected, desired, weak, success_memorder, failure_memorder */
    __atomic_compare_exchange(&atomic_var, &expected, &desired, 
                              weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Another atomic with different ordering */
    int old = __atomic_fetch_add(&atomic_var, 23, __ATOMIC_ACQ_REL);
    
    /* Complex atomic exchange */
    int new_val = __atomic_exchange_n(&atomic_var, old + 1, __ATOMIC_SEQ_CST);
    
    *result = new_val;
    use((void*)result);
}

/* ==================== PATTERN 4: OpenMP SIMD with many clauses ==================== */
NOIPA void test_openmp_simd(volatile int* result) {
    #define N 1024
    int a[N] __attribute__((aligned(64)));
    int b[N] __attribute__((aligned(64)));
    int c[N] __attribute__((aligned(64)));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to multi-operand operations */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(16) safelen(32)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i] * 3 - (i & 0xF);
    }
    
    /* Reduction with SIMD */
    int sum = 0;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    *result = sum;
    use((void*)result);
}

/* ==================== PATTERN 5: Inline assembly with many operands ==================== */
NOIPA void test_multi_operand_asm(volatile int* result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* 10-operand template */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand template */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    *result = out1 + out2;
    use((void*)result);
}

/* ==================== PATTERN 6: Complex builtin with many arguments ==================== */
NOIPA void test_complex_builtin(volatile int* result) {
    /* __builtin_constant_p with many arguments in expression */
    int x = 0;
    
    /* Complex expression that may expand to many operands during optimization */
    x = __builtin_constant_p((long)result) ? 1 : 0;
    x += __builtin_clz(0x12345678);
    x += __builtin_popcount(0xFFFFFFFF);
    x += __builtin_ffs(1024);
    x += __builtin_parity(255);
    
    /* Memory builtins with many parameters */
    char src[256], dst[256];
    __builtin_memcpy(dst, src, 256);
    __builtin_memset(src, x, 256);
    
    /* Complex math builtin */
    double y = __builtin_pow(2.0, 10.0);
    y += __builtin_sin(y);
    y += __builtin_cos(y);
    
    *result = x + (int)y;
    use((void*)result);
}

/* ==================== MAIN DRIVER ==================== */
int main(int argc, char *argv[]) {
    volatile int results[6] = {0};
    volatile int seed = 0;
    
    /* Create seed from program name */
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    
    /* Execute different patterns based on seed to avoid dead code elimination */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle(results);
            break;
        case 1:
#ifdef __x86_64__
            test_gather_intrinsic(results + 1);
#else
            test_vector_shuffle(results + 1);
#endif
            break;
        case 2:
            test_atomic_operations(results + 2);
            break;
        case 3:
            test_openmp_simd(results + 3);
            break;
        case 4:
            test_multi_operand_asm(results + 4);
            break;
        case 5:
            test_complex_builtin(results + 5);
            break;
    }
    
    /* Compute checksum to ensure all code contributes */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum ^= results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
