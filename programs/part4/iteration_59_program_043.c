/* test_optabs_10_11_operands.c
 * This test aims to cover the 10 and 11 operand cases in optabs.cc
 * by using AVX-512 intrinsics that expand to complex multi-operand instructions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

/* Force inline expansion of intrinsics */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Complex AVX-512 operation that likely expands to many operands */
FORCE_INLINE __m512 test_avx512_10_operands(__m512 a, __m512 b, __m512 c, 
                                           __mmask16 k, __m512 src) {
    /* _mm512_mask3_fmadd_ps has 5 explicit operands but expands to more during RTL generation */
    __m512 result = _mm512_mask3_fmadd_ps(a, b, c, k, src);
    
    /* Additional masked operation to increase operand count in the expansion */
    result = _mm512_mask_add_ps(result, k, result, _mm512_set1_ps(1.0f));
    
    return result;
}

/* Test with 11 operands using multiple masked operations */
FORCE_INLINE __m512 test_avx512_11_operands(__m512 a, __m512 b, __m512 c, __m512 d,
                                           __mmask16 k1, __mmask16 k2, __m512 src) {
    /* Chain multiple masked operations - each expands to many RTL operands */
    __m512 t1 = _mm512_mask_fmadd_ps(src, k1, a, b);
    __m512 t2 = _mm512_mask_fmadd_ps(t1, k2, c, d);
    
    /* Complex blend operation with mask */
    __m512 result = _mm512_mask_blend_ps(k1, t1, t2);
    
    /* Additional operation with immediate */
    result = _mm512_maskz_compress_ps(k2, result);
    
    return result;
}

/* Use GCC vector extensions to create complex expressions */
typedef float v16sf __attribute__((vector_size(64)));

FORCE_INLINE v16sf test_gcc_vector_ops(v16sf a, v16sf b, v16sf c, v16sf d) {
    /* Complex expression that may generate multi-operand patterns */
    v16sf result = a * b + c * d;
    result = result + a * c + b * d;
    result = result * 2.0f - a + b - c + d;
    
    /* Permutation-like operation using array syntax */
    v16sf temp = {result[15], result[14], result[13], result[12],
                  result[11], result[10], result[9], result[8],
                  result[7], result[6], result[5], result[4],
                  result[3], result[2], result[1], result[0]};
    
    return result + temp;
}

/* OpenMP SIMD reduction with vector types */
void test_openmp_reduction(float* output, const float* input, int n) {
    v16sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i += 16) {
        v16sf chunk = *(const v16sf*)(input + i);
        sum = sum + chunk * chunk;  /* Complex operation per element */
    }
    
    /* Store result */
    *(v16sf*)output = sum;
}

/* Inline assembly with many operands - directly tests operand handling */
void test_multi_operand_asm(void) {
    /* Use 11 operands in inline asm */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long out1, out2, out3;
    
    asm volatile (
        /* Multiple outputs and inputs - total 11 operands */
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %8\n\t"
        "imul %0, %9\n\t"
        "imul %1, %10\n\t"
        "imul %2, %11"
        : "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "r"(op6), "r"(op7), "r"(op8), "r"(op9)
        : "cc"
    );
    
    printf("ASM result: %ld, %ld, %ld\n", out1, out2, out3);
}

/* Built-in functions that may expand to multi-operand patterns */
FORCE_INLINE double test_builtin_fma_chain(double a, double b, double c, double d) {
    /* Chain of FMA operations - each __builtin_fma has 3 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, a, b);
    double t3 = __builtin_fma(c, d, a);
    double result = __builtin_fma(t1, t2, t3);
    
    /* Prevent optimization away */
    result += __builtin_fma(b, c, d);
    
    return result;
}

/* Main test function - marked as hot to encourage complex instruction selection */
__attribute__((hot))
int main(void) {
    const int N = 1024;
    float* input = aligned_alloc(64, N * sizeof(float));
    float* output = aligned_alloc(64, 16 * sizeof(float));
    
    /* Initialize input data */
    for (int i = 0; i < N; i++) {
        input[i] = (float)i / 100.0f;
    }
    
    /* Test 1: AVX-512 operations (requires -mavx512f) */
    #ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.5f);
        __m512 b = _mm512_set1_ps(2.5f);
        __m512 c = _mm512_set1_ps(3.5f);
        __m512 d = _mm512_set1_ps(4.5f);
        __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
        
        __m512 r1 = test_avx512_10_operands(a, b, c, mask, d);
        __m512 r2 = test_avx512_11_operands(a, b, c, d, mask, 0x5555, a);
        
        /* Store results to prevent elimination */
        _mm512_store_ps(output, r1);
        _mm512_store_ps(output + 16, r2);
        
        printf("AVX-512 test completed\n");
    }
    #endif
    
    /* Test 2: GCC vector extensions */
    {
        v16sf va = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                    9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
        v16sf vb = va * 0.5f;
        v16sf vc = va * 0.25f;
        v16sf vd = va * 0.125f;
        
        v16sf vresult = test_gcc_vector_ops(va, vb, vc, vd);
        
        /* Use result */
        float sum = 0;
        for (int i = 0; i < 16; i++) {
            sum += vresult[i];
        }
        printf("GCC vector sum: %f\n", sum);
    }
    
    /* Test 3: OpenMP reduction */
    test_openmp_reduction(output, input, N);
    printf("OpenMP reduction completed\n");
    
    /* Test 4: Inline assembly with many operands */
    test_multi_operand_asm();
    
    /* Test 5: Built-in FMA chain */
    double fma_result = test_builtin_fma_chain(1.1, 2.2, 3.3, 4.4);
    printf("FMA chain result: %f\n", fma_result);
    
    /* Cleanup */
    free(input);
    free(output);
    
    return 0;
}
