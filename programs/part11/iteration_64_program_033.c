/* test_optabs_coverage.c - Cover 10/11-operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Pattern 1: Vector shuffles with many operands ========== */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices = potentially many operands during expansion */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another complex shuffle pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* AVX-512 specific permute with mask */
    v16si mask = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    v16si e = __builtin_ia32_permvarhi512_mask(a, mask, b, 0xFFFF);
    
    /* Store results to volatile memory */
    memcpy((void*)result, &c, sizeof(c));
    memcpy((void*)(result + 16), &d, sizeof(d));
    memcpy((void*)(result + 32), &e, sizeof(e));
    
    use((void*)result);
}
#endif

/* ========== Pattern 2: Gather intrinsics (x86 AVX-512) ========== */
#ifdef __AVX512F__
__attribute__((noipa, noinline))
void test_gather_intrinsic(volatile double* result) {
    double base[64];
    int32_t index[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    v8df src = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    __mmask8 mask = 0xFF;
    
    /* Initialize base array */
    for (int i = 0; i < 64; i++) {
        base[i] = i * 1.5;
    }
    
    /* __builtin_ia32_gathersiv8df expands to many operands:
       result, base, scale, index, mask, src */
    v8df res = __builtin_ia32_gathersiv8df(src, (const double*)base,
                                          index, mask, 1);
    
    /* Another gather variant with different parameters */
    v8df res2 = __builtin_ia32_gatherdiv8df(src, (const double*)base,
                                           (long long*)index, mask, 2);
    
    memcpy((void*)result, &res, sizeof(res));
    memcpy((void*)(result + 8), &res2, sizeof(res2));
    
    use((void*)result);
}
#endif

/* ========== Pattern 3: Atomic operations with many parameters ========== */
__attribute__((noipa, noinline))
void test_atomic_operations(volatile int* result) {
    intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    intptr_t weak_expected = 100;
    intptr_t weak_desired = 200;
    
    /* __atomic_compare_exchange with many parameters */
    int success = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                           0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Weak version with different memory orders */
    int weak_success = __atomic_compare_exchange_n(&atomic_var, &weak_expected,
                                                  weak_desired, 1,
                                                  __ATOMIC_ACQ_REL,
                                                  __ATOMIC_ACQUIRE);
    
    /* __atomic_fetch_add with memory order */
    intptr_t fetch_result = __atomic_fetch_add(&atomic_var, 10, __ATOMIC_SEQ_CST);
    
    result[0] = success;
    result[1] = weak_success;
    result[2] = (int)fetch_result;
    result[3] = (int)atomic_var;
    
    use((void*)result);
}

/* ========== Pattern 4: OpenMP SIMD with complex clauses ========== */
#ifdef _OPENMP
__attribute__((noipa, noinline))
void test_openmp_simd(volatile double* result) {
    const int N = 128;
    double a[N] __attribute__((aligned(64)));
    double b[N] __attribute__((aligned(64)));
    double c[N] __attribute__((aligned(64)));
    double d[N] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.5;
        b[i] = i * 1.5;
        c[i] = i * 2.5;
        d[i] = 0.0;
    }
    
    /* Complex OpenMP SIMD pragma with many clauses */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(8) safelen(16) reduction(+:control)
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * b[i] + c[i];
    }
    
    /* Another SIMD loop with different clauses */
    #pragma omp simd aligned(a,d:64) simdlen(16) collapse(1) \
                private(i) lastprivate(j)
    for (int i = 0; i < N; i++) {
        a[i] = d[i] * 2.0;
    }
    
    memcpy((void*)result, d, sizeof(double) * 16);
    memcpy((void*)(result + 16), a, sizeof(double) * 16);
    
    use((void*)result);
}
#endif

/* ========== Pattern 5: Multi-operand inline assembly ========== */
__attribute__((noipa, noinline))
void test_multi_operand_asm(volatile int* result) {
    int out1, out2;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* 10-operand test */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* 11-operand asm statement */
    int k = 11;
    asm volatile (
        "/* 11-operand test */\n\t"
        "mov %0, %1\n\t"
        "imul %0, %2\n\t"
        "add %0, %3\n\t"
        "sub %0, %4\n\t"
        "and %0, %5\n\t"
        "or %0, %6\n\t"
        "xor %0, %7\n\t"
        "add %0, %8\n\t"
        "sub %0, %9\n\t"
        "add %0, %10"
        : "=r" (out2)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    result[0] = out1;
    result[1] = out2;
    
    use((void*)result);
}

/* ========== Pattern 6: AArch64-specific NEON multi-register ops ========== */
#ifdef __aarch64__
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_aarch64_neon(volatile int* result) {
    int32x4_t a = {1, 2, 3, 4};
    int32x4_t b = {5, 6, 7, 8};
    int32x4_t c = {9, 10, 11, 12};
    int32x4_t d = {13, 14, 15, 16};
    
    /* Complex NEON operations that may expand to many operands */
    int32x4_t e = vaddq_s32(a, b);
    int32x4_t f = vmlaq_s32(c, d, e);
    
    /* Matrix transpose-like operations */
    int32x4x2_t g = vzipq_s32(a, b);
    int32x4x4_t h = {a, b, c, d};
    
    /* Store multiple registers */
    int32_t buffer[16];
    vst1q_s32(buffer, a);
    vst1q_s32(buffer + 4, b);
    vst1q_s32(buffer + 8, c);
    vst1q_s32(buffer + 12, d);
    
    /* Load with deinterleave */
    int32x4x2_t i = vld2q_s32(buffer);
    
    memcpy((void*)result, buffer, sizeof(buffer));
    memcpy((void*)(result + 16), &i.val[0], sizeof(int32x4_t));
    memcpy((void*)(result + 20), &i.val[1], sizeof(int32x4_t));
    
    use((void*)result);
}
#endif

/* ========== Main test driver ========== */
int main(int argc, char *argv[]) {
    /* Create volatile result arrays */
    volatile int int_results[256] = {0};
    volatile double double_results[256] = {0};
    
    /* Simple hash from argv[0] for control flow */
    unsigned seed = 0;
    if (argc > 0) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    control = seed;
    
    /* Execute different patterns based on seed */
    switch (control % 6) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle(int_results);
            #endif
            break;
        case 1:
            #ifdef __AVX512F__
            test_gather_intrinsic(double_results);
            #endif
            break;
        case 2:
            test_atomic_operations(int_results);
            break;
        case 3:
            #ifdef _OPENMP
            test_openmp_simd(double_results);
            #endif
            break;
        case 4:
            test_multi_operand_asm(int_results);
            break;
        case 5:
            #ifdef __aarch64__
            test_aarch64_neon(int_results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure code executed */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum ^= int_results[i];
        checksum += (int)double_results[i];
    }
    
    printf("Checksum: %d (control: %d)\n", checksum, control);
    
    return 0;
}
