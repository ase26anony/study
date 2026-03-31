/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Enable target-specific intrinsics based on architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <x86intrin.h>
#define TARGET_X86 1
#elif defined(__aarch64__)
#include <arm_neon.h>
#include <arm_acle.h>
#define TARGET_AARCH64 1
#endif

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
void test_vector_intrinsics() {
#if TARGET_X86 && defined(__AVX512F__)
    /* AVX-512 mask operations can generate many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __m512 d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;
    
    /* Complex FMA chain with masking - may expand to many operands */
    __m512 result = _mm512_mask_fmadd_ps(a, mask, b, c);
    result = _mm512_mask_fmadd_ps(result, mask, c, d);
    result = _mm512_mask_fmadd_ps(result, mask, d, a);
    
    /* Store to prevent elimination */
    volatile __m512 store_var = result;
    
#elif TARGET_AARCH64
    /* ARM NEON/SVE operations with lane selection */
    float32x4_t v1 = vdupq_n_f32(1.0f);
    float32x4_t v2 = vdupq_n_f32(2.0f);
    float32x4_t v3 = vdupq_n_f32(3.0f);
    float32x4_t v4 = vdupq_n_f32(4.0f);
    
    /* Complex FMA operations */
    float32x4_t result = vfmaq_f32(v1, v2, v3);
    result = vfmaq_f32(result, v3, v4);
    result = vfmaq_f32(result, v4, v1);
    
    volatile float32x4_t store_var = result;
#endif
}

/* Complex reduction with GCC vector extensions */
__attribute__((hot, noinline))
v4df test_vector_reduction(v4df *arr, int n) {
    v4df sum = {0.0, 0.0, 0.0, 0.0};
    v4df prod = {1.0, 1.0, 1.0, 1.0};
    
    for (int i = 0; i < n; i++) {
        /* Complex expression that may generate multi-operand patterns */
        v4df temp = arr[i] * arr[i] + arr[i];
        sum = sum + temp;
        prod = prod * temp;
        
        /* FMA-like operation using GCC builtins */
        sum = sum + __builtin_fma(arr[i], arr[i], arr[i]);
    }
    
    /* Mix results to create dependency */
    return sum + prod;
}

/* Inline assembly with many operands - directly tests operand handling */
__attribute__((noinline))
void test_multi_operand_asm() {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2, result3;
    
    /* Extended asm with 11 operands (3 outputs, 8 inputs) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %2, %8\n\t"
        "imul %0, %0, %9\n\t"
        "imul %1, %1, %10\n\t"
        "imul %2, %2, %11"
        : "=&r" (result1), "=&r" (result2), "=&r" (result3)
        : "r" (op1), "r" (op2), "r" (op3), "r" (op4),
          "r" (op5), "r" (op6), "r" (op7), "r" (op8),
          "r" (op9)
        : "cc"
    );
    
    volatile long dummy = result1 + result2 + result3;
}

/* OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
__attribute__((hot, noinline))
v8sf test_omp_reduction(v8sf *data, int n) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex operation that may expand to many operands */
        v8sf temp = data[i] * data[i] + data[i];
        sum = sum + temp;
    }
    
    return sum;
}
#endif

/* Complex builtin usage */
__attribute__((hot, noinline))
double test_complex_builtins(double a, double b, double c, double d) {
    /* Chain of FMA operations - may expand to multi-operand patterns */
    double result = __builtin_fma(a, b, c);
    result = __builtin_fma(result, c, d);
    result = __builtin_fma(result, d, a);
    result = __builtin_fma(result, a, b);
    
    /* Complex math builtins */
    result = __builtin_pow(result, 2.0);
    result = __builtin_sin(result) + __builtin_cos(result);
    
    return result;
}

int main() {
    /* Initialize test data */
    const int N = 100;
    v4df vec_array[N];
    v8sf omp_data[N];
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) vec_array[i][j] = (i + j) * 0.1;
        for (int j = 0; j < 8; j++) omp_data[i][j] = (i + j) * 0.1f;
    }
    
    /* Execute all test patterns */
    test_vector_intrinsics();
    
    v4df vec_result = test_vector_reduction(vec_array, N);
    volatile double dummy1 = vec_result[0] + vec_result[1];
    
    test_multi_operand_asm();
    
    #ifdef _OPENMP
    v8sf omp_result = test_omp_reduction(omp_data, N);
    volatile float dummy2 = omp_result[0] + omp_result[1];
    #endif
    
    double builtin_result = test_complex_builtins(1.1, 2.2, 3.3, 4.4);
    volatile double dummy3 = builtin_result;
    
    printf("Test completed successfully\n");
    return 0;
}
