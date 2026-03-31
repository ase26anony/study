/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the uncovered 10 and 11-operand cases
 * in GCC's optabs.cc by generating code that requires complex multi-operand
 * instruction patterns during RTL expansion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* External function to prevent optimization */
extern void use(void*);

/* Volatile control to prevent dead code elimination */
static volatile int control = 0;

/* ========== Technique 1: Vector Extensions with Many Operands ========== */

/* Large vector types for shuffle operations */
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef long long v8di __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noipa, noinline))
void test_vector_shuffle_10_operands(volatile int* result) {
    v16si a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v16si b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    
    /* Complex shuffle with many constant indices - may expand to many operands */
    v16si c = __builtin_shufflevector(a, b, 
        0, 1, 2, 3, 4, 5, 6, 7,    /* First 8 from a */
        16, 17, 18, 19, 20, 21, 22, 23); /* Next 8 from b */
    
    /* Another shuffle with different pattern */
    v16si d = __builtin_shufflevector(a, b,
        15, 14, 13, 12, 11, 10, 9, 8,
        31, 30, 29, 28, 27, 26, 25, 24);
    
    /* Complex operation combining results */
    v16si e = c + d;
    
    /* Extract results to memory */
    for (int i = 0; i < 16; i++) {
        result[i] = e[i];
    }
    
    use(&e);
}

/* ========== Technique 2: Target-Specific Built-in Functions ========== */

#ifdef __x86_64__
#include <x86intrin.h>

__attribute__((noipa, noinline))
void test_avx512_gather_11_operands(volatile int* result) {
    /* AVX-512 gather instructions can have many operands */
    __m512i index = _mm512_set_epi32(0, 4, 8, 12, 16, 20, 24, 28,
                                     32, 36, 40, 44, 48, 52, 56, 60);
    __mmask16 mask = 0xFFFF;
    int scale = 4;
    
    /* Simulate gather-like operation with many parameters */
    int* base = (int*)result;
    __m512i gathered;
    
    /* Complex memory operation pattern that might expand to many operands */
    for (int i = 0; i < 16; i++) {
        int idx = index[i];
        if (mask & (1 << i)) {
            result[i] = base[idx / scale];
        }
    }
    
    /* Use AVX-512 intrinsic if available */
#if defined(__AVX512F__)
    /* __m512i _mm512_mask_i32gather_epi32(__m512i src, __mmask16 k, __m512i vindex, void const *base, int scale); */
    /* This intrinsic takes multiple parameters that could expand to many operands */
    gathered = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask, index, base, scale);
    use(&gathered);
#endif
}
#endif

/* ========== Technique 3: Atomic Operations with Multiple Parameters ========== */

__attribute__((noipa, noinline))
void test_atomic_compare_exchange_10_operands(volatile int* result) {
    /* __atomic_compare_exchange has many parameters */
    int expected = 42;
    int desired = 84;
    int weak = 0;
    
    /* Complex atomic operation with multiple memory orders */
    for (int i = 0; i < 8; i++) {
        int* ptr = (int*)&result[i];
        int local_expected = expected + i;
        
        /* This builtin has many parameters: ptr, expected, desired, weak, success_memorder, failure_memorder */
        __atomic_compare_exchange(ptr, &local_expected, &desired, 
                                 weak, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
        
        result[i + 8] = local_expected;
    }
}

/* ========== Technique 4: OpenMP SIMD Constructs with Complex Clauses ========== */

__attribute__((noipa, noinline))
void test_openmp_simd_many_clauses(volatile int* result, volatile int* input1, 
                                   volatile int* input2, int n) {
    /* OpenMP SIMD with many clauses can create complex multi-operand patterns */
    int i;
    
#pragma omp simd linear(i:1) aligned(result,input1,input2:64) \
                 simdlen(8) safelen(16) private(i) \
                 reduction(+:result[0])
    for (i = 0; i < n; i++) {
        /* Complex operation that might require many operands */
        result[i] = input1[i] * input2[i] + 
                   input1[i+1] - input2[i+1] +
                   (input1[i] >> 2) | (input2[i] << 2);
    }
}

/* ========== Technique 5: Custom Many-Argument Inline Assembly ========== */

__attribute__((noipa, noinline))
void test_multi_operand_asm_11_operands(volatile int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2, out3;
    
    /* 10-operand asm statement */
    asm volatile (
        "/* Multi-operand asm template %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 */\n"
        "add %0, %1, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9"
        : "=r" (out1)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), 
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    /* 11-operand asm statement with memory operand */
    asm volatile (
        "/* 11-operand asm with memory %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 */\n"
        "mov %0, [%1]\n"
        "add %0, %0, %2\n"
        "add %0, %0, %3\n"
        "add %0, %0, %4\n"
        "add %0, %0, %5\n"
        "add %0, %0, %6\n"
        "add %0, %0, %7\n"
        "add %0, %0, %8\n"
        "add %0, %0, %9\n"
        "mov [%10], %0"
        : "=&r" (out2)
        : "r" (result), "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (&out3)
        : "memory", "cc"
    );
    
    result[0] = out1 + out2 + out3;
}

/* ========== Technique 6: Complex Vector Operations ========== */

__attribute__((noipa, noinline))
void test_complex_vector_operations(volatile float* result_f, 
                                   volatile double* result_d) {
    /* Mixed vector operations that might require many operands */
    v16sf vf1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf vf2 = vf1 * 2.0f;
    v16sf vf3 = vf1 + vf2;
    v16sf vf4 = __builtin_shufflevector(vf1, vf2, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    /* Fused multiply-add style operations */
    v16sf vf5 = vf1 * vf2 + vf3;
    v16sf vf6 = vf4 - vf1 * 0.5f;
    
    /* Store results */
    for (int i = 0; i < 16; i++) {
        result_f[i] = vf5[i] + vf6[i];
    }
    
    /* Double precision vectors */
    v8df vd1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df vd2 = vd1 * 3.14;
    v8df vd3 = __builtin_shufflevector(vd1, vd2, 0, 8, 1, 9, 2, 10, 3, 11);
    
    for (int i = 0; i < 8; i++) {
        result_d[i] = vd3[i];
    }
    
    use(&vf5);
    use(&vd3);
}

/* ========== Main Test Driver ========== */

int main(int argc, char *argv[]) {
    /* Use argv[0] to create a volatile seed */
    volatile unsigned long seed = 0;
    for (int i = 0; argv[0][i]; i++) {
        seed = seed * 31 + argv[0][i];
    }
    
    /* Allocate test arrays with alignment */
    volatile int* int_results = aligned_alloc(64, 256 * sizeof(int));
    volatile float* float_results = aligned_alloc(64, 256 * sizeof(float));
    volatile double* double_results = aligned_alloc(64, 256 * sizeof(double));
    volatile int* input1 = aligned_alloc(64, 256 * sizeof(int));
    volatile int* input2 = aligned_alloc(64, 256 * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        int_results[i] = 0;
        if (i < 256) float_results[i] = 0.0f;
        if (i < 256) double_results[i] = 0.0;
        input1[i] = i;
        input2[i] = 255 - i;
    }
    
    /* Select test based on seed */
    int test_case = seed % 6;
    
    switch (test_case) {
        case 0:
            test_vector_shuffle_10_operands(int_results);
            break;
        case 1:
#ifdef __x86_64__
            test_avx512_gather_11_operands(int_results);
#else
            test_atomic_compare_exchange_10_operands(int_results);
#endif
            break;
        case 2:
            test_atomic_compare_exchange_10_operands(int_results);
            break;
        case 3:
            test_openmp_simd_many_clauses(int_results, input1, input2, 128);
            break;
        case 4:
            test_multi_operand_asm_11_operands(int_results);
            break;
        case 5:
            test_complex_vector_operations(float_results, double_results);
            break;
    }
    
    /* Compute checksum to ensure code executed */
    unsigned long checksum = 0;
    for (int i = 0; i < 128; i++) {
        checksum += int_results[i];
        checksum += (unsigned long)float_results[i];
        if (i < 64) checksum += (unsigned long)double_results[i];
    }
    
    printf("Test case %d executed. Checksum: %lu\n", test_case, checksum);
    
    /* Cleanup */
    free((void*)int_results);
    free((void*)float_results);
    free((void*)double_results);
    free((void*)input1);
    free((void*)input2);
    
    return 0;
}
