/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use AVX-512 intrinsics with many operands (for x86 targets) */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
static __m512 __attribute__((always_inline))
test_avx512_10_operands(__m512 a, __m512 b, __m512 c, __m512 d,
                       __m512 e, __m512 f, __m512 g, __m512 h,
                       __mmask16 k1, __mmask16 k2) {
    /* Complex sequence that might expand to multi-operand instructions */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);
    __m512 t2 = _mm512_mask_mul_ps(d, k2, e, f);
    __m512 t3 = _mm512_mask_fmadd_ps(g, k1, h, t1);
    return _mm512_mask_fmadd_ps(t2, k2, t3, _mm512_set1_ps(1.0f));
}

/* Test 11 operands with immediate constant */
static __m512i __attribute__((always_inline))
test_avx512_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                       __m512i e, __m512i f, __m512i g, __m512i h,
                       __mmask64 k, int imm) {
    /* Multiple operations that might combine */
    __m512i t1 = _mm512_mask_add_epi32(a, k, b, c);
    __m512i t2 = _mm512_mask_slli_epi32(d, k, e, imm);
    __m512i t3 = _mm512_mask_alignr_epi32(f, k, g, h, 4);
    return _mm512_mask_add_epi32(t1, k, t2, t3);
}
#endif

/* Strategy 2: GCC vector extensions with complex expressions */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

static v16sf __attribute__((always_inline, hot))
test_gcc_vector_ops(v16sf a, v16sf b, v16sf c, v16sf d,
                   v16sf e, v16sf f, v16sf g, v16sf h,
                   float s1, float s2, float s3) {
    /* Complex expression with many operands */
    v16sf t1 = a * b + c;
    v16sf t2 = d * e - f;
    v16sf t3 = g * h + a;
    v16sf t4 = t1 * s1 + t2 * s2 + t3 * s3;
    
    /* Nested operations to prevent simplification */
    v16sf t5 = __builtin_fmaf(t1, t2, t3);
    v16sf t6 = __builtin_fmaf(t4, t5, a);
    
    return t5 * t6 + b * c - d * e + f * g - h * a;
}

/* Strategy 3: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
static v16sf test_omp_reduction(v16sf *arr, int n) {
    v16sf sum = {0};
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex operation on each element */
        v16sf t = arr[i];
        sum = sum + t * t - t + arr[i % 16];
    }
    return sum;
}
#endif

/* Strategy 4: Inline assembly with many operands */
static void __attribute__((noinline))
test_many_operand_asm(uint64_t *out1, uint64_t *out2, uint64_t *out3,
                     uint64_t *out4, uint64_t *out5,
                     uint64_t in1, uint64_t in2, uint64_t in3,
                     uint64_t in4, uint64_t in5, uint64_t in6) {
    /* 11-operand asm statement */
    __asm__ volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out2]\n\t"
        "sub %[in3], %[out3]\n\t"
        "and %[in4], %[out4]\n\t"
        "or  %[in5], %[out5]\n\t"
        : [out1] "=r" (*out1),
          [out2] "=r" (*out2),
          [out3] "=r" (*out3),
          [out4] "=r" (*out4),
          [out5] "=r" (*out5)
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "r" (in3),
          [in4] "r" (in4),
          [in5] "r" (in5),
          [in6] "r" (in6)
        : "memory"
    );
}

/* Strategy 5: Complex built-in math operations */
static double __attribute__((noinline))
test_builtin_fma_chain(double a, double b, double c, double d,
                      double e, double f, double g, double h,
                      double i, double j, double k) {
    /* Chain of FMA operations - each expands to 3 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, i);
    double t4 = __builtin_fma(t1, t2, t3);
    return __builtin_fma(t4, j, k);
}

/* Main test driver */
int main() {
    int result = 0;
    
    /* Test GCC vector extensions */
    {
        v16sf a = {1.0f}, b = {2.0f}, c = {3.0f}, d = {4.0f};
        v16sf e = {5.0f}, f = {6.0f}, g = {7.0f}, h = {8.0f};
        
        v16sf res = test_gcc_vector_ops(a, b, c, d, e, f, g, h, 1.5f, 2.5f, 3.5f);
        result += (int)res[0];
    }
    
#ifdef __AVX512F__
    /* Test AVX-512 intrinsics */
    {
        __m512 av = _mm512_set1_ps(1.0f);
        __m512 bv = _mm512_set1_ps(2.0f);
        __m512 cv = _mm512_set1_ps(3.0f);
        __m512 dv = _mm512_set1_ps(4.0f);
        __m512 ev = _mm512_set1_ps(5.0f);
        __m512 fv = _mm512_set1_ps(6.0f);
        __m512 gv = _mm512_set1_ps(7.0f);
        __m512 hv = _mm512_set1_ps(8.0f);
        
        __mmask16 k1 = 0xAAAA;
        __mmask16 k2 = 0x5555;
        
        __m512 res = test_avx512_10_operands(av, bv, cv, dv, ev, fv, gv, hv, k1, k2);
        float f = _mm512_cvtss_f32(res);
        result += (int)f;
    }
#endif
    
    /* Test inline assembly with many operands */
    {
        uint64_t out1, out2, out3, out4, out5;
        test_many_operand_asm(&out1, &out2, &out3, &out4, &out5,
                              1, 2, 3, 4, 5, 6);
        result += (int)(out1 + out2 + out3 + out4 + out5);
    }
    
    /* Test built-in FMA chain */
    {
        double res = test_builtin_fma_chain(1.0, 2.0, 3.0, 4.0, 5.0,
                                           6.0, 7.0, 8.0, 9.0, 10.0, 11.0);
        result += (int)res;
    }
    
#ifdef _OPENMP
    /* Test OpenMP reduction */
    {
        v16sf arr[32];
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 16; j++) {
                arr[i][j] = (float)(i + j);
            }
        }
        v16sf sum = test_omp_reduction(arr, 32);
        result += (int)sum[0];
    }
#endif
    
    printf("Result: %d\n", result);
    return result != 0;
}
