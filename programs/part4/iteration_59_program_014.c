/* Test program to cover 10 and 11 operand cases in optabs.cc */
/* Compile with: gcc -O3 -mavx512f -mfma -ftree-vectorize -fopenmp -c test.c -o test.o */

#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>

/* GCC vector extensions for complex operations */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Hot function to encourage complex instruction patterns */
__attribute__((hot, noinline))
void test_avx512_multi_operand() {
    /* AVX-512 intrinsics with masks can generate many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __m512 d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;  /* Alternating mask */
    
    /* Fused multiply-add with mask - can expand to many operands */
    __m512 result1 = _mm512_mask_fmadd_ps(a, mask, b, c);
    
    /* Complex blend operation */
    __m512 result2 = _mm512_mask_blend_ps(mask, result1, d);
    
    /* Store to prevent elimination */
    volatile __m512 store_var = result2;
    (void)store_var;
}

/* Test with GCC vector extensions */
__attribute__((hot, noinline))
void test_gcc_vector_extensions() {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f,
                 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    v16sf v3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f,
                 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f};
    
    /* Complex expression that might generate multi-operand instructions */
    v16sf result = v1 * v2 + v3;
    result = result * v1 - v2;
    
    /* Use builtin fma to encourage complex patterns */
    for (int i = 0; i < 16; i++) {
        result[i] = __builtin_fmaf(result[i], v1[i], v2[i]);
    }
    
    volatile v16sf store_var = result;
    (void)store_var;
}

/* Inline assembly with many operands */
__attribute__((hot, noinline))
void test_multi_operand_asm() {
    /* 11-operand inline assembly */
    int out1, out2, out3;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9;
    
    asm volatile (
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "mov %4, %5\n\t"
        "add %4, %6\n\t"
        "mov %7, %8\n\t"
        : "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8), "r"(in9)
        : "cc"
    );
    
    volatile int store_var = out1 + out2 + out3;
    (void)store_var;
}

/* OpenMP SIMD reduction with vector types */
__attribute__((hot, noinline))
float test_omp_reduction() {
    float array[1024];
    for (int i = 0; i < 1024; i++) {
        array[i] = (float)i;
    }
    
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 1024; i++) {
        sum += array[i] * array[i];
    }
    
    return sum;
}

/* Test ARM NEON style intrinsics if compiled for ARM */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((hot, noinline))
void test_neon_multi_operand() {
    /* ARM NEON table lookup can generate many operands */
    uint8x16_t data = vdupq_n_u8(1);
    uint8x16_t table1 = vdupq_n_u8(2);
    uint8x16_t table2 = vdupq_n_u8(3);
    uint8x16_t table3 = vdupq_n_u8(4);
    uint8x16_t table4 = vdupq_n_u8(5);
    
    /* Complex permute/table lookup */
    uint8x16x4_t tables = {table1, table2, table3, table4};
    uint8x16_t result = vqtbl4q_u8(tables, data);
    
    volatile uint8x16_t store_var = result;
    (void)store_var;
}
#endif

/* Main function that calls all tests */
int main() {
    test_avx512_multi_operand();
    test_gcc_vector_extensions();
    test_multi_operand_asm();
    
    float result = test_omp_reduction();
    printf("Reduction result: %f\n", result);
    
    #ifdef __ARM_NEON
    test_neon_multi_operand();
    #endif
    
    return 0;
}
