/* test_optabs_coverage.c - Cover 10/11-operand expansion cases in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Prevent interprocedural optimization */
#define NOIPA __attribute__((noipa, noinline, noclone))

/* External function to keep values live */
extern void use(void *);

/* ==================== PATTERN 1: Vector shuffles with many operands ==================== */

NOIPA void test_vector_shuffle_10_operands(void) {
    /* Use AVX-512 style 512-bit vectors (16 x 32-bit) */
    typedef int v16si __attribute__((vector_size(64)));
    typedef int v32si __attribute__((vector_size(128)));
    
    volatile v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    volatile v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* __builtin_shufflevector with 32 indices - may expand to many operands */
    v32si combined = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
    
    use(&combined);
}

/* ==================== PATTERN 2: x86 AVX-512 gather intrinsics ==================== */

#ifdef __x86_64__
#include <x86intrin.h>

NOIPA void test_avx512_gather_11_operands(void) {
    /* AVX-512 gather instruction with many parameters */
    volatile __m512d base = _mm512_set1_pd(1.0);
    volatile __m256i index = _mm256_set1_epi64x(0);
    volatile __mmask8 mask = 0xFF;
    volatile __m512d scale = _mm512_set1_pd(1.0);
    
    /* __builtin_ia32_gathersiv8df takes 11 arguments in GCC's internal representation:
       result, base, index, scale, mask, src, rounding, etc. */
    double *addr = (double*)malloc(64 * sizeof(double));
    for (int i = 0; i < 64; i++) addr[i] = i * 1.5;
    
    __m512d result = _mm512_i64gather_pd(index, addr, 8);
    use(&result);
    free(addr);
}
#endif

/* ==================== PATTERN 3: Atomic operations with many parameters ==================== */

NOIPA void test_atomic_compare_exchange(void) {
    volatile intptr_t atomic_var = 0;
    intptr_t expected = 0;
    intptr_t desired = 42;
    volatile int weak = 0;
    
    /* __atomic_compare_exchange with all parameters specified */
    __atomic_compare_exchange(&atomic_var, &expected, &desired, 
                              weak, /* weak */
                              __ATOMIC_SEQ_CST, /* success_memorder */
                              __ATOMIC_ACQUIRE); /* failure_memorder */
    
    use(&atomic_var);
    use(&expected);
}

/* ==================== PATTERN 4: OpenMP SIMD with complex clauses ==================== */

NOIPA void test_openmp_simd_many_clauses(void) {
    #define N 1024
    volatile double a[N], b[N], c[N], d[N];
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0;
        b[i] = i * 2.0;
        c[i] = i * 3.0;
        d[i] = 0.0;
    }
    
    /* OpenMP SIMD with many clauses - expands to multi-operand SIMD operations */
    #pragma omp simd linear(i:1) aligned(a,b,c,d:64) \
                simdlen(16) safelen(32) reduction(+:d[0:N])
    for (int i = 0; i < N; i++) {
        d[i] = a[i] * b[i] + c[i];
    }
    
    use(d);
}

/* ==================== PATTERN 5: Inline assembly with 10-11 operands ==================== */

NOIPA void test_multi_operand_asm(void) {
    volatile int out1, out2;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* 10-operand template */\n\t"
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
    
    use(&out1);
    use(&out2);
}

/* ==================== PATTERN 6: ARM/AArch64 NEON multi-register operations ==================== */

#ifdef __aarch64__
#include <arm_neon.h>

NOIPA void test_aarch64_multi_reg_ops(void) {
    /* AArch64 load/store multiple registers - can expand to many operands */
    volatile int32x4x4_t data;
    volatile int32_t *ptr = (int32_t*)malloc(64 * sizeof(int32_t));
    
    for (int i = 0; i < 64; i++) ptr[i] = i;
    
    /* __builtin_aarch64_ld1x4 intrinsic (if available) or equivalent */
    data = vld1q_s32_x4(ptr);
    
    /* Modify and store back */
    data.val[0] = vaddq_s32(data.val[0], vdupq_n_s32(1));
    vst1q_s32_x4(ptr, data);
    
    use(ptr);
    free(ptr);
}
#endif

/* ==================== PATTERN 7: Complex ternary/quadrary operations ==================== */

NOIPA void test_ternary_operations(void) {
    /* Use GCC vector extensions with conditional operations */
    typedef float v8sf __attribute__((vector_size(32)));
    
    volatile v8sf x = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    volatile v8sf y = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    volatile v8sf mask = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    volatile v8sf z = {0.0};
    
    /* Complex expression that may expand to many operands */
    z = __builtin_shuffle(x, y, 
        __builtin_convertvector(mask > 0.5, v8sf) * 
        (v8sf){0,1,2,3,4,5,6,7} + 
        (v8sf){8,9,10,11,12,13,14,15});
    
    use(&z);
}

/* ==================== Main driver with volatile control flow ==================== */

int main(int argc, char *argv[]) {
    /* Create volatile seed from argv[0] */
    volatile unsigned seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Use seed to select which patterns to execute */
    volatile unsigned choice = seed % 7;
    
    /* Execute different patterns based on runtime value */
    switch (choice) {
        case 0:
            test_vector_shuffle_10_operands();
            break;
        #ifdef __x86_64__
        case 1:
            test_avx512_gather_11_operands();
            break;
        #endif
        case 2:
            test_atomic_compare_exchange();
            break;
        case 3:
            test_openmp_simd_many_clauses();
            break;
        case 4:
            test_multi_operand_asm();
            break;
        #ifdef __aarch64__
        case 5:
            test_aarch64_multi_reg_ops();
            break;
        #endif
        case 6:
            test_ternary_operations();
            break;
        default:
            /* Fallback to ensure all are compiled */
            test_vector_shuffle_10_operands();
            test_atomic_compare_exchange();
            test_multi_operand_asm();
    }
    
    /* Compute checksum to ensure execution */
    volatile int checksum = (int)seed;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy use function to prevent optimization */
void use(void *ptr) {
    asm volatile ("" : : "r"(ptr) : "memory");
}
