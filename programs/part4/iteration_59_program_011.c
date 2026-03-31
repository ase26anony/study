/* Test program to cover 10-11 operand cases in optabs.cc */
/* Compile with: gcc -O3 -mavx512f -mfma -mavx512vl -mavx512bw -ftree-vectorize -c test.c -o test.o */

#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Complex AVX-512 operation that should generate many operands */
FORCE_INLINE __m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                                          __mmask16 k, __m512 src) {
    /* _mm512_mask3_fmadd_ps has implicit 3 source operands + mask + rounding mode */
    /* Combined with other operations to reach 10+ operands */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);
    __m512 t2 = _mm512_maskz_fmadd_round_ps(k, a, b, c, _MM_FROUND_CUR_DIRECTION);
    
    /* Use blend with immediate constant (8-bit immediate counts as operand) */
    __m512 result = _mm512_mask_blend_ps(0xAA, t1, t2);
    
    /* Additional operation with mask and immediate */
    result = _mm512_mask_add_ps(src, k, result, _mm512_set1_ps(1.0f));
    
    return result;
}

/* Test case using GCC vector extensions for complex reduction */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

FORCE_INLINE v16sf complex_vector_op(v16sf a, v16sf b, v16sf c, v16sf d) {
    /* Complex expression that might expand to many operands */
    v16sf t1 = a * b + c;
    v16sf t2 = b * c - d;
    v16sf t3 = c * d + a;
    v16sf t4 = d * a - b;
    
    /* Nested FMA-like operations */
    v16sf result = t1 + t2 * t3 - t4;
    
    /* Use conditional select with vector mask */
    v16si mask = (v16si)(a > b);
    result = (v16sf)((mask & (v16si)result) | (~mask & (v16si)t1));
    
    return result;
}

/* Inline assembly with many operands */
void test_asm_11_operands(void) {
    int64_t a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;
    
    /* Initialize variables to prevent optimization */
    a1 = 1; a2 = 2; a3 = 3; a4 = 4; a5 = 5;
    a6 = 6; a7 = 7; a8 = 8; a9 = 9; a10 = 10; a11 = 11;
    
    /* 11-operand inline asm */
    __asm__ volatile (
        "add %[v2], %[v1]\n\t"
        "add %[v3], %[v2]\n\t"
        "add %[v4], %[v3]\n\t"
        "add %[v5], %[v4]\n\t"
        "add %[v6], %[v5]\n\t"
        "add %[v7], %[v6]\n\t"
        "add %[v8], %[v7]\n\t"
        "add %[v9], %[v8]\n\t"
        "add %[v10], %[v9]\n\t"
        "add %[v11], %[v10]"
        : [v1] "+r" (a1), [v2] "+r" (a2), [v3] "+r" (a3),
          [v4] "+r" (a4), [v5] "+r" (a5), [v6] "+r" (a6),
          [v7] "+r" (a7), [v8] "+r" (a8), [v9] "+r" (a9),
          [v10] "+r" (a10), [v11] "+r" (a11)
        :
        : "cc"
    );
    
    /* Use results to prevent dead code elimination */
    printf("ASM result: %ld\n", a1 + a11);
}

/* OpenMP SIMD reduction with vector types */
void test_omp_reduction(void) {
    #define N 1024
    v16sf array[N];
    v16sf sum = {0};
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            array[i][j] = (float)(i * 16 + j) * 0.1f;
        }
    }
    
    /* Complex reduction that might generate multi-operand patterns */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < N; i++) {
        /* Complex expression combining multiple vector operations */
        v16sf t = array[i];
        sum = sum + t * t - t / 2.0f + 1.0f;
    }
    
    /* Use result */
    float total = 0;
    for (int i = 0; i < 16; i++) {
        total += sum[i];
    }
    printf("OMP reduction: %f\n", total);
}

/* Main test function */
int main(void) {
    /* Test AVX-512 intrinsics if supported */
    #ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __m512 src = _mm512_set1_ps(0.0f);
        __mmask16 k = 0xAAAA;
        
        __m512 result = test_avx512_10_operand(a, b, c, k, src);
        
        /* Use result to prevent optimization */
        float res_array[16];
        _mm512_storeu_ps(res_array, result);
        printf("AVX-512 result[0]: %f\n", res_array[0]);
    }
    #endif
    
    /* Test GCC vector extensions */
    {
        v16sf va = {0};
        v16sf vb = {0};
        v16sf vc = {0};
        v16sf vd = {0};
        
        for (int i = 0; i < 16; i++) {
            va[i] = i * 0.1f;
            vb[i] = i * 0.2f;
            vc[i] = i * 0.3f;
            vd[i] = i * 0.4f;
        }
        
        v16sf vresult = complex_vector_op(va, vb, vc, vd);
        
        /* Use result */
        float total = 0;
        for (int i = 0; i < 16; i++) {
            total += vresult[i];
        }
        printf("Vector extension result: %f\n", total);
    }
    
    /* Test inline assembly */
    test_asm_11_operands();
    
    /* Test OpenMP reduction */
    #ifdef _OPENMP
    test_omp_reduction();
    #endif
    
    return 0;
}
