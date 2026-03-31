/* test_optabs_coverage.c - Cover 10/11-operand expansion cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ==================== PATTERN 1: Vector shuffles with many elements ==================== */

#ifdef __AVX512F__
typedef int32_t v16si __attribute__((vector_size(64)));
typedef int64_t v8di __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle_10(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Shuffle with 16 indices = 2 vectors + 16 indices = 18 operands in RTL */
    v16si c = __builtin_shufflevector(a, b, 
        0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
    
    /* Another complex shuffle pattern */
    v16si d = __builtin_shufflevector(a, b,
        15,14,13,12,11,10,9,8,31,30,29,28,27,26,25,24);
    
    /* Mix them based on control */
    v16si r = control ? c : d;
    memcpy((void*)result, &r, sizeof(r));
    use((void*)result);
}
#endif

/* ==================== PATTERN 2: AVX-512 gather intrinsics (many operands) ==================== */

#ifdef __AVX512F__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_gather_intrinsic_11(volatile double* result) {
    __m512d src = _mm512_set1_pd(3.14);
    __m512i index = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __mmask8 mask = 0xFF;
    double* base = (double*)result;
    
    /* __builtin_ia32_gathersiv8df expands to many operands:
       src, base, index, scale, mask, etc. */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, index, base, 8);
    
    /* Force use */
    _mm512_storeu_pd((double*)result, gathered);
    use((void*)result);
}
#endif

/* ==================== PATTERN 3: Atomic operations with many parameters ==================== */

__attribute__((noipa, noinline))
void test_atomic_compare_exchange_10(volatile int* result) {
    int expected = 42;
    int desired = 100;
    int* ptr = (int*)result;
    
    /* __atomic_compare_exchange with all parameters:
       ptr, expected, desired, weak, success_memorder, failure_memorder */
    int weak = control & 1;
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
    
    *result = expected;
    use((void*)result);
}

/* ==================== PATTERN 4: OpenMP SIMD with many clauses ==================== */

__attribute__((noipa, noinline))
void test_openmp_simd_11(volatile int* result, int n) {
    int a[128], b[128], c[128];
    
    for (int i = 0; i < 128; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* OpenMP SIMD with multiple clauses that increase operand count */
    #pragma omp simd linear(i:1) aligned(a,b,c:64) simdlen(8) safelen(16) \
                    reduction(+:c[0:128]) if(control)
    for (int i = 0; i < n && i < 128; i++) {
        c[i] = a[i] + b[i] + control;
    }
    
    memcpy((void*)result, c, sizeof(c));
    use((void*)result);
}

/* ==================== PATTERN 5: Inline assembly with 10-11 operands ==================== */

__attribute__((noipa, noinline))
void test_multi_operand_asm_10(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out;
    
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
        : "=r"(out)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    *result = out + j;  /* Makes 11 values total */
    use((void*)result);
}

__attribute__((noipa, noinline))
void test_multi_operand_asm_11(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int out;
    
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
        : "=r"(out)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    *result = out + k;  /* 12th value for checksum */
    use((void*)result);
}

/* ==================== PATTERN 6: Complex builtin with many arguments ==================== */

#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((noipa, noinline))
void test_neon_ld4_11(volatile int* result) {
    /* ARM NEON ld4 intrinsic loads 4 registers - expands to many operands */
    int32_t data[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int32x4x4_t v = vld4q_s32(data);
    
    /* Use all results */
    int32x4_t sum = vaddq_s32(v.val[0], v.val[1]);
    sum = vaddq_s32(sum, v.val[2]);
    sum = vaddq_s32(sum, v.val[3]);
    
    vst1q_s32((int32_t*)result, sum);
    use((void*)result);
}
#endif

/* ==================== Main execution driver ==================== */

int main(int argc, char *argv[]) {
    /* Create volatile result arrays */
    volatile int results[256] = {0};
    volatile double dresults[128] = {0};
    
    /* Simple hash from argv[0] for control */
    unsigned seed = 0;
    if (argc > 0 && argv[0]) {
        for (char *p = argv[0]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    control = seed;
    
    int checksum = 0;
    
    /* Execute different patterns based on seed */
    switch (seed % 7) {
        case 0:
            #ifdef __AVX512F__
            test_vector_shuffle_10(results);
            #endif
            break;
        case 1:
            #ifdef __AVX512F__
            test_gather_intrinsic_11(dresults);
            #endif
            break;
        case 2:
            test_atomic_compare_exchange_10(results);
            break;
        case 3:
            test_openmp_simd_11(results, 64 + (seed % 64));
            break;
        case 4:
            test_multi_operand_asm_10(results);
            break;
        case 5:
            test_multi_operand_asm_11(results);
            break;
        case 6:
            #ifdef __ARM_NEON
            test_neon_ld4_11(results);
            #endif
            break;
    }
    
    /* Compute checksum to ensure execution */
    for (int i = 0; i < 256; i++) {
        checksum ^= results[i];
    }
    
    printf("Checksum: %d (control: %d)\n", checksum, control);
    
    return 0;
}
