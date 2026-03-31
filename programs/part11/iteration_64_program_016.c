/* test_optabs_coverage.c - Cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent interprocedural optimization */
#define NOIPA __attribute__((noipa, noinline, noclone))

/* External function to keep values live */
extern void use(void*);

/* ==================== Pattern 1: Vector Shuffle with Many Operands ==================== */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

NOIPA void test_vector_shuffle(volatile int* result) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c;
    
    /* Shuffle with explicit indices - may expand to many operands */
    c = __builtin_shufflevector(a, b, 0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Use volatile store to prevent optimization */
    volatile v4si* vptr = &c;
    result[0] = (*vptr)[0] + (*vptr)[1] + (*vptr)[2] + (*vptr)[3];
}
#endif

/* ==================== Pattern 2: AVX-512 Gather Intrinsics ==================== */
#ifdef __AVX512F__
#include <x86intrin.h>

NOIPA void test_avx512_gather(volatile int* result) {
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __mmask16 mask = 0xFFFF;
    __m512d src = _mm512_set1_pd(1.0);
    
    /* Aligned memory for gather */
    double base_array[64] __attribute__((aligned(64)));
    for (int i = 0; i < 64; i++) base_array[i] = i * 1.5;
    
    /* Gather intrinsic with many parameters */
    __m512d gathered = _mm512_mask_i32gather_pd(src, mask, index, base_array, 8);
    
    /* Extract result */
    volatile __m512d* vptr = &gathered;
    double sum = 0;
    for (int i = 0; i < 8; i++) sum += (*vptr)[i];
    result[0] = (int)sum;
}
#endif

/* ==================== Pattern 3: Atomic Compare Exchange ==================== */
NOIPA void test_atomic_ops(volatile int* result) {
    volatile int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int success;
    
    /* __atomic_compare_exchange with many parameters */
    success = __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                          0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Use both success and atomic_var */
    result[0] = success ? atomic_var : -1;
    result[1] = expected;
}

/* ==================== Pattern 4: OpenMP SIMD with Many Clauses ==================== */
#ifdef _OPENMP
NOIPA void test_openmp_simd(volatile int* result, int n) {
    int a[256] __attribute__((aligned(64)));
    int b[256] __attribute__((aligned(64)));
    int c[256] __attribute__((aligned(64)));
    
    for (int i = 0; i < 256; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses - may expand to many operands */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < n; i++) sum += c[i];
    result[0] = sum;
}
#endif

/* ==================== Pattern 5: Inline Assembly with Many Operands ==================== */
NOIPA void test_multi_operand_asm(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm for coverage */\n\t"
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
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), 
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    asm volatile (
        "/* 11-operand asm */\n\t"
        "imul %1, %0\n\t"
        "imul %2, %0\n\t"
        "imul %3, %0\n\t"
        "imul %4, %0\n\t"
        "imul %5, %0\n\t"
        "imul %6, %0\n\t"
        "imul %7, %0\n\t"
        "imul %8, %0\n\t"
        "imul %9, %0\n\t"
        "imul %10, %0"
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    result[0] = out1 + out2;
}

/* ==================== Pattern 6: Complex Vector Operations ==================== */
#ifdef __ARM_NEON
#include <arm_neon.h>

NOIPA void test_neon_operations(volatile int* result) {
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    
    /* Multiple vector operations in sequence */
    int32x4_t e = vaddq_s32(a, b);
    int32x4_t f = vmulq_s32(c, d);
    int32x4_t g = vaddq_s32(e, f);
    
    /* Lane extraction operations */
    int32_t lane0 = vgetq_lane_s32(g, 0);
    int32_t lane1 = vgetq_lane_s32(g, 1);
    int32_t lane2 = vgetq_lane_s32(g, 2);
    int32_t lane3 = vgetq_lane_s32(g, 3);
    
    result[0] = lane0 + lane1 + lane2 + lane3;
}
#endif

/* ==================== Main Function with Volatile Control Flow ==================== */
int main(int argc, char* argv[]) {
    volatile int results[10] = {0};
    volatile int seed = 0;
    
    /* Create seed from argv[0] */
    if (argc > 0 && argv[0]) {
        for (char* p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Call different patterns based on seed to ensure all paths are considered */
    switch (seed % 6) {
        case 0:
            #ifdef __SSE2__
            test_vector_shuffle(results);
            #endif
            break;
        case 1:
            #ifdef __AVX512F__
            test_avx512_gather(results);
            #endif
            break;
        case 2:
            test_atomic_ops(results);
            break;
        case 3:
            #ifdef _OPENMP
            test_openmp_simd(results, 128);
            #endif
            break;
        case 4:
            test_multi_operand_asm(results);
            break;
        case 5:
            #ifdef __ARM_NEON
            test_neon_operations(results);
            #endif
            break;
    }
    
    /* Compute final checksum to ensure all operations contribute */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += results[i];
        use((void*)&results[i]);  /* Keep values live */
    }
    
    printf("Checksum: %d (Seed: %d)\n", checksum, seed);
    return checksum != 0 ? 0 : 1;
}

/* Dummy use function to prevent dead code elimination */
void use(void* ptr) {
    /* Empty but referenced */
    (void)ptr;
}
