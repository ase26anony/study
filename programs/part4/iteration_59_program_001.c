/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* Force inlining */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Hot function attribute */
__attribute__((hot, noinline))
void complex_vector_operations() {
#ifdef __x86_64__
    /* AVX-512 intrinsics with many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __mmask16 k = 0xAAAA;  /* 1010101010101010 pattern */
    
    /* These intrinsics can expand to many operands */
    __m512 r1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 r2 = _mm512_mask_mul_ps(r1, k, a, b);
    
    /* FMA operations with masking - potentially many operands */
    __m512 r3 = _mm512_mask_fmadd_ps(a, k, b, c);
    __m512 r4 = _mm512_mask3_fmadd_ps(a, b, c, k);
    
    /* Blend with multiple sources */
    __m512 r5 = _mm512_mask_blend_ps(k, a, b);
    
    /* Store to prevent elimination */
    float result[16];
    _mm512_storeu_ps(result, r5);
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE style operations */
    float32x4_t v1 = vdupq_n_f32(1.0f);
    float32x4_t v2 = vdupq_n_f32(2.0f);
    float32x4_t v3 = vdupq_n_f32(3.0f);
    float32x4_t v4 = vdupq_n_f32(4.0f);
    
    /* Complex FMA-like operations */
    float32x4_t r1 = vfmaq_laneq_f32(v1, v2, v3, 1);
    float32x4_t r2 = vfmsq_laneq_f32(v2, v3, v4, 2);
    
    /* Multiple vector operations */
    float32x4_t r3 = vaddq_f32(vmulq_f32(v1, v2), v3);
    float32x4_t r4 = vmlaq_f32(v1, v2, v3);
    
    /* Store results */
    float result[4];
    vst1q_f32(result, r4);
#endif
}

/* Function using GCC vector extensions */
FORCE_INLINE
v8sf complex_vector_expr(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression that might expand to multi-operand instruction */
    return a * b + c * d + (a + b) * (c - d) + a * c * b * d;
}

/* OpenMP SIMD reduction with vector types */
void omp_vector_reduction(float* output, const float* input, int n) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i += 8) {
        v8sf chunk = *(v8sf*)(input + i);
        sum = sum + chunk * chunk + chunk;
    }
    
    *(v8sf*)output = sum;
}

/* Built-in FMA usage */
FORCE_INLINE
double builtin_fma_chain(double a, double b, double c, double d, double e) {
    /* Chain of FMA operations - each expands to 3+ operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, t1);
    double t3 = __builtin_fma(a, c, t2);
    double t4 = __builtin_fma(b, d, t3);
    return __builtin_fma(e, a, t4);
}

/* Inline assembly with many operands */
void many_operand_asm() {
    int64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int64_t result1, result2, result3;
    
    /* 11-operand asm statement */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "mul %[r1], %[r1], %[g]\n\t"
        "mul %[r2], %[r2], %[h]\n\t"
        "mul %[r3], %[r3], %[i]\n\t"
        "add %[r1], %[r1], %[j]\n\t"
        "add %[r2], %[r2], %[k]"
        : [r1] "=r" (result1), [r2] "=r" (result2), [r3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
}

/* Main function combining all approaches */
int main() {
    float data[64];
    float output[8];
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data[i] = i * 0.1f;
    }
    
    /* Execute various operations that might trigger multi-operand expansion */
    complex_vector_operations();
    
    omp_vector_reduction(output, data, 64);
    
    double fma_result = builtin_fma_chain(1.1, 2.2, 3.3, 4.4, 5.5);
    
    many_operand_asm();
    
    /* Use GCC vector extensions */
    v8sf va = {1,2,3,4,5,6,7,8};
    v8sf vb = {2,3,4,5,6,7,8,9};
    v8sf vc = {3,4,5,6,7,8,9,10};
    v8sf vd = {4,5,6,7,8,9,10,11};
    v8sf vr = complex_vector_expr(va, vb, vc, vd);
    
    /* Prevent dead code elimination */
    float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += output[i] + vr[i];
    }
    
    printf("Result: %f (FMA: %f)\n", sum, fma_result);
    return (int)(sum * 1000);
}
