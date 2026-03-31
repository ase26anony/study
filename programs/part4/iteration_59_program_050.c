/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* For x86 AVX-512 */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
static __m512 __attribute__((always_inline))
test_avx512_10_operands(__m512 a, __m512 b, __m512 c, __m512 d,
                       __mmask16 k, float constant) {
    /* Complex expression that may expand to multi-operand instruction */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, _mm512_set1_ps(constant));
    __m512 t3 = _mm512_mask_sub_ps(t2, k, t2, a);
    __m512 t4 = _mm512_mask_fmadd_ps(t3, k, b, c, d);
    
    /* Nested FMA operations - each FMA has 4 operands */
    __m512 result = _mm512_mask_fmadd_ps(
        t4, k,
        _mm512_mask_fmadd_ps(a, k, b, c, d),
        _mm512_set1_ps(2.0f),
        _mm512_mask_add_ps(b, k, c, d)
    );
    
    return result;
}

/* Test with 11 operands using gather/scatter */
static __m512i __attribute__((always_inline))
test_avx512_11_operands(__m512i base, __m512i index, __m512i mask,
                       __m512i src1, __m512i src2, __mmask16 k,
                       int scale, int hint) {
    /* Complex gather operation with many parameters */
    __m512i gathered = _mm512_mask_i32gather_epi32(
        src1,            /* src operand 0 */
        k,               /* operand 1 */
        index,           /* operand 2 */
        (void*)0x1000,   /* operand 3 (base address) */
        scale,           /* operand 4 */
        hint             /* operand 5 */
    );
    
    /* Additional operations to ensure expansion */
    __m512i added = _mm512_mask_add_epi32(gathered, k, src1, src2);
    __m512i blended = _mm512_mask_blend_epi32(k, added, mask);
    
    return blended;
}
#endif

/* For ARM NEON/AArch64 */
#ifdef __ARM_NEON
#include <arm_neon.h>

/* Complex vector operation that may require many operands */
static float32x4_t __attribute__((always_inline))
test_neon_multi_operand(float32x4_t a, float32x4_t b, float32x4_t c,
                        float32x4_t d, float32x4_t e, float32x4_t f,
                        float32x4_t g, float32x4_t h, uint32x4_t mask) {
    /* Complex expression with multiple operations */
    float32x4_t t1 = vfmaq_f32(a, b, c);
    float32x4_t t2 = vfmaq_f32(d, e, f);
    float32x4_t t3 = vaddq_f32(t1, t2);
    float32x4_t t4 = vmulq_f32(t3, g);
    float32x4_t t5 = vbslq_f32(mask, t4, h);
    
    /* Nested operations */
    float32x4_t result = vfmaq_f32(
        t5,
        vmulq_f32(a, b),
        vfmaq_f32(c, d, e)
    );
    
    return result;
}
#endif

/* GCC vector extensions - generic approach */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

static v8sf __attribute__((always_inline, hot))
test_gcc_vector_ops(v8sf a, v8sf b, v8sf c, v8sf d,
                    v8sf e, v8sf f, v8sf g, v8sf h,
                    v8sf i, v8sf j) {
    /* Complex expression with many vector operands */
    v8sf t1 = a + b * c;
    v8sf t2 = d - e / f;
    v8sf t3 = g * h + i;
    v8sf t4 = __builtin_fmaf(a, b, c);  /* Builtin FMA */
    v8sf t5 = __builtin_fmaf(d, e, f);
    v8sf t6 = __builtin_fmaf(g, h, i);
    
    /* Combine all - this complex expression may require
     * many operands during RTL expansion */
    v8sf result = t1 * t2 + t3 * t4 - t5 / t6 + j;
    
    /* Additional builtins to encourage complex expansion */
    result = __builtin_fmaf(result, a, b);
    result = __builtin_fmaf(result, c, d);
    
    return result;
}

/* Inline assembly with many operands */
static void __attribute__((always_inline))
test_many_asm_operands(void) {
    /* 11-operand inline asm statement */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    asm volatile (
        "/* Dummy multi-operand asm */\n\t"
        "add %0, %1, %2\n\t"
        "add %3, %4, %5\n\t"
        "add %6, %7, %8\n\t"
        "add %9, %10, %0"
        : "+r" (a), "+r" (b), "+r" (c)
        : "r" (d), "r" (e), "r" (f),
          "r" (g), "r" (h), "r" (i),
          "r" (j), "r" (k)
        : "cc", "memory"
    );
    
    /* Use results to prevent elimination */
    volatile int sink = a + b + c + d + e + f + g + h + i + j + k;
    (void)sink;
}

/* OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
static v8sf test_omp_reduction(v8sf *array, int n) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex reduction expression */
        sum = sum + array[i] * array[i] + 
              __builtin_fmaf(array[i], array[(i+1)%n], array[(i+2)%n]);
    }
    
    return sum;
}
#endif

/* Main test function */
__attribute__((hot))
int main(void) {
    float result = 0.0f;
    
    /* Test 1: GCC vector extensions */
    {
        v8sf a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        v8sf c = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        v8sf d = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
        v8sf e = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
        v8sf f = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f};
        v8sf g = {7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f};
        v8sf h = {8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f};
        v8sf i = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
        v8sf j = {10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
        
        v8sf res = test_gcc_vector_ops(a, b, c, d, e, f, g, h, i, j);
        result += res[0];
    }
    
    /* Test 2: Inline assembly with many operands */
    test_many_asm_operands();
    
    /* Test 3: OpenMP reduction if available */
    #ifdef _OPENMP
    {
        v8sf array[16];
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 8; j++) {
                array[i][j] = (float)(i * 8 + j);
            }
        }
        v8sf sum = test_omp_reduction(array, 16);
        result += sum[0];
    }
    #endif
    
    /* Test 4: Target-specific intrinsics */
    #ifdef __AVX512F__
    {
        __m512 avx_a = _mm512_set1_ps(1.0f);
        __m512 avx_b = _mm512_set1_ps(2.0f);
        __m512 avx_c = _mm512_set1_ps(3.0f);
        __m512 avx_d = _mm512_set1_ps(4.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 avx_res = test_avx512_10_operands(
            avx_a, avx_b, avx_c, avx_d, mask, 5.0f
        );
        
        float avx_tmp[16];
        _mm512_storeu_ps(avx_tmp, avx_res);
        result += avx_tmp[0];
    }
    #endif
    
    #ifdef __ARM_NEON
    {
        float32x4_t neon_a = {1.0f, 2.0f, 3.0f, 4.0f};
        float32x4_t neon_b = {2.0f, 3.0f, 4.0f, 5.0f};
        float32x4_t neon_c = {3.0f, 4.0f, 5.0f, 6.0f};
        float32x4_t neon_d = {4.0f, 5.0f, 6.0f, 7.0f};
        float32x4_t neon_e = {5.0f, 6.0f, 7.0f, 8.0f};
        float32x4_t neon_f = {6.0f, 7.0f, 8.0f, 9.0f};
        float32x4_t neon_g = {7.0f, 8.0f, 9.0f, 10.0f};
        float32x4_t neon_h = {8.0f, 9.0f, 10.0f, 11.0f};
        uint32x4_t mask = {0xFFFFFFFF, 0, 0xFFFFFFFF, 0};
        
        float32x4_t neon_res = test_neon_multi_operand(
            neon_a, neon_b, neon_c, neon_d, neon_e,
            neon_f, neon_g, neon_h, mask
        );
        
        float neon_tmp[4];
        vst1q_f32(neon_tmp, neon_res);
        result += neon_tmp[0];
    }
    #endif
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", result);
    
    return (result > 0.0f) ? 0 : 1;
}
