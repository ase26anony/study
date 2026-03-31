/* Test for covering 10/11-operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Strategy 1: Use AVX-512 intrinsics with many operands */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inlining to ensure expansion */
__attribute__((always_inline, target("avx512f")))
static inline __m512 test_avx512_many_operands(__m512 a, __m512 b, __m512 c, 
                                               __m512 d, __m512 e, __m512 f,
                                               __mmask16 k) {
    /* Complex expression that might expand to many operands */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, e);
    __m512 t3 = _mm512_mask_fmadd_ps(t2, k, f, a, b);  /* 5 explicit operands */
    __m512 t4 = _mm512_mask_sub_ps(t3, k, c, d);
    return _mm512_mask_add_ps(t4, k, e, f);
}
#endif

/* Strategy 2: GCC vector extensions with complex operations */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

__attribute__((always_inline))
static inline v16sf test_vector_extensions(v16sf a, v16sf b, v16sf c,
                                           v16sf d, v16sf e, v16sf f) {
    /* Complex expression that might require many temporaries */
    v16sf t1 = a + b * c;
    v16sf t2 = d - e / f;
    v16sf t3 = t1 * t2 + a;
    v16sf t4 = b * c - d;
    v16sf t5 = e + f * t3;
    v16sf t6 = t4 / t5 * t2;
    return t6 + t1 - t3;
}

/* Strategy 3: Inline assembly with many operands */
__attribute__((always_inline))
static inline void test_many_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long out1, out2, out3;
    
    /* Extended asm with 11 total operands (3 outputs, 8 inputs) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "sub %1, %6\n\t"
        "mov %2, %7\n\t"
        "imul %2, %8\n\t"
        "add %0, %9\n\t"
        "sub %1, %10\n\t"
        "add %2, %11"
        : "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "r"(op6), "r"(op7), "r"(op8), "r"(op9)
        : "cc"
    );
    
    /* Use results to prevent optimization */
    printf("ASM results: %ld %ld %ld\n", out1, out2, out3);
}

/* Strategy 4: Complex built-in functions */
__attribute__((always_inline))
static inline double test_builtin_fma_chain(double a, double b, double c,
                                            double d, double e, double f,
                                            double g, double h, double i,
                                            double j, double k) {
    /* Chain of FMA operations - each FMA has 3 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, i);
    double t4 = __builtin_fma(t1, t2, t3);
    double t5 = __builtin_fma(j, k, t4);
    double t6 = __builtin_fma(a, d, g);
    double t7 = __builtin_fma(b, e, h);
    double t8 = __builtin_fma(c, f, i);
    double t9 = __builtin_fma(t5, t6, t7);
    return __builtin_fma(t8, t9, j);
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
__attribute__((always_inline))
static inline float test_omp_simd_reduction(void) {
    #define N 1024
    float array[N];
    float result = 0.0f;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = (i % 10) * 0.1f;
    }
    
    /* Complex reduction that might generate multi-operand patterns */
    #pragma omp simd reduction(+:result) simdlen(16)
    for (int i = 0; i < N; i++) {
        /* Complex expression to encourage multi-operand expansion */
        result += array[i] * 2.0f - array[(i + 1) % N] * 1.5f 
                + array[(i + 2) % N] * 0.5f - array[(i + 3) % N] * 0.25f;
    }
    
    return result;
}
#endif

/* Hot function to encourage aggressive optimization */
__attribute__((hot, noinline))
static void run_complex_operations(void) {
    volatile int use_result = 0;
    
    /* Test 1: Vector extensions */
    {
        v16sf v1 = {1.0f}, v2 = {2.0f}, v3 = {3.0f};
        v16sf v4 = {4.0f}, v5 = {5.0f}, v6 = {6.0f};
        v16sf result = test_vector_extensions(v1, v2, v3, v4, v5, v6);
        use_result += ((float*)&result)[0];
    }
    
    /* Test 2: Built-in FMA chain */
    {
        double fma_result = test_builtin_fma_chain(1.1, 2.2, 3.3, 4.4, 5.5,
                                                   6.6, 7.7, 8.8, 9.9, 10.1,
                                                   11.1);
        use_result += (int)fma_result;
    }
    
    /* Test 3: Inline assembly */
    test_many_operand_asm();
    
#ifdef __AVX512F__
    /* Test 4: AVX-512 intrinsics */
    {
        __m512 avx_a = _mm512_set1_ps(1.0f);
        __m512 avx_b = _mm512_set1_ps(2.0f);
        __m512 avx_c = _mm512_set1_ps(3.0f);
        __m512 avx_d = _mm512_set1_ps(4.0f);
        __m512 avx_e = _mm512_set1_ps(5.0f);
        __m512 avx_f = _mm512_set1_ps(6.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 avx_result = test_avx512_many_operands(avx_a, avx_b, avx_c,
                                                      avx_d, avx_e, avx_f,
                                                      mask);
        use_result += _mm512_cvtss_f32(avx_result);
    }
#endif
    
#ifdef _OPENMP
    /* Test 5: OpenMP SIMD reduction */
    {
        float omp_result = test_omp_simd_reduction();
        use_result += (int)omp_result;
    }
#endif
    
    /* Prevent dead code elimination */
    if (use_result == 0x1234) {
        printf("Impossible branch\n");
    }
}

int main(void) {
    /* Run complex operations multiple times to increase chances
       of hitting the coverage during compilation */
    for (int i = 0; i < 10; i++) {
        run_complex_operations();
    }
    
    return 0;
}
