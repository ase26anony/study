#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define DFP_SUPPORTED 1
    #include <decimal/decimal.h>
#else
    #define DFP_SUPPORTED 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define COMPLEX_SUPPORTED 1
#else
    #define COMPLEX_SUPPORTED 0
#endif

/* Vector type definitions */
#if defined(__GNUC__) && (__GNUC__ >= 4)
    #define VECTOR_SUPPORTED 1
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float64x4_t __attribute__((vector_size(32)));
    typedef double double64x4_t __attribute__((vector_size(32)));
#else
    #define VECTOR_SUPPORTED 0
    typedef struct { int32_t data[8]; } int32x8_t;
    typedef struct { float data[4]; } float64x4_t;
    typedef struct { double data[4]; } double64x4_t;
#endif

/* DFP fallback using integer arrays */
#if !DFP_SUPPORTED
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t w[2];
} decimal128_fb;

#define DEC64(x) ((decimal64_fb){x, 0})
#define DEC128(x) ((decimal128_fb){{x, 0}})
#endif

/* Complex fallback */
#if !COMPLEX_SUPPORTED
typedef struct {
    long double real;
    long double imag;
} long_double_complex;
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) 
long double helper_11_args(
    #if DFP_SUPPORTED
    _Decimal64 d1, _Decimal64 d2, _Decimal128 d3,
    #else
    decimal64_fb d1, decimal64_fb d2, decimal128_fb d3,
    #endif
    #if COMPLEX_SUPPORTED
    long double _Complex c1, long double _Complex c2,
    #else
    long_double_complex c1, long_double_complex c2,
    #endif
    int32x8_t v1, float64x4_t v2, double64x4_t v3,
    int extra1, long double extra2
) {
    long double result = 0;
    
    /* Process DFP arguments */
    #if DFP_SUPPORTED
    /* Use DFP builtins that may expand to multi-operand patterns */
    result += (long double)__builtin_dadd(d1, d2);
    result += (long double)d3;
    #else
    /* Fallback: simple integer arithmetic */
    result += (long double)d1.lo + d2.lo;
    result += (long double)d3.w[0];
    #endif
    
    /* Process complex arguments */
    #if COMPLEX_SUPPORTED
    result += creal(c1) + cimag(c1);
    result += creal(c2) + cimag(c2);
    #else
    result += c1.real + c1.imag;
    result += c2.real + c2.imag;
    #endif
    
    /* Process vector arguments */
    #if VECTOR_SUPPORTED
    /* Vector reduction - may expand to many operations */
    for (int i = 0; i < 8; i++) result += v1[i];
    for (int i = 0; i < 4; i++) result += v2[i];
    for (int i = 0; i < 4; i++) result += v3[i];
    #else
    for (int i = 0; i < 8; i++) result += v1.data[i];
    for (int i = 0; i < 4; i++) result += v2.data[i];
    for (int i = 0; i < 4; i++) result += v3.data[i];
    #endif
    
    result += extra1 + extra2;
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
long double helper_10_args(
    #if DFP_SUPPORTED
    _Decimal128 d1, _Decimal128 d2, _Decimal128 d3,
    #else
    decimal128_fb d1, decimal128_fb d2, decimal128_fb d3,
    #endif
    #if COMPLEX_SUPPORTED
    long double _Complex c1, long double _Complex c2, long double _Complex c3,
    #else
    long_double_complex c1, long_double_complex c2, long_double_complex c3,
    #endif
    int32x8_t v1, int32x8_t v2,
    long double extra
) {
    long double result = extra;
    
    /* Complex DFP arithmetic that may require many operands */
    #if DFP_SUPPORTED
    /* This DFP expression may expand to many RTL operands */
    _Decimal128 temp = __builtin_dadd(d1, __builtin_dmul(d2, d3));
    result += (long double)temp;
    #else
    result += (long double)d1.w[0] + d2.w[0] * d3.w[0];
    #endif
    
    /* Complex number operations */
    #if COMPLEX_SUPPORTED
    result += creal(c1 * c2 / c3);
    #else
    result += (c1.real * c2.real - c1.imag * c2.imag) / c3.real;
    #endif
    
    /* Vector operations */
    #if VECTOR_SUPPORTED
    for (int i = 0; i < 8; i++) result += v1[i] + v2[i];
    #else
    for (int i = 0; i < 8; i++) result += v1.data[i] + v2.data[i];
    #endif
    
    return result;
}

/* Volatile storage to prevent optimization */
volatile long double global_accumulator[10];
volatile int global_index = 0;

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize base variables */
    #if DFP_SUPPORTED
    _Decimal64 d64_a = 1.23456dd;
    _Decimal64 d64_b = 7.89012dd;
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 7.8901234567890123dl;
    _Decimal128 d128_c = 3.1415926535897932dl;
    _Decimal128 d128_d = 2.7182818284590452dl;
    #else
    decimal64_fb d64_a = DEC64(123456);
    decimal64_fb d64_b = DEC64(789012);
    decimal128_fb d128_a = DEC128(12345678901234567ULL);
    decimal128_fb d128_b = DEC128(78901234567890123ULL);
    decimal128_fb d128_c = DEC128(31415926535897932ULL);
    decimal128_fb d128_d = DEC128(27182818284590452ULL);
    #endif
    
    #if COMPLEX_SUPPORTED
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L + 4.0LI;
    long double _Complex cc = 5.0L + 6.0LI;
    long double _Complex cd = 7.0L + 8.0LI;
    #else
    long_double_complex ca = {1.0L, 2.0L};
    long_double_complex cb = {3.0L, 4.0L};
    long_double_complex cc = {5.0L, 6.0L};
    long_double_complex cd = {7.0L, 8.0L};
    #endif
    
    /* Initialize vectors */
    int32x8_t vec_int1, vec_int2;
    float64x4_t vec_float;
    double64x4_t vec_double;
    
    for (int i = 0; i < 8; i++) {
        #if VECTOR_SUPPORTED
        vec_int1[i] = seed + i;
        vec_int2[i] = seed * 2 + i;
        #else
        vec_int1.data[i] = seed + i;
        vec_int2.data[i] = seed * 2 + i;
        #endif
    }
    
    for (int i = 0; i < 4; i++) {
        #if VECTOR_SUPPORTED
        vec_float[i] = (seed + i) * 1.1f;
        vec_double[i] = (seed + i) * 1.234;
        #else
        vec_float.data[i] = (seed + i) * 1.1f;
        vec_double.data[i] = (seed + i) * 1.234;
        #endif
    }
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = (iter % 2 == 0) ? 1 : 0;
        long double result = 0;
        
        if (condition) {
            /* Branch 1: Complex DFP arithmetic */
            #if DFP_SUPPORTED
            _Decimal128 d128_result = __builtin_dadd(
                __builtin_dmul(d128_a, d128_b),
                __builtin_ddiv(d128_c, d128_d)
            );
            #else
            decimal128_fb d128_result = DEC128(
                d128_a.w[0] * d128_b.w[0] + d128_c.w[0] / d128_d.w[0]
            );
            #endif
            
            /* Complex arithmetic */
            #if COMPLEX_SUPPORTED
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            #else
            long_double_complex cl_result = {
                (ca.real * cb.real - ca.imag * cb.imag) / (cc.real - cd.real),
                (ca.real * cb.imag + ca.imag * cb.real) / (cc.imag - cd.imag)
            };
            #endif
            
            /* Vector reduction */
            long double vec_sum = 0;
            #if VECTOR_SUPPORTED
            for (int i = 0; i < 8; i++) vec_sum += vec_int1[i];
            for (int i = 0; i < 4; i++) vec_sum += vec_float[i];
            #else
            for (int i = 0; i < 8; i++) vec_sum += vec_int1.data[i];
            for (int i = 0; i < 4; i++) vec_sum += vec_float.data[i];
            #endif
            
            /* Call helper with 11 arguments - may trigger 11-operand expansion */
            result = helper_11_args(
                #if DFP_SUPPORTED
                d64_a, d64_b, d128_result,
                #else
                d64_a, d64_b, d128_result,
                #endif
                #if COMPLEX_SUPPORTED
                cl_result, ca * cb,
                #else
                cl_result, (long_double_complex){ca.real * cb.real, ca.imag * cb.imag},
                #endif
                vec_int1, vec_float, vec_double,
                iter, vec_sum
            );
        } else {
            /* Branch 2: Different complex operations */
            #if COMPLEX_SUPPORTED
            long double _Complex cl_result2 = ca + cb * cc - cd;
            #ifdef __STDC_IEC_559_COMPLEX__
            /* Use complex math functions that may expand */
            long double _Complex csqrt_result = csqrt(ca);
            long double _Complex cpow_result = cpow(cb, cc);
            #endif
            #else
            long_double_complex cl_result2 = {
                ca.real + cb.real * cc.real - cd.real,
                ca.imag + cb.imag * cc.imag - cd.imag
            };
            #endif
            
            /* More DFP operations */
            #if DFP_SUPPORTED
            _Decimal128 d128_result2 = __builtin_dadd(
                __builtin_dsub(d128_a, d128_b),
                __builtin_dmul(d128_c, d128_d)
            );
            #else
            decimal128_fb d128_result2 = DEC128(
                d128_a.w[0] - d128_b.w[0] + d128_c.w[0] * d128_d.w[0]
            );
            #endif
            
            /* Call helper with 10 arguments - may trigger 10-operand expansion */
            result = helper_10_args(
                #if DFP_SUPPORTED
                d128_result2, d128_a, d128_b,
                #else
                d128_result2, d128_a, d128_b,
                #endif
                #if COMPLEX_SUPPORTED
                #ifdef __STDC_IEC_559_COMPLEX__
                csqrt_result, cpow_result, cl_result2,
                #else
                ca, cb, cl_result2,
                #endif
                #else
                ca, cb, cl_result2,
                #endif
                vec_int1, vec_int2,
                (long double)iter * 2.5L
            );
        }
        
        /* Store result to volatile memory to prevent optimization */
        if (global_index < 10) {
            global_accumulator[global_index++] = result;
        }
        
        /* Modify some inputs for next iteration */
        #if DFP_SUPPORTED
        d64_a = __builtin_dadd(d64_a, 1.0dd);
        d128_a = __builtin_dadd(d128_a, 1.0dl);
        #else
        d64_a.lo++;
        d128_a.w[0]++;
        #endif
        
        #if COMPLEX_SUPPORTED
        ca += 0.5L + 0.5LI;
        #else
        ca.real += 0.5L;
        ca.imag += 0.5L;
        #endif
    }
    
    /* Compute checksum */
    long double checksum = 0;
    for (int i = 0; i < global_index && i < 10; i++) {
        checksum += global_accumulator[i];
    }
    
    printf("Checksum: %Lf\n", checksum);
    return 0;
}
