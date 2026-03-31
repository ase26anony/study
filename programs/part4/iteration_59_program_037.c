/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* GCC vector extensions for additional patterns */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Complex inline assembly with many operands */
static void inline_asm_11_operands(int *a, int *b, int *c, int *d, int *e,
                                   int *f, int *g, int *h, int *i, int *j, int *k) {
    asm volatile (
        "mov %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "sub %[e], %[f]\n\t"
        "mul %[g], %[h]\n\t"
        "and %[i], %[j]\n\t"
        "or %[k], %[a]"
        : [a] "+r" (*a), [b] "+r" (*b), [c] "+r" (*c)
        : [d] "r" (*d), [e] "r" (*e), [f] "r" (*f),
          [g] "r" (*g), [h] "r" (*h), [i] "r" (*i),
          [j] "r" (*j), [k] "r" (*k)
        : "cc"
    );
}

/* AVX-512 mask operations with many operands */
FORCE_INLINE __m512 avx512_complex_op(__m512 a, __m512 b, __m512 c, 
                                      __m512 d, __m512 e, __m512 f,
                                      __mmask16 k1, __mmask16 k2) {
    /* Chain multiple masked operations to increase operand count */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);
    __m512 t2 = _mm512_mask_mul_ps(t1, k2, d, e);
    __m512 t3 = _mm512_mask_fmadd_ps(t2, k1, f, a);
    __m512 t4 = _mm512_mask_sub_ps(t3, k2, b, c);
    
    /* Complex expression that might expand to many operands */
    return _mm512_mask_add_ps(t4, k1 & k2, 
        _mm512_mask_mul_ps(a, k1, b, c),
        _mm512_mask_sub_ps(d, k2, e, f));
}

/* FMA builtins with complex nesting */
FORCE_INLINE double complex_fma_chain(double a, double b, double c, 
                                      double d, double e, double f,
                                      double g, double h, double i) {
    /* Nested FMA calls create complex expression trees */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, i);
    double t4 = __builtin_fma(t1, t2, t3);
    double t5 = __builtin_fma(a, d, g);
    double t6 = __builtin_fma(b, e, h);
    
    /* Final complex expression */
    return __builtin_fma(__builtin_fma(t4, t5, t6),
                        __builtin_fma(a, b, c),
                        __builtin_fma(d, e, f));
}

/* Vector extension operations */
FORCE_INLINE v16sf vector_complex_op(v16sf a, v16sf b, v16sf c, 
                                     v16sf d, v16sf e, v16sf f) {
    /* Complex expression with many vector operands */
    v16sf t1 = a + b * c;
    v16sf t2 = d - e / f;
    v16sf t3 = t1 * t2 + a;
    v16sf t4 = b * c - d * e;
    v16sf t5 = f + a * b;
    
    /* Final expression with many operands */
    return (t1 * t2) + (t3 * t4) - (t5 * a) + (b * c) - (d * e) + (f * t1);
}

/* Hot function to encourage complex instruction selection */
__attribute__((hot, noinline))
void hot_vector_computation(float *result, const float *input, int n) {
    v16sf acc = {0};
    v16sf coeffs = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                    9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    for (int i = 0; i < n; i += 16) {
        v16sf data;
        for (int j = 0; j < 16 && (i + j) < n; j++) {
            ((float*)&data)[j] = input[i + j];
        }
        
        /* Complex vector operation with many operands */
        acc = vector_complex_op(acc, data, coeffs, 
                               acc * 0.5f, data * 2.0f, coeffs * 1.5f);
    }
    
    /* Store result */
    for (int i = 0; i < 16; i++) {
        result[i] = ((float*)&acc)[i];
    }
}

int main() {
    /* Test AVX-512 operations if supported */
    #ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __m512 d = _mm512_set1_ps(4.0f);
        __m512 e = _mm512_set1_ps(5.0f);
        __m512 f = _mm512_set1_ps(6.0f);
        __mmask16 k1 = 0xAAAA;
        __mmask16 k2 = 0x5555;
        
        __m512 result = avx512_complex_op(a, b, c, d, e, f, k1, k2);
        float res[16];
        _mm512_storeu_ps(res, result);
        printf("AVX-512 result[0] = %f\n", res[0]);
    }
    #endif
    
    /* Test complex FMA chain */
    {
        double fma_result = complex_fma_chain(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
        printf("FMA chain result = %f\n", fma_result);
    }
    
    /* Test inline assembly with many operands */
    {
        int vars[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        inline_asm_11_operands(&vars[0], &vars[1], &vars[2], &vars[3],
                              &vars[4], &vars[5], &vars[6], &vars[7],
                              &vars[8], &vars[9], &vars[10]);
        printf("Inline assembly vars[0] = %d\n", vars[0]);
    }
    
    /* Test hot vector computation */
    {
        float input[64];
        float result[16];
        for (int i = 0; i < 64; i++) {
            input[i] = i * 0.1f;
        }
        hot_vector_computation(result, input, 64);
        printf("Vector computation result[0] = %f\n", result[0]);
    }
    
    return 0;
}
