#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Enable complex.h if available */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#endif

/* Feature detection and fallbacks */
#if defined(__DECIMAL_BID_FORMAT__) || defined(__DECIMAL_DPD_FORMAT__)
#define HAS_DFP 1
#else
#define HAS_DFP 0
#endif

/* Vector type definitions */
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define HAS_VECTOR_TYPES 1
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));
#else
#define HAS_VECTOR_TYPES 0
#endif

/* Complex fallback for systems without complex.h */
#ifndef HAS_COMPLEX_TYPES
typedef struct {
    long double real;
    long double imag;
} long_double_complex;
#endif

/* DFP fallback using integer arrays */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fallback;

typedef struct {
    uint64_t parts[2];
} decimal128_fallback;
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
long double complex_helper_11(
#ifdef __STDC_IEC_559_COMPLEX__
    long double _Complex a1, long double _Complex a2,
    long double _Complex a3, long double _Complex a4,
    long double _Complex a5, long double _Complex a6,
    long double _Complex a7, long double _Complex a8,
    long double _Complex a9, long double _Complex a10,
    long double _Complex a11
#else
    long_double_complex a1, long_double_complex a2,
    long_double_complex a3, long_double_complex a4,
    long_double_complex a5, long_double_complex a6,
    long_double_complex a7, long_double_complex a8,
    long_double_complex a9, long_double_complex a10,
    long_double_complex a11
#endif
) {
    /* Combine all arguments with complex arithmetic */
#ifdef __STDC_IEC_559_COMPLEX__
    long double _Complex result = a1 + a2 - a3 * a4 + a5 / a6 + a7 * a8 - a9 + a10 * a11;
    return result;
#else
    long_double_complex result;
    result.real = a1.real + a2.real - a3.real * a4.real + a5.real / a6.real + 
                  a7.real * a8.real - a9.real + a10.real * a11.real;
    result.imag = a1.imag + a2.imag - a3.imag * a4.imag + a5.imag / a6.imag + 
                  a7.imag * a8.imag - a9.imag + a10.imag * a11.imag;
    return result;
#endif
}

/* Helper function with 10 arguments for DFP operations */
static __attribute__((noinline))
#if HAS_DFP
_Decimal128 dfp_helper_10(
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3, _Decimal64 d4,
    _Decimal64 d5, _Decimal64 d6, _Decimal64 d7, _Decimal64 d8,
    _Decimal128 d9, _Decimal128 d10
) {
    /* Complex DFP expression that may expand to many operands */
    _Decimal128 result = (_Decimal128)d1 * (_Decimal128)d2 + 
                        (_Decimal128)d3 / (_Decimal128)d4 -
                        (_Decimal128)d5 * (_Decimal128)d6 +
                        (_Decimal128)d7 * (_Decimal128)d8 +
                        d9 * d10;
    return result;
}
#else
decimal128_fallback dfp_helper_10(
    decimal64_fallback d1, decimal64_fallback d2, decimal64_fallback d3, 
    decimal64_fallback d4, decimal64_fallback d5, decimal64_fallback d6,
    decimal64_fallback d7, decimal64_fallback d8,
    decimal128_fallback d9, decimal128_fallback d10
) {
    /* Simulate DFP with integer arithmetic */
    decimal128_fallback result;
    uint64_t t1_lo = d1.lo * d2.lo;
    uint64_t t1_hi = d1.hi * d2.hi;
    uint64_t t2_lo = d3.lo / (d4.lo ? d4.lo : 1);
    uint64_t t2_hi = d3.hi / (d4.hi ? d4.hi : 1);
    
    result.parts[0] = t1_lo + t2_lo - d5.lo * d6.lo + d7.lo * d8.lo + d9.parts[0] * d10.parts[0];
    result.parts[1] = t1_hi + t2_hi - d5.hi * d6.hi + d7.hi * d8.hi + d9.parts[1] * d10.parts[1];
    return result;
}
#endif

/* Vector reduction helper */
#if HAS_VECTOR_TYPES
static __attribute__((noinline))
float vector_reduce_accumulate(float32x4_t v1, float32x4_t v2, float32x4_t v3,
                               float32x4_t v4, float32x4_t v5, float accumulator) {
    /* Horizontal reduction that may create many intermediate operands */
    float sum1 = v1[0] + v1[1] + v1[2] + v1[3];
    float sum2 = v2[0] + v2[1] + v2[2] + v2[3];
    float sum3 = v3[0] + v3[1] + v3[2] + v3[3];
    float sum4 = v4[0] + v4[1] + v4[2] + v4[3];
    float sum5 = v5[0] + v5[1] + v5[2] + v5[3];
    
    return accumulator + sum1 * sum2 - sum3 / (sum4 + 1.0f) + sum5;
}
#endif

/* Volatile storage to prevent dead code elimination */
static volatile double result_storage[10];
static volatile int storage_idx = 0;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_vals[8];
    _Decimal128 d128_vals[4];
    
    for (int i = 0; i < 8; i++) {
        d64_vals[i] = (_Decimal64)(1.0 + (rand() % 100) / 100.0);
    }
    for (int i = 0; i < 4; i++) {
        d128_vals[i] = (_Decimal128)(1.0 + (rand() % 100) / 100.0);
    }
#else
    decimal64_fallback d64_vals[8];
    decimal128_fallback d128_vals[4];
    
    for (int i = 0; i < 8; i++) {
        d64_vals[i].lo = 1 + rand() % 100;
        d64_vals[i].hi = 1 + rand() % 100;
    }
    for (int i = 0; i < 4; i++) {
        d128_vals[i].parts[0] = 1 + rand() % 100;
        d128_vals[i].parts[1] = 1 + rand() % 100;
    }
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    long double _Complex complex_vals[12];
    for (int i = 0; i < 12; i++) {
        complex_vals[i] = (rand() % 100) / 100.0L + 
                         (rand() % 100) / 100.0L * I;
    }
#else
    long_double_complex complex_vals[12];
    for (int i = 0; i < 12; i++) {
        complex_vals[i].real = (rand() % 100) / 100.0L;
        complex_vals[i].imag = (rand() % 100) / 100.0L;
    }
#endif

#if HAS_VECTOR_TYPES
    float32x4_t vector_vals[6];
    for (int i = 0; i < 6; i++) {
        vector_vals[i] = (float32x4_t){
            (float)(rand() % 100) / 100.0f,
            (float)(rand() % 100) / 100.0f,
            (float)(rand() % 100) / 100.0f,
            (float)(rand() % 100) / 100.0f
        };
    }
#endif

    /* Main computation loop */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = rand() % 2;
        double loop_result = 0.0;
        
        if (condition) {
            /* Branch 1: Complex DFP operations */
#if HAS_DFP
            _Decimal128 d128_result = d64_vals[0] * d64_vals[1] + 
                                     d64_vals[2] / d64_vals[3] -
                                     d64_vals[4] * d64_vals[5] +
                                     d128_vals[0] * d128_vals[1];
            
            /* Call helper with 10 DFP arguments */
            _Decimal128 helper_result = dfp_helper_10(
                d64_vals[0], d64_vals[1], d64_vals[2], d64_vals[3],
                d64_vals[4], d64_vals[5], d64_vals[6], d64_vals[7],
                d128_vals[0], d128_vals[1]
            );
            
            loop_result = (double)d128_result + (double)helper_result;
#else
            decimal128_fallback d128_result;
            d128_result.parts[0] = d64_vals[0].lo * d64_vals[1].lo + 
                                  d64_vals[2].lo / (d64_vals[3].lo ? d64_vals[3].lo : 1) -
                                  d64_vals[4].lo * d64_vals[5].lo +
                                  d128_vals[0].parts[0] * d128_vals[1].parts[0];
            d128_result.parts[1] = d64_vals[0].hi * d64_vals[1].hi + 
                                  d64_vals[2].hi / (d64_vals[3].hi ? d64_vals[3].hi : 1) -
                                  d64_vals[4].hi * d64_vals[5].hi +
                                  d128_vals[0].parts[1] * d128_vals[1].parts[1];
            
            decimal128_fallback helper_result = dfp_helper_10(
                d64_vals[0], d64_vals[1], d64_vals[2], d64_vals[3],
                d64_vals[4], d64_vals[5], d64_vals[6], d64_vals[7],
                d128_vals[0], d128_vals[1]
            );
            
            loop_result = (double)d128_result.parts[0] + (double)d128_result.parts[1] +
                         (double)helper_result.parts[0] + (double)helper_result.parts[1];
#endif
        } else {
            /* Branch 2: Complex number operations */
#ifdef __STDC_IEC_559_COMPLEX__
            long double _Complex complex_result = 
                (complex_vals[0] * complex_vals[1]) / (complex_vals[2] - complex_vals[3]) +
                (complex_vals[4] * complex_vals[5]) - (complex_vals[6] / complex_vals[7]);
            
            /* Call helper with 11 complex arguments */
            long double _Complex helper_complex = complex_helper_11(
                complex_vals[0], complex_vals[1], complex_vals[2], complex_vals[3],
                complex_vals[4], complex_vals[5], complex_vals[6], complex_vals[7],
                complex_vals[8], complex_vals[9], complex_vals[10]
            );
            
            loop_result = creal(complex_result) + cimag(complex_result) +
                         creal(helper_complex) + cimag(helper_complex);
#else
            long_double_complex complex_result;
            complex_result.real = (complex_vals[0].real * complex_vals[1].real) / 
                                 (complex_vals[2].real - complex_vals[3].real) +
                                 (complex_vals[4].real * complex_vals[5].real) - 
                                 (complex_vals[6].real / complex_vals[7].real);
            complex_result.imag = (complex_vals[0].imag * complex_vals[1].imag) / 
                                 (complex_vals[2].imag - complex_vals[3].imag) +
                                 (complex_vals[4].imag * complex_vals[5].imag) - 
                                 (complex_vals[6].imag / complex_vals[7].imag);
            
            long_double_complex helper_complex = complex_helper_11(
                complex_vals[0], complex_vals[1], complex_vals[2], complex_vals[3],
                complex_vals[4], complex_vals[5], complex_vals[6], complex_vals[7],
                complex_vals[8], complex_vals[9], complex_vals[10]
            );
            
            loop_result = complex_result.real + complex_result.imag +
                         helper_complex.real + helper_complex.imag;
#endif
            
#if HAS_VECTOR_TYPES
            /* Add vector reduction */
            float vec_acc = 0.0f;
            for (int i = 0; i < 2; i++) {
                vec_acc = vector_reduce_accumulate(
                    vector_vals[0], vector_vals[1], vector_vals[2],
                    vector_vals[3], vector_vals[4], vec_acc
                );
            }
            loop_result += vec_acc;
#endif
        }
        
        /* Store result to prevent optimization */
        if (storage_idx < 10) {
            result_storage[storage_idx++] = loop_result;
        }
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < storage_idx; i++) {
        checksum += result_storage[i];
    }
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
