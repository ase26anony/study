/* Test program to cover 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Strategy 1: Use target-specific vector intrinsics with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
#define USE_AVX512 1
#elif defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
#define USE_NEON 1
#endif

/* Strategy 2: GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Strategy 4: Complex math builtins */
static inline float complex_multiply(float a, float b, float c, float d) {
    /* Complex multiplication: (a+bi)*(c+di) = (ac-bd) + (ad+bc)i */
    /* Use FMA builtins to create multi-operand patterns */
    float real = __builtin_fmaf(a, c, -__builtin_fmaf(b, d, 0.0f));
    float imag = __builtin_fmaf(a, d, __builtin_fmaf(b, c, 0.0f));
    return real + imag; /* Simplified return */
}

/* Hot function attribute to encourage complex instruction selection */
__attribute__((hot, noinline))
static void test_vector_operations(float* result) {
    /* Strategy 1: AVX-512 mask operations with many operands */
#if USE_AVX512
    __m512 vec1 = _mm512_set1_ps(1.0f);
    __m512 vec2 = _mm512_set1_ps(2.0f);
    __m512 vec3 = _mm512_set1_ps(3.0f);
    __m512 vec4 = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
    
    /* This may expand to a pattern with many operands:
       mask, src1, src2, src3, rounding mode, etc. */
    __m512 res1 = _mm512_mask_fmadd_ps(vec1, mask, vec2, vec3);
    __m512 res2 = _mm512_mask3_fmadd_ps(vec1, vec2, vec3, mask);
    
    /* Store results */
    _mm512_storeu_ps(result, res1);
    _mm512_storeu_ps(result + 16, res2);
    
#elif USE_NEON
    /* Strategy 1: ARM NEON multi-vector operations */
    float32x4x4_t vecs;
    vecs.val[0] = vdupq_n_f32(1.0f);
    vecs.val[1] = vdupq_n_f32(2.0f);
    vecs.val[2] = vdupq_n_f32(3.0f);
    vecs.val[3] = vdupq_n_f32(4.0f);
    
    /* Complex multi-vector operations */
    float32x4_t res = vmlaq_f32(vecs.val[0], vecs.val[1], vecs.val[2]);
    res = vfmaq_f32(res, vecs.val[3], vecs.val[0]);
    
    vst1q_f32(result, res);
    
#else
    /* Strategy 2: GCC vector extensions with complex expressions */
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf v3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf v4 = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    
    /* Complex expression that might generate multi-operand patterns */
    v8sf r1 = v1 * v2 + v3 * v4;
    v8sf r2 = v1 / (v2 + v3) - v4;
    v8sf result_vec = r1 * r2 + v1 - v2 + v3 - v4;
    
    memcpy(result, &result_vec, sizeof(v8sf));
#endif
    
    /* Strategy 4: Complex math builtins in a loop */
    for (int i = 0; i < 8; i++) {
        float a = result[i] + 1.0f;
        float b = result[i] + 2.0f;
        float c = result[i] + 3.0f;
        float d = result[i] + 4.0f;
        
        /* Nested FMA calls - each expands to multiple operands */
        float t1 = __builtin_fmaf(a, b, c);
        float t2 = __builtin_fmaf(b, c, d);
        float t3 = __builtin_fmaf(c, d, a);
        float t4 = __builtin_fmaf(d, a, b);
        
        result[i] = t1 + t2 + t3 + t4;
    }
}

/* Strategy 3: Inline assembly with many operands */
__attribute__((noinline))
static void test_multi_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    
    /* Inline assembly with 11 operands (10 inputs + 1 output) */
    asm volatile (
        /* Template for x86 */
        "#APP\n\t"
        "# Test multi-operand asm\n\t"
        "#NOAPP"
        : "=r"(op1)  /* output */
        : "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "r"(op6), "r"(op7), "r"(op8), "r"(op9),
          "r"(op10), "r"(op11)  /* 10 inputs */
        : "memory"
    );
    
    /* Use the result to prevent optimization */
    volatile long dummy = op1;
    (void)dummy;
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
__attribute__((noinline))
static float test_omp_reduction(void) {
    const int N = 1024;
    float array[N];
    float result = 0.0f;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = (i % 10) * 0.1f;
    }
    
    /* OpenMP SIMD reduction - may generate complex vector patterns */
    #pragma omp simd reduction(+:result)
    for (int i = 0; i < N; i++) {
        /* Complex expression to encourage multi-operand expansion */
        result += array[i] * array[(i + 1) % N] - 
                  array[(i + 2) % N] / (array[i] + 1.0f);
    }
    
    return result;
}

int main(void) {
    float* buffer = aligned_alloc(64, 64 * sizeof(float));
    if (!buffer) return 1;
    
    /* Test 1: Vector operations with intrinsics/GCC vectors */
    test_vector_operations(buffer);
    
    /* Test 2: Multi-operand inline assembly */
    test_multi_operand_asm();
    
    /* Test 3: OpenMP reduction */
    float reduction_result = test_omp_reduction();
    
    /* Use results to prevent dead code elimination */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += buffer[i];
    }
    sum += reduction_result;
    
    printf("Result: %f\n", sum);
    
    free(buffer);
    return 0;
}
