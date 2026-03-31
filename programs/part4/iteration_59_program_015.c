/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Strategy 1: Use target-specific vector intrinsics with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Strategy 2: GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
void test_vector_operations(float* result) {
    /* Strategy 1a: AVX-512 masked operations with many operands */
#ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __m512 d = _mm512_set1_ps(4.0f);
        __mmask16 mask = 0xAAAA;
        
        /* This may expand to a pattern with many operands:
           mask, a, b, c, d, rounding mode, etc. */
        __m512 res = _mm512_mask_fmadd_ps(a, mask, b, c);
        _mm512_storeu_ps(result, res);
        
        /* Another complex operation with many operands */
        __m512 res2 = _mm512_mask3_fmadd_ps(a, b, c, mask);
        _mm512_storeu_ps(result + 16, res2);
    }
#endif

    /* Strategy 1b: ARM NEON multi-vector operations */
#ifdef __ARM_NEON
    {
        float32x4_t v1 = vdupq_n_f32(1.0f);
        float32x4_t v2 = vdupq_n_f32(2.0f);
        float32x4_t v3 = vdupq_n_f32(3.0f);
        float32x4_t v4 = vdupq_n_f32(4.0f);
        
        /* Complex FMA-like operations that may require many operands */
        float32x4_t res = vfmaq_laneq_f32(v1, v2, v3, 1);
        vst1q_f32(result, res);
        
        /* Table lookup operations can have many operands */
        uint8x16_t tbl = vdupq_n_u8(0);
        uint8x16_t indices = vdupq_n_u8(0);
        uint8x16x2_t table = {tbl, tbl};
        uint8x16_t res2 = vqtbl2q_u8(table, indices);
        memcpy(result + 4, &res2, 16);
    }
#endif

    /* Strategy 2: GCC vector extensions with complex expressions */
    {
        v8sf va = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf vb = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        v8sf vc = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        v8sf vd = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
        
        /* Complex expression that might generate multi-operand instructions */
        v8sf ve = va * vb + vc * vd;
        memcpy(result + 32, &ve, sizeof(v8sf));
    }
}

/* Strategy 3: Inline assembly with many operands */
__attribute__((noinline))
void test_many_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    unsigned long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    unsigned long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    unsigned long op11 = 11;
    unsigned long out1, out2, out3;
    
    /* Extended asm with 11 total operands (3 outputs, 8 inputs) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %2, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %1, %1, %10\n\t"
        "add %2, %2, %11"
        : "=&r" (out1), "=&r" (out2), "=&r" (out3)
        : "r" (op1), "r" (op2), "r" (op3), "r" (op4), 
          "r" (op5), "r" (op6), "r" (op7), "r" (op8),
          "r" (op9), "r" (op10)
        : "cc"
    );
    
    /* Use results to prevent optimization */
    printf("ASM results: %lu %lu %lu\n", out1, out2, out3);
}

/* Strategy 4: Built-in functions for complex math */
__attribute__((noinline))
double test_builtin_fma(void) {
    /* Use __builtin_fma which takes 3 arguments and may expand 
       to multi-operand FMA instruction */
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
    
    /* Chain multiple FMA operations to create complex patterns */
    double res1 = __builtin_fma(a, b, c);
    double res2 = __builtin_fma(res1, d, e);
    double res3 = __builtin_fma(a, res2, __builtin_fma(b, c, d));
    
    return res1 + res2 + res3;
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
__attribute__((noinline))
float test_omp_reduction(void) {
    #define N 1024
    float array[N];
    float sum = 0.0f;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = (float)i;
    }
    
    /* OpenMP SIMD reduction that may generate multi-operand patterns */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += array[i] * 2.0f - array[i] / 3.0f;
    }
    
    return sum;
}
#endif

int main(void) {
    float* buffer = aligned_alloc(64, 1024 * sizeof(float));
    if (!buffer) return 1;
    
    /* Test 1: Vector operations with intrinsics */
    test_vector_operations(buffer);
    
    /* Test 2: Inline assembly with many operands */
    test_many_operand_asm();
    
    /* Test 3: Built-in FMA functions */
    double fma_result = test_builtin_fma();
    printf("FMA result: %f\n", fma_result);
    
    /* Test 4: OpenMP reduction */
    #ifdef _OPENMP
    float omp_result = test_omp_reduction();
    printf("OMP reduction result: %f\n", omp_result);
    #endif
    
    /* Use buffer to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < 64; i++) {
        checksum += buffer[i];
    }
    printf("Buffer checksum: %f\n", checksum);
    
    free(buffer);
    return 0;
}
