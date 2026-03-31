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
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
#endif

/* Vector type fallbacks */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
    #define HAS_VECTOR 1
    typedef int32_t int32x4_t __attribute__((vector_size(16)));
    typedef float float32x4_t __attribute__((vector_size(16)));
    typedef double float64x2_t __attribute__((vector_size(16)));
#else
    #define HAS_VECTOR 0
    typedef struct { int32_t v[4]; } int32x4_t;
    typedef struct { float v[4]; } float32x4_t;
    typedef struct { double v[2]; } float64x2_t;
#endif

/* DFP fallback structures */
#if !HAS_DFP
typedef struct { uint64_t hi, lo; } decimal64_fb;
typedef struct { uint64_t w[2]; } decimal128_fb;
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10) / a11);
}

/* Helper with 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    /* Another complex expression */
    return (b1 + b2) * (b3 - b4) / (b5 * b6) + 
           (b7 * b8) - (b9 / b10);
}

/* Vector reduction with accumulation */
#if HAS_VECTOR
static float vector_reduce_sum(float32x4_t v)
{
    float sum = 0.0f;
    sum += v[0] + v[1] + v[2] + v[3];
    return sum;
}

static double vector_reduce_double(float64x2_t v)
{
    return v[0] + v[1];
}
#else
static float vector_reduce_sum(float32x4_t v)
{
    return v.v[0] + v.v[1] + v.v[2] + v.v[3];
}

static double vector_reduce_double(float64x2_t v)
{
    return v.v[0] + v.v[1];
}
#endif

/* Complex number operations */
#if HAS_COMPLEX
static long double complex complex_operation(
    long double complex c1, long double complex c2,
    long double complex c3, long double complex c4)
{
    /* Complex expression that may expand to many real/imag operations */
    return (c1 * c2) / (c3 - c4) + csqrt(c1 * c2);
}
#endif

/* DFP operations with fallbacks */
#if HAS_DFP
static _Decimal128 dfp_operation(
    _Decimal128 d1, _Decimal128 d2,
    _Decimal128 d3, _Decimal128 d4)
{
    /* DFP expression that may use multi-operand builtins */
    return d1 * d2 + d3 / d4;
}
#else
static double dfp_fallback_operation(
    double d1, double d2, double d3, double d4)
{
    /* Simulate DFP with double precision */
    return d1 * d2 + d3 / d4;
}
#endif

/* Main computation loop */
int main(int argc, char *argv[])
{
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Initialize base variables */
    volatile double result_accumulator = 0.0;
    double results[5] = {0};
    
    /* Initialize DFP-like values */
    double dfp1 = 1.23456789;
    double dfp2 = 9.87654321;
    double dfp3 = 3.14159265;
    double dfp4 = 2.71828182;
    
#if HAS_DFP
    _Decimal128 d128_a = 1.23456789dl;
    _Decimal128 d128_b = 9.87654321dl;
    _Decimal128 d128_c = 3.14159265dl;
    _Decimal128 d128_d = 2.71828182dl;
#endif
    
#if HAS_COMPLEX
    long double complex ca = 1.0 + 2.0 * I;
    long double complex cb = 3.0 - 4.0 * I;
    long double complex cc = 5.0 + 6.0 * I;
    long double complex cd = 7.0 - 8.0 * I;
#endif
    
#if HAS_VECTOR
    float32x4_t vec_float = {1.1f, 2.2f, 3.3f, 4.4f};
    float64x2_t vec_double = {5.5, 6.6};
#endif
    
    /* Main computation loop - 3 iterations */
    for (int iter = 0; iter < 3; iter++) {
        double iter_result = 0.0;
        
        /* Conditional execution based on volatile variable */
        volatile int condition = iter % 2;
        
        if (condition) {
            /* Branch 1: Complex DFP and vector operations */
            
#if HAS_DFP
            /* DFP arithmetic that may expand to multi-operand patterns */
            _Decimal128 d128_result = dfp_operation(
                d128_a, d128_b, d128_c, d128_d);
            
            /* Additional DFP computation */
            d128_result = d128_result * d128_a + d128_b / d128_c;
            
            /* Convert to double for accumulation */
            iter_result += (double)d128_result;
#endif
            
#if HAS_COMPLEX
            /* Complex arithmetic */
            long double complex cl_result = complex_operation(ca, cb, cc, cd);
            
            /* More complex operations */
            cl_result = (ca * cb) / (cc - cd) + csqrt(ca * cb);
            
            iter_result += creal(cl_result) + cimag(cl_result);
#endif
        } else {
            /* Branch 2: Vector reductions and helper calls */
            
#if HAS_VECTOR
            /* Vector reduction with accumulation */
            float vec_sum = vector_reduce_sum(vec_float);
            double vec_dbl_sum = vector_reduce_double(vec_double);
            
            /* Modify vector elements */
            vec_float[0] += 0.1f;
            vec_float[1] -= 0.1f;
            vec_double[0] *= 1.01;
            vec_double[1] /= 1.01;
            
            iter_result += vec_sum + vec_dbl_sum;
#endif
            
            /* Call helper with 10 arguments - creates large operand list */
            double helper_result = helper_10_args(
                dfp1, dfp2, dfp3, dfp4,
                dfp1 * 2.0, dfp2 / 2.0,
                dfp3 + 1.0, dfp4 - 1.0,
                sin(dfp1), cos(dfp2));
            
            iter_result += helper_result;
        }
        
        /* Call helper with 11 arguments - targeting the 11-operand case */
        long double big_helper_result = helper_11_args(
            (long double)dfp1, (long double)dfp2, (long double)dfp3,
            (long double)dfp4, (long double)(dfp1 * 1.1),
            (long double)(dfp2 * 1.2), (long double)(dfp3 * 1.3),
            (long double)(dfp4 * 1.4), (long double)sin(dfp1),
            (long double)cos(dfp2), (long double)exp(dfp3));
        
        iter_result += (double)big_helper_result;
        
        /* Additional mixed operations to increase operand count */
        double mixed_result = 
            dfp1 * dfp2 + 
            dfp3 / dfp4 - 
            sin(dfp1) * cos(dfp2) + 
            exp(dfp3) / log(dfp4 + 1.0);
        
        iter_result += mixed_result;
        
        /* Store result to prevent elimination */
        results[iter] = iter_result;
        result_accumulator += iter_result;
        
        /* Modify base values for next iteration */
        dfp1 += 0.12345;
        dfp2 -= 0.054321;
        dfp3 *= 1.01;
        dfp4 /= 1.01;
        
#if HAS_COMPLEX
        ca += 0.1 + 0.2 * I;
        cb *= 0.9 - 0.1 * I;
#endif
    }
    
    /* Compute final checksum */
    double checksum = 0.0;
    for (int i = 0; i < 3; i++) {
        checksum += results[i];
    }
    
    /* Also use result_accumulator to prevent optimization */
    checksum += result_accumulator;
    
    printf("Checksum: %.15f\n", checksum);
    
    return 0;
}
