/* Test program to cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Force inline expansion */
#define ALWAYS_INLINE __attribute__((always_inline)) inline

/* Complex vector operation using AVX-512 intrinsics */
ALWAYS_INLINE __m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                                           __m512 d, __mmask16 k, float imm) {
    /* This should expand to a pattern with many operands:
       - 4 input vectors (a, b, c, d)
       - 1 mask register (k)
       - 1 immediate constant (imm)
       - Multiple temporary registers during expansion
       Total operands in RTL expansion could reach 10+
    */
    __m512 t1 = _mm512_maskz_fmadd_ps(k, a, b, c);
    __m512 t2 = _mm512_mask_sub_ps(t1, k, d, _mm512_set1_ps(imm));
    __m512 t3 = _mm512_mask_add_ps(t2, k, t2, _mm512_set1_ps(imm * 2.0f));
    
    /* Complex blend with multiple operands */
    return _mm512_mask_blend_ps(k, t1, _mm512_fmadd_ps(t3, a, b));
}

/* Use GCC vector extensions for complex expressions */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

ALWAYS_INLINE v16sf test_gcc_vectors(v16sf a, v16sf b, v16sf c, 
                                     v16sf d, v16sf e, v16sf f) {
    /* Complex expression that might generate multi-operand patterns */
    v16sf t1 = a * b + c;
    v16sf t2 = d * e - f;
    v16sf t3 = t1 / (t2 + 1.0f);
    v16sf t4 = __builtin_ia32_rcp14ps512(t3);
    
    /* Nested FMA operations */
    v16sf result = t4;
    for (int i = 0; i < 4; i++) {
        result = __builtin_fma(result, a, b);
        result = __builtin_fma(result, c, d);
    }
    
    return result;
}

/* Inline assembly with many operands */
ALWAYS_INLINE void test_multi_operand_asm(void) {
    /* 11-operand inline asm statement */
    uint64_t out1, out2, out3, out4;
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6;
    
    asm volatile (
        "mov %0, %5\n\t"
        "add %0, %6\n\t"
        "mov %1, %7\n\t"
        "add %1, %8\n\t"
        "mov %2, %9\n\t"
        "add %2, %10\n\t"
        "mov %3, %11\n\t"
        : "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4)
        : "0"(out1), "r"(in1), "r"(in2), "r"(in3), 
          "r"(in4), "r"(in5), "r"(in6), "i"(7)
        : "cc", "memory"
    );
    
    /* Use results to prevent optimization */
    printf("ASM results: %lu %lu %lu %lu\n", out1, out2, out3, out4);
}

/* OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
void test_omp_reduction(void) {
    v16sf array[100];
    v16sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        /* Complex operation that might expand to multi-operand pattern */
        v16sf temp = array[i];
        temp = __builtin_fma(temp, temp, temp);
        temp = temp / (temp + 1.0f);
        sum = sum + temp;
    }
    
    /* Use sum to prevent dead code elimination */
    float* p = (float*)&sum;
    printf("Reduction: %f\n", p[0]);
}
#endif

/* Main test function */
__attribute__((hot)) 
int main(void) {
    /* Initialize test data */
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;
    
    /* Test AVX-512 multi-operand intrinsic */
    __m512 result1 = test_avx512_10_operand(avx_a, avx_b, avx_c, avx_d, mask, 5.0f);
    
    /* Test GCC vector extensions */
    v16sf gcc_a = {0};
    v16sf gcc_b = {1.0f};
    v16sf gcc_c = {2.0f};
    v16sf gcc_d = {3.0f};
    v16sf gcc_e = {4.0f};
    v16sf gcc_f = {5.0f};
    
    v16sf result2 = test_gcc_vectors(gcc_a, gcc_b, gcc_c, gcc_d, gcc_e, gcc_f);
    
    /* Test inline assembly with many operands */
    test_multi_operand_asm();
    
    #ifdef _OPENMP
    test_omp_reduction();
    #endif
    
    /* Use results to prevent dead code elimination */
    float* r1 = (float*)&result1;
    float* r2 = (float*)&result2;
    
    printf("Results: %f %f\n", r1[0], r2[0]);
    
    return 0;
}
