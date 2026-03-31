#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimal/decimal.h>
#else
    #define HAS_DFP 0
    /* Fallback DFP types using unions */
    typedef union {
        unsigned long long ll[2];
        double d;
    } fake_decimal64;
    
    typedef union {
        unsigned long long ll[4];
        long double ld;
    } fake_decimal128;
    
    #define _Decimal64 fake_decimal64
    #define _Decimal128 fake_decimal128
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
    typedef struct {
        long double re, im;
    } fake_complex;
    #define _Complex fake_complex
#endif

/* Vector type definitions */
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));

/* Global volatile to prevent optimization */
volatile long double global_sink = 0.0L;

/* Helper functions with many arguments (10-11) */
static __attribute__((noinline))
long double helper_10_args(_Decimal64 d1, _Decimal64 d2, 
                          _Decimal128 d3, _Decimal128 d4,
                          long double _Complex c1, long double _Complex c2,
                          int32x4_t v1, int32x4_t v2,
                          float64x2_t fv1, float64x2_t fv2) {
    long double result = 0.0L;
    
    /* Process DFP values */
    #if HAS_DFP
    result += (long double)d1 + (long double)d2;
    result += (long double)d3 + (long double)d4;
    #else
    result += d1.d + d2.d;
    result += d3.ld + d4.ld;
    #endif
    
    /* Process complex values */
    #if HAS_COMPLEX
    result += creall(c1) + creall(c2);
    result += cimagl(c1) + cimagl(c2);
    #else
    result += ((fake_complex*)&c1)->re + ((fake_complex*)&c1)->im;
    result += ((fake_complex*)&c2)->re + ((fake_complex*)&c2)->im;
    #endif
    
    /* Process vectors */
    for (int i = 0; i < 4; i++) {
        result += v1[i] + v2[i];
    }
    
    for (int i = 0; i < 2; i++) {
        result += fv1[i] + fv2[i];
    }
    
    return result;
}

static __attribute__((noinline))
long double helper_11_args(_Decimal64 d1, _Decimal64 d2, _Decimal64 d3,
                          _Decimal128 d4, _Decimal128 d5,
                          long double _Complex c1, long double _Complex c2,
                          int32x4_t v1, int32x4_t v2,
                          float64x2_t fv1, float64x2_t fv2) {
    long double result = 0.0L;
    
    /* Process DFP values */
    #if HAS_DFP
    result += (long double)d1 + (long double)d2 + (long double)d3;
    result += (long double)d4 + (long double)d5;
    #else
    result += d1.d + d2.d + d3.d;
    result += d4.ld + d5.ld;
    #endif
    
    /* Process complex values */
    #if HAS_COMPLEX
    result += creall(c1) + creall(c2);
    result += cimagl(c1) + cimagl(c2);
    #else
    result += ((fake_complex*)&c1)->re + ((fake_complex*)&c1)->im;
    result += ((fake_complex*)&c2)->re + ((fake_complex*)&c2)->im;
    #endif
    
    /* Process vectors */
    for (int i = 0; i < 4; i++) {
        result += v1[i] + v2[i];
    }
    
    for (int i = 0; i < 2; i++) {
        result += fv1[i] + fv2[i];
    }
    
    return result;
}

/* DFP arithmetic with many operands */
static __attribute__((noinline))
_Decimal128 dfp_complex_expression(_Decimal128 a, _Decimal128 b,
                                   _Decimal128 c, _Decimal128 d,
                                   _Decimal128 e, _Decimal128 f) {
    /* This should expand to many operands */
    #if HAS_DFP
    return a * b + c / d - e * f + a / c + b * d - e / f;
    #else
    fake_decimal128 result;
    result.ld = a.ld * b.ld + c.ld / d.ld - e.ld * f.ld + 
                a.ld / c.ld + b.ld * d.ld - e.ld / f.ld;
    return result;
    #endif
}

/* Complex arithmetic with many operands */
static __attribute__((noinline))
long double _Complex complex_complex_expression(long double _Complex a,
                                                long double _Complex b,
                                                long double _Complex c,
                                                long double _Complex d) {
    #if HAS_COMPLEX
    /* Complex expression that may expand to many real/imaginary operations */
    return (a * b + c * d) / (a - b) * (c + d) - (a / b) + (c * d);
    #else
    fake_complex result;
    fake_complex* pa = (fake_complex*)&a;
    fake_complex* pb = (fake_complex*)&b;
    fake_complex* pc = (fake_complex*)&c;
    fake_complex* pd = (fake_complex*)&d;
    
    /* Manual complex arithmetic */
    result.re = (pa->re * pb->re - pa->im * pb->im) + 
                (pc->re * pd->re - pc->im * pd->im);
    result.im = (pa->re * pb->im + pa->im * pb->re) + 
                (pc->re * pd->im + pc->im * pd->re);
    
    return *(long double _Complex*)&result;
    #endif
}

/* Vector reduction with accumulation */
static __attribute__((noinline))
int32_t vector_reduction(int32x4_t v1, int32x4_t v2, int32x4_t v3) {
    /* Horizontal reduction that may create many intermediate operands */
    int32x4_t sum1 = v1 + v2;
    int32x4_t sum2 = v2 + v3;
    int32x4_t prod = v1 * v3;
    
    /* Manual horizontal reduction */
    int32_t result = 0;
    for (int i = 0; i < 4; i++) {
        result += sum1[i] + sum2[i] + prod[i];
    }
    
    /* Additional operations to increase operand count */
    result += (sum1[0] * sum2[1]) - (sum1[2] / (sum2[3] ? sum2[3] : 1));
    result += (prod[0] << 2) | (prod[1] >> 3);
    
    return result;
}

int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize variables with deterministic but "random" values */
    #if HAS_DFP
    _Decimal64 d64_vals[4] = {1.0dd, 2.0dd, 3.0dd, 4.0dd};
    _Decimal128 d128_vals[6] = {1.0dl, 2.0dl, 3.0dl, 4.0dl, 5.0dl, 6.0dl};
    #else
    fake_decimal64 d64_vals[4];
    fake_decimal128 d128_vals[6];
    for (int i = 0; i < 4; i++) {
        d64_vals[i].d = (double)(i + 1);
        d64_vals[i].ll[0] = i + 1;
        d64_vals[i].ll[1] = 0;
    }
    for (int i = 0; i < 6; i++) {
        d128_vals[i].ld = (long double)(i + 1);
        for (int j = 0; j < 4; j++) {
            d128_vals[i].ll[j] = (i + 1) * (j + 1);
        }
    }
    #endif
    
    long double _Complex complex_vals[4];
    for (int i = 0; i < 4; i++) {
        #if HAS_COMPLEX
        complex_vals[i] = (i + 1) + (i + 2) * I;
        #else
        fake_complex fc = {(long double)(i + 1), (long double)(i + 2)};
        complex_vals[i] = *(long double _Complex*)&fc;
        #endif
    }
    
    int32x4_t int_vecs[3];
    float64x2_t float_vecs[2];
    
    for (int i = 0; i < 3; i++) {
        int_vecs[i] = (int32x4_t){seed + i, seed + i + 1, 
                                  seed + i + 2, seed + i + 3};
    }
    
    for (int i = 0; i < 2; i++) {
        float_vecs[i] = (float64x2_t){(double)(seed + i) / 10.0, 
                                      (double)(seed + i + 1) / 10.0};
    }
    
    /* Storage for results to prevent optimization */
    long double results[5] = {0};
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = seed % 2;
        
        if (condition) {
            /* Branch 1: DFP-heavy computations */
            _Decimal128 dfp_result = dfp_complex_expression(
                d128_vals[0], d128_vals[1], d128_vals[2],
                d128_vals[3], d128_vals[4], d128_vals[5]);
            
            long double _Complex complex_result = complex_complex_expression(
                complex_vals[0], complex_vals[1], 
                complex_vals[2], complex_vals[3]);
            
            int32_t vec_result = vector_reduction(
                int_vecs[0], int_vecs[1], int_vecs[2]);
            
            /* Call helper with 10 arguments */
            results[iter * 2] = helper_10_args(
                d64_vals[0], d64_vals[1],
                dfp_result, d128_vals[2],
                complex_result, complex_vals[0],
                int_vecs[0], int_vecs[1],
                float_vecs[0], float_vecs[1]);
        } else {
            /* Branch 2: Different computation pattern */
            _Decimal128 dfp_result2 = dfp_complex_expression(
                d128_vals[5], d128_vals[4], d128_vals[3],
                d128_vals[2], d128_vals[1], d128_vals[0]);
            
            long double _Complex complex_result2 = complex_complex_expression(
                complex_vals[3], complex_vals[2], 
                complex_vals[1], complex_vals[0]);
            
            /* Call helper with 11 arguments */
            results[iter * 2 + 1] = helper_11_args(
                d64_vals[0], d64_vals[1], d64_vals[2],
                dfp_result2, d128_vals[0],
                complex_result2, complex_vals[1],
                int_vecs[0], int_vecs[2],
                float_vecs[0], float_vecs[1]);
        }
        
        /* Modify values slightly for next iteration */
        #if HAS_DFP
        d64_vals[iter % 4] += 0.5dd;
        d128_vals[iter % 6] += 0.25dl;
        #else
        d64_vals[iter % 4].d += 0.5;
        d128_vals[iter % 6].ld += 0.25L;
        #endif
        
        #if HAS_COMPLEX
        complex_vals[iter % 4] += 0.1L + 0.1L * I;
        #else
        fake_complex* fc = (fake_complex*)&complex_vals[iter % 4];
        fc->re += 0.1L;
        fc->im += 0.1L;
        #endif
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
        global_sink = results[i]; /* Prevent dead code elimination */
    }
    
    printf("Checksum: %Lf\n", checksum);
    return 0;
}
