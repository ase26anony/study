/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ====== Pattern 1: Vector shuffles with many operands ====== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle requiring many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 31, 30, 29, 28, 11, 10, 9, 8, 27, 26, 25, 24);
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result[i] = c[i] + d[i];
    }
    use((void*)result);
}
#endif

/* ====== Pattern 2: x86 gather intrinsics (many operands) ====== */
#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsic(volatile int* result) {
    __m512i index = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    int base[64];
    
    for (int i = 0; i < 64; i++) base[i] = i * 2;
    
    /* __m512i _mm512_mask_i32gather_epi32(__m512i src, __mmask16 k, __m512i index, void const *base, int scale) */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  /* src */
        mask,                    /* mask */
        index,                   /* index */
        base,                    /* base */
        4                        /* scale */
    );
    
    _mm512_store_epi32((void*)result, gathered);
    use((void*)result);
}
#endif

/* ====== Pattern 3: Atomic operations with many parameters ====== */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    volatile int shared = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0;  /* Use weak compare-exchange */
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange_n(
        &shared,           /* ptr */
        &expected,         /* expected */
        desired,           /* desired */
        weak,              /* weak */
        __ATOMIC_SEQ_CST,  /* success_memorder */
        __ATOMIC_ACQUIRE   /* failure_memorder */
    );
    
    /* Another atomic with fetch-and-add */
    int old = __atomic_fetch_add(&shared, 23, __ATOMIC_RELAXED);
    
    result[0] = success;
    result[1] = shared;
    result[2] = old;
    use((void*)result);
}

/* ====== Pattern 4: OpenMP SIMD with many clauses ====== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd(volatile int* result) {
    #define N 128
    int a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) \
                     simdlen(8) safelen(16) \
                     reduction(+:result[0])
    for (int i = 0; i < N; i++) {
        a[i] = b[i] * c[i] + i;
        result[0] += a[i];
    }
    
    /* Another loop with collapse clause */
    int matrix[16][16];
    #pragma omp simd collapse(2) aligned(matrix:64)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    result[1] = matrix[15][15];
    use((void*)result);
}
#endif

/* ====== Pattern 5: Inline assembly with many operands ====== */
__attribute__((noipa, noinline))
void test_many_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Custom 10-operand operation */\n\t"
        "add %1, %2\n\t"
        "add %3, %4\n\t"
        "add %5, %6\n\t"
        "add %7, %8\n\t"
        "imul %9, %10\n\t"
        : "=r"(out1), "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d), 
          "r"(e), "r"(f), "r"(g), "r"(h),
          "r"(i), "r"(j)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* Custom 11-operand operation */\n\t"
        "lea (%1,%2), %%rax\n\t"
        "add %3, %%rax\n\t"
        "add %4, %%rax\n\t"
        "add %5, %%rax\n\t"
        "add %6, %%rax\n\t"
        "add %7, %%rax\n\t"
        "add %8, %%rax\n\t"
        "add %9, %%rax\n\t"
        "add %10, %%rax\n\t"
        "add %11, %%rax\n\t"
        : "=r"(out2)
        : "r"(a), "r"(b), "r"(c), "r"(d),
          "r"(e), "r"(f), "r"(g), "r"(h),
          "r"(i), "r"(j), "r"(k)
        : "rax", "cc"
    );
    
    result[0] = out1 + out2;
    use((void*)result);
}

/* ====== Pattern 6: Complex builtin with many arguments ====== */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_neon_complex(volatile int* result) {
    /* Using multiple NEON operations that may expand to many operands */
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    
    /* Chain of operations */
    int32x4_t r1 = vmlaq_s32(a, b, c);
    int32x4_t r2 = vmlaq_s32(r1, c, d);
    int32x4_t r3 = vaddq_s32(r2, a);
    
    vst1q_s32((int32_t*)result, r3);
    use((void*)result);
}
#endif

/* ====== Main function with runtime dispatch ====== */
int main(int argc, char *argv[]) {
    /* Create volatile seed from program name */
    unsigned seed = 0;
    for (char *p = argv[0]; *p; p++) {
        seed = seed * 31 + *p;
    }
    control = seed;
    
    /* Allocate result arrays */
    volatile int *results = (volatile int*)malloc(256 * sizeof(int));
    if (!results) return 1;
    
    /* Initialize results */
    for (int i = 0; i < 256; i++) {
        results[i] = i;
    }
    
    /* Dispatch based on seed to test different patterns */
    int pattern = seed % 6;
    
    switch (pattern) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle(results);
            #endif
            break;
        case 1:
            #ifdef __AVX512F__
            test_gather_intrinsic(results + 16);
            #endif
            break;
        case 2:
            test_atomic_operations(results + 32);
            break;
        case 3:
            #ifdef _OPENMP
            test_openmp_simd(results + 48);
            #endif
            break;
        case 4:
            test_many_operand_asm(results + 64);
            break;
        case 5:
            #ifdef __ARM_NEON
            test_neon_complex(results + 80);
            #endif
            break;
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= results[i];
    }
    
    printf("Checksum: %d (pattern: %d)\n", checksum, pattern);
    
    free((void*)results);
    return 0;
}
