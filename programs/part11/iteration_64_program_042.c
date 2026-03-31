/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent interprocedural optimizations */
#define NOIPA __attribute__((noipa, noinline, noclone))

/* External function to keep values live */
extern void use(void*);

/* ========== Pattern 1: Vector shuffles with many operands ========== */
NOIPA void test_vector_shuffle(volatile int* result) {
    /* Large vector types for potential many-operand shuffles */
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v16si __attribute__((vector_size(64)));
    
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    v16si c = {0};
    
    /* Complex shuffle that might expand to many operands */
    c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7,  /* First 8 from a */
        0, 1, 2, 3, 4, 5, 6, 7); /* Next 8 from a again */
    
    /* Store result to volatile memory */
    memcpy((void*)result, &c, sizeof(c));
    use((void*)result);
}

/* ========== Pattern 2: x86-specific gather intrinsics ========== */
#ifdef __x86_64__
#include <x86intrin.h>
NOIPA void test_gather_intrinsic(volatile double* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512d src = _mm512_set1_pd(2.0);
    __m512i index = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    double base[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    
    /* Gather with scale - may expand to many operands */
    __m512d gathered = _mm512_i64gather_pd(index, base, 8);
    
    _mm512_store_pd((double*)result, gathered);
    use((void*)result);
}
#endif

/* ========== Pattern 3: Atomic operations with many parameters ========== */
NOIPA void test_atomic_ops(volatile int* result) {
    int expected = 42;
    int desired = 43;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange has 6 arguments + implicit ones */
    int success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                            0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Force use of result */
    *result = success + expected;
    use((void*)result);
}

/* ========== Pattern 4: OpenMP SIMD with complex clauses ========== */
NOIPA void test_omp_simd(volatile int* result) {
    #define N 128
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                     reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
        result[0] += a[i];
    }
    
    use((void*)result);
}

/* ========== Pattern 5: Inline assembly with many operands ========== */
NOIPA void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
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
    
    /* 11-operand asm statement */
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
    
    *result = out1 + out2;
    use((void*)result);
}

/* ========== Pattern 6: ARM/AArch64 specific intrinsics ========== */
#ifdef __aarch64__
#include <arm_neon.h>
NOIPA void test_aarch64_intrinsics(volatile int* result) {
    /* Multi-register load/store operations can have many operands */
    int32x4x4_t data;
    int32_t src[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    
    /* Load 4 registers - may expand to many operands */
    data = vld1q_s32_x4(src);
    
    /* Store back */
    vst1q_s32_x4((int32_t*)result, data);
    use((void*)result);
}
#endif

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    volatile int seed = 0;
    volatile int buffer[256] = {0};
    volatile double dbl_buffer[16] = {0.0};
    
    /* Create a simple hash from program name for seed */
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    
    /* Execute different patterns based on seed */
    switch (seed % 6) {
        case 0:
            test_vector_shuffle(buffer);
            break;
        case 1:
            #ifdef __x86_64__
            test_gather_intrinsic(dbl_buffer);
            #else
            test_vector_shuffle(buffer);
            #endif
            break;
        case 2:
            test_atomic_ops(buffer);
            break;
        case 3:
            test_omp_simd(buffer);
            break;
        case 4:
            test_multi_operand_asm(buffer);
            break;
        case 5:
            #ifdef __aarch64__
            test_aarch64_intrinsics(buffer);
            #else
            test_multi_operand_asm(buffer);
            #endif
            break;
    }
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += buffer[i];
    }
    
    printf("Checksum: %d (Seed: %d)\n", checksum, seed);
    return 0;
}
