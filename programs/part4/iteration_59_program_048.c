/* Test program to cover 10-11 operand cases in optabs.cc */
/* Compile with: gcc -O3 -mavx512f -mfma -ftree-vectorize -fno-math-errno -ffast-math -c test.c -o test.o */

#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>

/* GCC vector extensions for complex patterns */
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

/* Always inline to force expansion */
static inline __attribute__((always_inline))
__m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                              __m512 d, __m512 e, __mmask16 k) {
    /* AVX-512 masked FMA with multiple operations - can expand to many operands */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_fmadd_ps(d, k, e, t1);
    
    /* Complex expression to prevent optimization */
    __m512 t3 = _mm512_maskz_mul_ps(k, t2, _mm512_set1_ps(2.0f));
    __m512 t4 = _mm512_mask_add_ps(t3, k, t3, _mm512_set1_ps(1.0f));
    
    /* Nested operations that might expand to 10-11 operand patterns */
    return _mm512_mask_sub_ps(a, k, t4, _mm512_mask_mul_ps(b, k, c, d));
}

/* Test with GCC built-in FMA operations */
static inline __attribute__((always_inline))
v16sf test_gcc_vector_fma(v16sf a, v16sf b, v16sf c, v16sf d) {
    /* Complex expression with multiple FMA operations */
    v16sf t1 = a * b + c;
    v16sf t2 = d * a + b;
    v16sf t3 = t1 * t2 + c;
    v16sf t4 = t3 * a + d;
    
    /* Use __builtin_fma to force specific pattern expansion */
    v16sf result = t4;
    for (int i = 0; i < 16; i++) {
        /* This might expand to multi-operand patterns */
        result[i] = __builtin_fmaf(a[i], b[i], result[i]);
        result[i] = __builtin_fmaf(c[i], d[i], result[i]);
    }
    
    return result;
}

/* Inline assembly with many operands - directly tests operand handling */
static inline __attribute__((always_inline))
void test_many_operand_asm(float *out, float *in1, float *in2, 
                          float *in3, float *in4, float *in5) {
    /* 11-operand asm statement */
    asm volatile (
        "vmovaps %1, %%zmm0\n\t"
        "vmovaps %2, %%zmm1\n\t"
        "vmovaps %3, %%zmm2\n\t"
        "vmovaps %4, %%zmm3\n\t"
        "vmovaps %5, %%zmm4\n\t"
        "vfmadd213ps %%zmm0, %%zmm1, %%zmm2\n\t"
        "vfmadd231ps %%zmm3, %%zmm4, %%zmm2\n\t"
        "vmovaps %%zmm2, %0\n\t"
        : "=m" (*out)
        : "m" (*in1), "m" (*in2), "m" (*in3), "m" (*in4), "m" (*in5)
        : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "memory"
    );
}

/* Complex reduction pattern that might generate multi-operand instructions */
#pragma omp declare simd
static inline float complex_reduction(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex expression that might use FMA */
        sum = __builtin_fmaf(arr[i], arr[i], sum);
        sum = __builtin_fmaf(arr[i], 2.0f, sum);
    }
    return sum;
}

/* Main test function */
__attribute__((hot))
int main() {
    /* Align memory for AVX-512 */
    float *arr1 = __builtin_assume_aligned(
        (float*)__builtin_alloca(64 * sizeof(float)), 64);
    float *arr2 = __builtin_assume_aligned(
        (float*)__builtin_alloca(64 * sizeof(float)), 64);
    float *arr3 = __builtin_assume_aligned(
        (float*)__builtin_alloca(64 * sizeof(float)), 64);
    float *arr4 = __builtin_assume_aligned(
        (float*)__builtin_alloca(64 * sizeof(float)), 64);
    float *arr5 = __builtin_assume_aligned(
        (float*)__builtin_alloca(64 * sizeof(float)), 64);
    float out[64] __attribute__((aligned(64)));
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i + 1);
        arr3[i] = (float)(i + 2);
        arr4[i] = (float)(i + 3);
        arr5[i] = (float)(i + 4);
    }
    
    /* Test 1: AVX-512 masked operations */
    __m512 v1 = _mm512_load_ps(arr1);
    __m512 v2 = _mm512_load_ps(arr2);
    __m512 v3 = _mm512_load_ps(arr3);
    __m512 v4 = _mm512_load_ps(arr4);
    __m512 v5 = _mm512_load_ps(arr5);
    
    __mmask16 mask = 0xAAAA; /* Alternating mask */
    
    /* This call should trigger complex pattern expansion */
    __m512 result1 = test_avx512_10_operand(v1, v2, v3, v4, v5, mask);
    _mm512_store_ps(out, result1);
    
    /* Test 2: GCC vector extensions */
    v16sf gv1 = *(v16sf*)arr1;
    v16sf gv2 = *(v16sf*)arr2;
    v16sf gv3 = *(v16sf*)arr3;
    v16sf gv4 = *(v16sf*)arr4;
    
    v16sf result2 = test_gcc_vector_fma(gv1, gv2, gv3, gv4);
    *(v16sf*)out = result2;
    
    /* Test 3: Many-operand inline assembly */
    test_many_operand_asm(out, arr1, arr2, arr3, arr4, arr5);
    
    /* Test 4: Complex reduction */
    float reduction_result = complex_reduction(arr1, 64);
    
    /* Use results to prevent dead code elimination */
    float sum = 0.0f;
    for (int i = 0; i < 64; i++) {
        sum += out[i];
    }
    sum += reduction_result;
    
    printf("Result: %f\n", sum);
    return (int)sum;
}

/* Additional test with immediate operands */
__attribute__((noinline))
void test_immediate_operands() {
    /* AVX-512 with immediate shift counts - can add to operand count */
    __m512i v1 = _mm512_set1_epi32(1);
    __m512i v2 = _mm512_slli_epi32(v1, 3);  /* Immediate operand */
    __m512i v3 = _mm512_srai_epi32(v2, 2);  /* Another immediate */
    
    /* Complex permute with immediate */
    __m512i v4 = _mm512_permutexvar_epi32(_mm512_set1_epi32(0x3210), v3);
    
    /* Store to prevent elimination */
    volatile __m512i* dummy = &v4;
    (void)dummy;
}
