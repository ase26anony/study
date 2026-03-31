#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimal/decimal.h>
#else
    #define HAS_DFP 0
    /* Fallback DFP types using arrays */
    typedef struct { uint64_t lo, hi; } decimal64_fb;
    typedef struct { uint64_t w[2]; } decimal128_fb;
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
    typedef struct { double real, imag; } complex_double;
#endif

/* Vector extensions - check GCC vector support */
#ifdef __VECTOR_TYPES_SUPPORTED__
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
    typedef struct { int32_t v[8]; } int32x8_t;
    typedef struct { double v[4]; } float64x4_t;
#endif

/* Helper function with 11 arguments - marked noinline */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression to prevent optimization */
    return (a1 * a2) + (a3 / a4) - (a5 * a6) + 
           (a7 - a8) * (a9 + a10) / a11;
}

/* Helper function with 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    return ((b1 + b2) * (b3 - b4)) / (b5 * b6) + 
           (b7 * b8) - (b9 / b10);
}

/* Global volatile to prevent dead code elimination */
volatile double global_accumulator = 0.0;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789dd;
    _Decimal64 d64_b = 9.87654321dd;
    _Decimal64 d64_c = 5.55555555dd;
    _Decimal64 d64_d = 3.33333333dd;
    
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 9.8765432109876543dl;
    _Decimal128 d128_c = 5.5555555555555555dl;
    _Decimal128 d128_d = 2.2222222222222222dl;
#else
    /* Fallback DFP simulation */
    decimal64_fb d64_a = {123456789ULL, 0};
    decimal64_fb d64_b = {987654321ULL, 0};
    decimal64_fb d64_c = {555555555ULL, 0};
    decimal64_fb d64_d = {333333333ULL, 0};
    
    decimal128_fb d128_a = {{12345678901234567ULL, 0}};
    decimal128_fb d128_b = {{98765432109876543ULL, 0}};
    decimal128_fb d128_c = {{55555555555555555ULL, 0}};
    decimal128_fb d128_d = {{22222222222222222ULL, 0}};
#endif

#if HAS_COMPLEX
    long double _Complex cl_a = 1.5L + 2.5L * I;
    long double _Complex cl_b = 3.5L + 4.5L * I;
    long double _Complex cl_c = 5.5L + 6.5L * I;
    long double _Complex cl_d = 7.5L + 8.5L * I;
#else
    complex_double cl_a = {1.5, 2.5};
    complex_double cl_b = {3.5, 4.5};
    complex_double cl_c = {5.5, 6.5};
    complex_double cl_d = {7.5, 8.5};
#endif

#if HAS_VECTORS
    int32x8_t vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
    float64x4_t vec_dbl = {1.1, 2.2, 3.3, 4.4};
#else
    int32x8_t vec_int = {{1, 2, 3, 4, 5, 6, 7, 8}};
    float64x4_t vec_dbl = {{1.1, 2.2, 3.3, 4.4}};
#endif

    /* Results array to prevent optimization */
    double results[5] = {0};
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        double iter_result = 0.0;
        
        /* Conditional execution based on volatile-like condition */
        volatile int condition = seed + iter;
        
        if (condition % 2 == 0) {
            /* DFP arithmetic - may expand to multi-operand patterns */
#if HAS_DFP
            /* Use DFP builtins that may expand to complex RTL */
            _Decimal128 d128_result;
            d128_result = __builtin_dadd(d128_a, d128_b);
            d128_result = __builtin_dmul(d128_result, d128_c);
            d128_result = __builtin_ddiv(d128_result, d128_d);
            
            /* Convert to double for accumulation */
            iter_result += (double)d128_result;
#else
            /* Fallback DFP simulation */
            decimal128_fb d128_result = d128_a;
            /* Simulate addition: a + b */
            d128_result.w[0] += d128_b.w[0];
            d128_result.w[1] += d128_b.w[1];
            /* Simulate multiplication: (a+b) * c */
            d128_result.w[0] *= d128_c.w[0];
            d128_result.w[1] *= d128_c.w[1];
            /* Simulate division: ((a+b)*c) / d */
            if (d128_d.w[0] != 0) d128_result.w[0] /= d128_d.w[0];
            if (d128_d.w[1] != 0) d128_result.w[1] /= d128_d.w[1];
            
            iter_result += (double)d128_result.w[0] + (double)d128_result.w[1];
#endif
        } else {
            /* Complex number operations */
#if HAS_COMPLEX
            long double _Complex cl_result;
            /* Complex expression that may expand to many operands */
            cl_result = (cl_a * cl_b) / (cl_c - cl_d);
            
            /* Use complex library functions */
            cl_result = csqrt(cl_result);
            cl_result = cpow(cl_result, 2.0L);
            
            iter_result += creal(cl_result) + cimag(cl_result);
#else
            complex_double cl_result;
            /* Manual complex arithmetic */
            double real = (cl_a.real * cl_b.real - cl_a.imag * cl_b.imag) /
                         (cl_c.real - cl_d.real);
            double imag = (cl_a.real * cl_b.imag + cl_a.imag * cl_b.real) /
                         (cl_c.imag - cl_d.imag);
            cl_result.real = real;
            cl_result.imag = imag;
            
            /* Manual sqrt approximation */
            double mag = sqrt(real*real + imag*imag);
            cl_result.real = sqrt((mag + real)/2.0);
            cl_result.imag = sqrt((mag - real)/2.0) * (imag < 0 ? -1 : 1);
            
            /* Square it */
            double new_real = cl_result.real*cl_result.real - cl_result.imag*cl_result.imag;
            double new_imag = 2*cl_result.real*cl_result.imag;
            
            iter_result += new_real + new_imag;
#endif
        }
        
        /* Vector reduction with accumulation */
#if HAS_VECTORS
        /* Horizontal sum of vector elements */
        int32_t vec_sum = 0;
        for (int i = 0; i < 8; i++) {
            vec_sum += vec_int[i];
        }
        
        double dbl_sum = 0.0;
        for (int i = 0; i < 4; i++) {
            dbl_sum += vec_dbl[i];
        }
        
        iter_result += (double)vec_sum + dbl_sum;
#else
        int32_t vec_sum = 0;
        for (int i = 0; i < 8; i++) {
            vec_sum += vec_int.v[i];
        }
        
        double dbl_sum = 0.0;
        for (int i = 0; i < 4; i++) {
            dbl_sum += vec_dbl.v[i];
        }
        
        iter_result += (double)vec_sum + dbl_sum;
#endif
        
        /* Call helper functions with many arguments */
        double helper_result;
        if (iter % 2 == 0) {
            /* 11-argument function */
            helper_result = helper_11_args(
                iter_result, 2.0, 3.0, 4.0, 5.0,
                6.0, 7.0, 8.0, 9.0, 10.0, 11.0
            );
        } else {
            /* 10-argument function */
            helper_result = helper_10_args(
                iter_result, 1.5, 2.5, 3.5, 4.5,
                5.5, 6.5, 7.5, 8.5, 9.5
            );
        }
        
        /* Store result to prevent optimization */
        results[iter] = helper_result;
        global_accumulator += helper_result;
        
        /* Modify inputs for next iteration to prevent constant folding */
#if HAS_DFP
        d64_a += 0.1dd;
        d128_b += 0.01dl;
#else
        d64_a.lo += 1000000;
        d128_b.w[0] += 10000000;
#endif
        
#if HAS_COMPLEX
        cl_a += 0.1L + 0.1L * I;
#else
        cl_a.real += 0.1;
        cl_a.imag += 0.1;
#endif
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < 3; i++) {
        checksum += results[i];
    }
    checksum += global_accumulator;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
