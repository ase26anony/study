/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use target-specific vector intrinsics */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Strategy 2: GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage aggressive optimization */
__attribute__((hot, noinline))
void test_vector_intrinsics() {
    /* This function uses various multi-operand intrinsics */
    
#ifdef __AVX512F__
    /* AVX-512 mask operations often have many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __mmask16 mask = 0xAAAA;
    
    /* These intrinsics can expand to many operands */
    __m512 r1 = _mm512_mask_add_ps(a, mask, b, c);
    __m512 r2 = _mm512_mask_fmadd_ps(a, mask, b, c);
    __m512 r3 = _mm512_mask3_fmadd_ps(a, b, c, mask);
    
    /* Complex permute with mask */
    __m512i idx = _mm512_set1_epi32(0);
    __m512 r4 = _mm512_mask_permutexvar_ps(a, mask, idx, b);
    
    volatile __m512 sink = r1;
    sink = _mm512_add_ps(sink, r2);
    sink = _mm512_add_ps(sink, r3);
    sink = _mm512_add_ps(sink, r4);
#endif

#ifdef __ARM_NEON
    /* ARM NEON multi-vector loads/stores */
    int8x16x4_t v4;
    int8x16_t src[4];
    
    /* vld4q can generate multiple operands */
    v4 = vld4q_s8((const int8_t*)src);
    
    /* Table lookup with multiple registers */
    uint8x16_t result = vqtbl4q_u8(v4, v4.val[0]);
    
    volatile uint8x16_t sink = result;
#endif
}

/* Strategy 3: Complex reductions with GCC vector extensions */
__attribute__((hot, noinline))
v8sf test_vector_reduction(v8sf* arr, int n) {
    v8sf sum = {0};
    v8sf prod = {1, 1, 1, 1, 1, 1, 1, 1};
    
    /* Complex expression that might use FMA */
    for (int i = 0; i < n; i++) {
        /* This complex expression might expand to multi-operand instructions */
        sum = sum + arr[i] * prod;
        prod = prod * arr[i] + sum;
        
        /* Nested FMA-like operations */
        sum = sum + arr[i] * prod + arr[i];
        prod = prod * arr[i] * sum + arr[i];
    }
    
    return sum + prod;
}

/* Strategy 4: Built-in functions for complex math */
__attribute__((hot, noinline))
float test_builtin_fma(float a, float b, float c, float d) {
    /* Chain multiple FMA operations */
    float r1 = __builtin_fmaf(a, b, c);
    float r2 = __builtin_fmaf(b, c, d);
    float r3 = __builtin_fmaf(c, d, a);
    float r4 = __builtin_fmaf(d, a, b);
    
    /* Complex expression preventing early folding */
    return __builtin_fmaf(r1, r2, __builtin_fmaf(r3, r4, a + b + c + d));
}

/* Strategy 5: Inline assembly with many operands */
__attribute__((hot, noinline))
void test_multi_operand_asm() {
    /* Create 11 distinct variables for inline assembly */
    int64_t out1 = 0, out2 = 0, out3 = 0;
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6, in7 = 7;
    
    /* Inline assembly with 10 operands */
    asm volatile (
        "/* Multi-operand test %0, %1, %2, %3, %4, %5, %6, %7, %8, %9 */"
        : "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), 
          "r"(in5), "r"(in6), "r"(in7)
        : "memory"
    );
    
    /* Prevent dead code elimination */
    volatile int64_t sink = out1 + out2 + out3;
}

/* Strategy: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
__attribute__((hot, noinline))
v8sf test_omp_reduction(v8sf* data, int n) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex operation that might use multi-operand instructions */
        v8sf temp = data[i];
        sum = sum + temp * temp + temp;
    }
    
    return sum;
}
#endif

/* Main function that exercises all strategies */
int main() {
    const int N = 100;
    
    /* Test vector intrinsics */
    test_vector_intrinsics();
    
    /* Test GCC vector extensions */
    v8sf* arr = (v8sf*)aligned_alloc(32, N * sizeof(v8sf));
    if (!arr) return 1;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = (i + j) * 0.1f;
        }
    }
    
    v8sf result = test_vector_reduction(arr, N);
    
    /* Test built-in FMA */
    float fma_result = test_builtin_fma(1.1f, 2.2f, 3.3f, 4.4f);
    
    /* Test inline assembly */
    test_multi_operand_asm();
    
#ifdef _OPENMP
    /* Test OpenMP reduction */
    v8sf omp_result = test_omp_reduction(arr, N);
    volatile v8sf sink = omp_result;
#endif
    
    /* Use results to prevent optimization */
    volatile float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    sum += fma_result;
    
    printf("Result: %f\n", sum);
    
    free(arr);
    return 0;
}
