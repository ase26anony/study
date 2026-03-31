/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */
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
    /* Large vector type requiring many shuffle indices */
    typedef int v16si __attribute__((vector_size(64)));
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with explicit indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,    /* 8 from a */
        16,17,18,19,20,21,22,23  /* 8 from b (16-23) */
    );
    
    /* Store result through volatile pointer */
    memcpy((void*)result, &c, sizeof(c));
    use((void*)result);
}

/* ==================== PATTERN 2: x86 Gather Intrinsic (if available) ==================== */
#ifdef __x86_64__
#include <x86intrin.h>
__attribute__((target("avx512f"), noipa, noinline))
void test_gather_intrinsic(volatile double* result) {
    /* AVX-512 gather with many parameters */
    __m512d src = _mm512_set1_pd(1.0);
    __m512i index = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __mmask8 mask = 0xFF;
    
    /* This builtin typically expands to many operands */
    __m512d gathered = _mm512_i64gather_pd(index, (const void*)result, 8, src, mask);
    
    _mm512_storeu_pd((void*)result, gathered);
    use((void*)result);
}
#endif

/* ==================== PATTERN 3: Atomic Compare Exchange with Many Parameters ==================== */
__attribute__((noipa, noinline))
void test_atomic_ops(volatile int* result) {
    int expected = 42;
    int desired = 84;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange has many parameters that may expand */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0,  /* weak */
                                            __ATOMIC_SEQ_CST,  /* success memorder */
                                            __ATOMIC_RELAXED); /* failure memorder */
    
    /* Force use of result */
    *result = success + expected + desired;
    use((void*)result);
}

/* ==================== PATTERN 4: OpenMP SIMD with Complex Clauses ==================== */
__attribute__((noipa, noinline))
void test_omp_simd(volatile double* a, volatile double* b, volatile double* c, int n) {
    int i;
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(32) \
                    private(i) reduction(+:control)
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + (double)control;
    }
    
    use((void*)a);
    use((void*)b);
    use((void*)c);
}

/* ==================== PATTERN 5: Inline Assembly with Many Operands ==================== */
__attribute__((noipa, noinline))
void test_many_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand template %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */\n"
        "add %0, %1, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9"
        : "=r"(out1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand template */\n"
        "mov %0, %1\n"
        "add %0, %0, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9\n"
        "add %0, %0, %10"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    *result = out1 + out2;
    use((void*)result);
}

/* ==================== PATTERN 6: Complex Vector Operation ==================== */
__attribute__((noipa, noinline))
void test_complex_vector_op(volatile int* result) {
    /* Using GCC vector extensions with ternary operation */
    typedef int v8si __attribute__((vector_size(32)));
    v8si v1 = {1,2,3,4,5,6,7,8};
    v8si v2 = {8,7,6,5,4,3,2,1};
    v8si mask = {0,-1,0,-1,0,-1,0,-1};
    
    /* Conditional select with vector mask - may expand to many operands */
    v8si v3 = __builtin_shuffle(v1, v2, mask);
    
    memcpy((void*)result, &v3, sizeof(v3));
    use((void*)result);
}

/* ==================== MAIN DRIVER ==================== */
int main(int argc, char *argv[]) {
    /* Use argv[0] to create a simple hash for control flow */
    unsigned seed = 0;
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    control = seed;
    
    /* Allocate volatile memory regions */
    volatile int result1 = 0;
    volatile double array1[64], array2[64], array3[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        array1[i] = (double)i;
        array2[i] = (double)(i * 2);
        array3[i] = (double)(i * 3);
    }
    
    /* Execute different patterns based on seed */
    switch (control % 6) {
        case 0:
            test_vector_shuffle(&result1);
            break;
        case 1:
#ifdef __x86_64__
            test_gather_intrinsic(array1);
#endif
            break;
        case 2:
            test_atomic_ops(&result1);
            break;
        case 3:
            test_omp_simd(array1, array2, array3, 64);
            break;
        case 4:
            test_many_operand_asm(&result1);
            break;
        case 5:
            test_complex_vector_op(&result1);
            break;
    }
    
    /* Compute checksum to ensure all code affects output */
    int checksum = result1;
    for (int i = 0; i < 64; i++) {
        checksum += (int)array1[i] + (int)array2[i] + (int)array3[i];
    }
    
    printf("Checksum: %d (control: %d)\n", checksum, control);
    return 0;
}
