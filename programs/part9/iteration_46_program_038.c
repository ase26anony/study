#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimal/decimal.h>
#else
    #define HAS_DFP 0
    /* Fallback DFP types using unions */
    typedef union {
        uint64_t i;
        double f;
    } fake_decimal64;
    
    typedef union {
        struct { uint64_t lo, hi; } i;
        long double f;
    } fake_decimal128;
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
    typedef struct { double re, im; } fake_complex_double;
    typedef struct { long double re, im; } fake_complex_long_double;
#endif

/* Vector extensions if supported */
#ifdef __VECTOR_TYPES_SUPPORTED__
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
    #define HAS_VECTOR 1
#else
    #define HAS_VECTOR 0
    typedef struct { int32_t v[8]; } int32x8_t;
    typedef struct { double v[4]; } float64x4_t;
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression that might expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10) / a11);
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    /* Nested operations to increase operand count */
    return (((b1 + b2) * (b3 - b4)) / 
            ((b5 * b6) + (b7 / b8) - (b9 * b10)));
}

/* DFP operations if available */
#if HAS_DFP
static __attribute__((noinline))
_Decimal128 dfp_complex_operation(
    _Decimal128 d1, _Decimal128 d2, 
    _Decimal128 d3, _Decimal128 d4,
    _Decimal128 d5, _Decimal128 d6)
{
    /* Complex DFP expression that may expand to many RTL operands */
    return ((d1 * d2) + (d3 / d4) - (d5 * d6)) * 
           ((d1 + d2) / (d3 - d4) + (d5 + d6));
}
#endif

/* Vector reduction operation */
static __attribute__((noinline))
double vector_reduce_sum(float64x4_t vec)
{
#if HAS_VECTOR
    /* Horizontal sum that may expand to multiple operations */
    double sum = 0.0;
    sum += vec[0] + vec[1] + vec[2] + vec[3];
    
    /* Additional operations to increase operand count */
    sum = sum * 2.0 - (vec[0] * vec[1]) + (vec[2] / vec[3]);
    return sum;
#else
    /* Manual reduction for fallback */
    double sum = 0.0;
    sum += vec.v[0] + vec.v[1] + vec.v[2] + vec.v[3];
    sum = sum * 2.0 - (vec.v[0] * vec.v[1]) + (vec.v[2] / vec.v[3]);
    return sum;
#endif
}

/* Complex number operations */
#if HAS_COMPLEX
static __attribute__((noinline))
long double _Complex complex_high_precision_op(
    long double _Complex c1, long double _Complex c2,
    long double _Complex c3, long double _Complex c4)
{
    /* Complex expression that may expand real/imag parts separately */
    long double _Complex result;
    
    /* Multiple operations to increase operand count */
    result = (c1 * c2) + (c3 / c4);
    result = result * (c1 - c2) / (c3 + c4);
    
    /* Call to complex function that may expand */
    result = csqrt(result);
    
    return result;
}
#endif

int main(int argc, char *argv[])
{
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Prevent optimization */
    volatile long double volatile_result = 0.0;
    volatile double volatile_array[10] = {0};
    int volatile_index = 0;
    
    /* Initialize base values */
    double base_double[20];
    long double base_long_double[20];
    
    for (int i = 0; i < 20; i++) {
        base_double[i] = (double)(rand() % 1000) / 100.0 + 1.0;
        base_long_double[i] = (long double)(rand() % 1000) / 100.0 + 1.0;
    }
    
#if HAS_DFP
    /* Initialize DFP values */
    _Decimal64 dfp64_vals[10];
    _Decimal128 dfp128_vals[10];
    
    for (int i = 0; i < 10; i++) {
        dfp64_vals[i] = (_Decimal64)(rand() % 1000) / 100.0 + 1.0;
        dfp128_vals[i] = (_Decimal128)(rand() % 1000) / 100.0 + 1.0;
    }
#endif
    
#if HAS_COMPLEX
    /* Initialize complex values */
    long double _Complex complex_vals[10];
    for (int i = 0; i < 10; i++) {
        complex_vals[i] = base_long_double[i] + 
                         base_long_double[i+10] * I;
    }
#endif
    
#if HAS_VECTOR
    /* Initialize vector values */
    float64x4_t vector_vals[5];
    for (int i = 0; i < 5; i++) {
        vector_vals[i] = (float64x4_t){
            base_double[i*4], base_double[i*4+1],
            base_double[i*4+2], base_double[i*4+3]
        };
    }
#else
    /* Fallback vector initialization */
    float64x4_t vector_vals[5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            vector_vals[i].v[j] = base_double[i*4 + j];
        }
    }
#endif
    
    /* Main computation loop */
    for (int iteration = 0; iteration < 5; iteration++) {
        long double iteration_result = 0.0;
        
        /* Conditional execution to prevent constant folding */
        if (seed % 2 == iteration % 2) {
            /* Branch 1: Use helper with 11 arguments */
            iteration_result += helper_11_args(
                base_long_double[0], base_long_double[1],
                base_long_double[2], base_long_double[3],
                base_long_double[4], base_long_double[5],
                base_long_double[6], base_long_double[7],
                base_long_double[8], base_long_double[9],
                base_long_double[10]
            );
            
            /* Use helper with 10 arguments */
            iteration_result += helper_10_args(
                base_double[0], base_double[1], base_double[2],
                base_double[3], base_double[4], base_double[5],
                base_double[6], base_double[7], base_double[8],
                base_double[9]
            );
        } else {
            /* Branch 2: Different combination */
            iteration_result += helper_11_args(
                base_long_double[10], base_long_double[9],
                base_long_double[8], base_long_double[7],
                base_long_double[6], base_long_double[5],
                base_long_double[4], base_long_double[3],
                base_long_double[2], base_long_double[1],
                base_long_double[0]
            );
        }
        
#if HAS_DFP
        /* DFP operations - may expand to many operands */
        if (iteration % 3 != 0) {
            _Decimal128 dfp_result;
            
            /* Complex DFP expression */
            dfp_result = dfp128_vals[0] * dfp128_vals[1] + 
                        dfp128_vals[2] / dfp128_vals[3] - 
                        dfp128_vals[4] * dfp128_vals[5];
            
            /* Convert to long double for accumulation */
            iteration_result += (long double)dfp_result;
            
            /* Another DFP operation */
            dfp_result = dfp_complex_operation(
                dfp128_vals[0], dfp128_vals[1], dfp128_vals[2],
                dfp128_vals[3], dfp128_vals[4], dfp128_vals[5]
            );
            
            iteration_result += (long double)dfp_result;
        }
#endif
        
#if HAS_COMPLEX
        /* Complex number operations */
        if (iteration % 2 == 0) {
            long double _Complex c_result;
            
            /* Complex arithmetic that may expand real/imag separately */
            c_result = (complex_vals[0] * complex_vals[1]) / 
                      (complex_vals[2] - complex_vals[3]);
            
            /* More complex operations */
            c_result = complex_high_precision_op(
                complex_vals[0], complex_vals[1],
                complex_vals[2], complex_vals[3]
            );
            
            /* Extract real part */
            iteration_result += creal(c_result);
        }
#endif
        
        /* Vector reduction */
        double vec_result = vector_reduce_sum(vector_vals[iteration % 5]);
        iteration_result += vec_result;
        
        /* Store result to prevent elimination */
        volatile_array[volatile_index++ % 10] = (double)iteration_result;
        volatile_result += iteration_result;
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < 10; i++) {
        checksum += volatile_array[i];
    }
    checksum += (double)volatile_result;
    
    printf("Checksum: %.15f\n", checksum);
    
    return 0;
}
