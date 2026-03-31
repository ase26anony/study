#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimal/decimal.h>
#else
    #define HAS_DFP 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
#endif

/* DFP fallback using integer arrays */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dfp128_fallback;

/* Vector types for GCC vector extensions */
#if defined(__VECTOR_TYPES_SUPPORTED__) || defined(__GNUC__)
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
#endif

/* Helper functions with many arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
#if HAS_DFP
_Decimal128 helper_10_args_dfp(_Decimal128 a1, _Decimal128 a2, _Decimal128 a3,
                               _Decimal128 a4, _Decimal128 a5, _Decimal128 a6,
                               _Decimal128 a7, _Decimal128 a8, _Decimal128 a9,
                               _Decimal128 a10) {
    /* Complex DFP expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + (a7 - a8) * (a9 + a10)) /
           ((a1 + a2) - (a3 * a4) + (a5 / a6) - (a7 * a8) + (a9 - a10));
}
#else
dfp128_fallback helper_10_args_fallback(dfp128_fallback a1, dfp128_fallback a2,
                                        dfp128_fallback a3, dfp128_fallback a4,
                                        dfp128_fallback a5, dfp128_fallback a6,
                                        dfp128_fallback a7, dfp128_fallback a8,
                                        dfp128_fallback a9, dfp128_fallback a10) {
    /* Simulate DFP operations using integer arithmetic */
    dfp128_fallback result;
    result.lo = a1.lo + a2.lo + a3.lo + a4.lo + a5.lo + 
                a6.lo + a7.lo + a8.lo + a9.lo + a10.lo;
    result.hi = a1.hi + a2.hi + a3.hi + a4.hi + a5.hi + 
                a6.hi + a7.hi + a8.hi + a9.hi + a10.hi;
    return result;
}
#endif

#if HAS_COMPLEX
static __attribute__((noinline))
long double _Complex helper_11_args_complex(long double _Complex c1,
                                           long double _Complex c2,
                                           long double _Complex c3,
                                           long double _Complex c4,
                                           long double _Complex c5,
                                           long double _Complex c6,
                                           long double _Complex c7,
                                           long double _Complex c8,
                                           long double _Complex c9,
                                           long double _Complex c10,
                                           long double _Complex c11) {
    /* Complex expression that may require many operands during expansion */
    return (c1 * c2 + c3 / c4 - c5 * c6 + c7 * c8 - c9 * c10) * c11 +
           (c1 / c2 - c3 * c4 + c5 / c6 - c7 * c8 + c9 / c10) / c11;
}
#endif

#if HAS_VECTORS
static __attribute__((noinline))
double vector_reduction_with_accumulation(float64x4_t v1, float64x4_t v2,
                                         float64x4_t v3, float64x4_t v4,
                                         float64x4_t v5, float64x4_t v6,
                                         float64x4_t v7, float64x4_t v8,
                                         float64x4_t v9, float64x4_t v10) {
    /* Vector operations that may expand to many RTL operands */
    float64x4_t sum1 = v1 + v2 + v3 + v4 + v5;
    float64x4_t sum2 = v6 + v7 + v8 + v9 + v10;
    float64x4_t prod = sum1 * sum2;
    
    /* Horizontal reduction */
    double result = 0.0;
    for (int i = 0; i < 4; i++) {
        result += prod[i];
    }
    
    /* Additional accumulation to increase register pressure */
    result += sum1[0] * sum1[1] + sum1[2] * sum1[3];
    result += sum2[0] * sum2[1] + sum2[2] * sum2[3];
    
    return result;
}
#endif

/* Volatile storage to prevent dead code elimination */
static volatile double global_accumulator[10];
static volatile int global_index = 0;

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789e10DL;
    _Decimal64 d64_b = 9.87654321e9DL;
    _Decimal128 d128_a = 1.2345678901234567890123456789e30DL;
    _Decimal128 d128_b = 9.8765432109876543210987654321e29DL;
    _Decimal128 d128_c = 5.5555555555555555555555555555e28DL;
    _Decimal128 d128_d = 2.2222222222222222222222222222e27DL;
#else
    dfp128_fallback d128_a = { .lo = 0x123456789ABCDEF0ULL, .hi = 0xFEDCBA9876543210ULL };
    dfp128_fallback d128_b = { .lo = 0x0FEDCBA987654321ULL, .hi = 0x0123456789ABCDEFULL };
    dfp128_fallback d128_c = { .lo = 0x5555555555555555ULL, .hi = 0xAAAAAAAAAAAAAAAAULL };
    dfp128_fallback d128_d = { .lo = 0x2222222222222222ULL, .hi = 0x4444444444444444ULL };
#endif

#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L - 4.0LI;
    long double _Complex cc = 5.0L + 6.0LI;
    long double _Complex cd = 7.0L - 8.0LI;
    long double _Complex ce = 9.0L + 10.0LI;
#endif

#if HAS_VECTORS
    float64x4_t vec1 = {1.0, 2.0, 3.0, 4.0};
    float64x4_t vec2 = {5.0, 6.0, 7.0, 8.0};
    float64x4_t vec3 = {9.0, 10.0, 11.0, 12.0};
    float64x4_t vec4 = {13.0, 14.0, 15.0, 16.0};
    float64x4_t vec5 = {17.0, 18.0, 19.0, 20.0};
#endif
    
    double checksum = 0.0;
    
    /* Main computation loop - designed to create many operands */
    for (int iteration = 0; iteration < 3; iteration++) {
        volatile int condition = rand() % 2; /* Prevent constant folding */
        
        if (condition) {
            /* DFP arithmetic with complex expressions */
#if HAS_DFP
            _Decimal128 d128_result;
            
            /* Complex DFP expression that may expand to many operands */
            d128_result = ((d128_a * d128_b) + (d128_c / d128_d)) *
                         ((d128_a + d128_b) - (d128_c * d128_d)) /
                         ((d128_a - d128_b) + (d128_c + d128_d)) *
                         ((d128_a / d128_b) - (d128_c - d128_d));
            
            /* Call helper with 10 DFP arguments */
            _Decimal128 helper_result = helper_10_args_dfp(
                d128_result, d128_a, d128_b, d128_c, d128_d,
                d128_result * 2.0DL, d128_a / 3.0DL,
                d128_b + 4.0DL, d128_c - 5.0DL, d128_d * 6.0DL);
            
            global_accumulator[global_index++] = (double)helper_result;
#else
            dfp128_fallback d128_result;
            d128_result.lo = d128_a.lo + d128_b.lo + d128_c.lo + d128_d.lo;
            d128_result.hi = d128_a.hi + d128_b.hi + d128_c.hi + d128_d.hi;
            
            dfp128_fallback helper_result = helper_10_args_fallback(
                d128_result, d128_a, d128_b, d128_c, d128_d,
                d128_result, d128_a, d128_b, d128_c, d128_d);
            
            global_accumulator[global_index++] = (double)helper_result.lo;
#endif
        } else {
#if HAS_COMPLEX
            /* Complex number operations */
            long double _Complex cl_result;
            
            /* Complex expression that may require many operands */
            cl_result = (ca * cb) / (cc - cd) + 
                       (ca + cb) * (cc * cd) -
                       (ca - cb) / (cc + cd) +
                       (ca * cd) * (cb / cc);
            
            /* Additional complex operations */
            long double _Complex cl_pow = cl_result * cl_result;
            long double _Complex cl_sqrt = cl_pow + 1.0L;
            
            /* Call helper with 11 complex arguments */
            long double _Complex complex_helper_result = helper_11_args_complex(
                cl_result, ca, cb, cc, cd, ce,
                cl_pow, cl_sqrt, ca * 2.0L, cb / 3.0L, cc + 4.0L);
            
            global_accumulator[global_index++] = creal(complex_helper_result);
#endif
        }
        
        /* Vector operations in all iterations */
#if HAS_VECTORS
        double vec_result = vector_reduction_with_accumulation(
            vec1, vec2, vec3, vec4, vec5,
            vec1 * 2.0, vec2 / 3.0, vec3 + 4.0, vec4 - 5.0, vec5 * 6.0);
        
        global_accumulator[global_index++] = vec_result;
#endif
        
        /* Mix operations to create complex dependency chains */
        if (iteration % 2 == 0) {
#if HAS_DFP && HAS_COMPLEX
            /* Cross-type operations */
            _Decimal128 mixed_result = (_Decimal128)creal(ca) * d128_a;
            global_accumulator[global_index++] = (double)mixed_result;
#endif
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < global_index && i < 10; i++) {
        checksum += global_accumulator[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Iterations completed: %d\n", global_index);
    
    return 0;
}
