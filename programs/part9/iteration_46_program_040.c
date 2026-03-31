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
    /* Fallback DFP types using integer arrays */
    typedef struct { uint64_t lo, hi; } decimal64_fb;
    typedef struct { uint64_t w[2]; } decimal128_fb;
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
    typedef struct { double re, im; } complex_double_fb;
#endif

/* Vector extensions if available */
#ifdef __VECTOR_TYPES_SUPPORTED__
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
    typedef struct { int32_t v[8]; } int32x8_fb;
    typedef struct { double v[4]; } float64x4_fb;
#endif

/* Helper function with exactly 11 arguments to trigger case 11 */
static __attribute__((noinline)) 
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression to prevent optimization */
    volatile long double result = 0.0L;
    result = a1 * a2 + a3 / a4 - a5 * a6 + a7 - a8 / a9 + a10 * a11;
    return result;
}

/* Helper function with exactly 10 arguments to trigger case 10 */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    volatile double sum = 0.0;
    sum = b1 + b2 - b3 * b4 + b5 / b6 - b7 + b8 * b9 - b10;
    return sum;
}

/* DFP helper with mixed operations */
#if HAS_DFP
static __attribute__((noinline))
_Decimal128 dfp_complex_op(
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3,
    _Decimal128 ld1, _Decimal128 ld2, _Decimal128 ld3,
    _Decimal64 d4, _Decimal64 d5, _Decimal64 d6,
    _Decimal128 ld4)
{
    /* Complex DFP expression that may expand to many operands */
    volatile _Decimal128 result;
    result = (ld1 * ld2 + ld3 / ld4) * (_Decimal128)d1 
             - (_Decimal128)d2 * (_Decimal128)d3 
             + (_Decimal128)d4 / (_Decimal128)d5 
             - (_Decimal128)d6;
    return result;
}
#else
static __attribute__((noinline))
decimal128_fb dfp_complex_op_fb(
    decimal64_fb d1, decimal64_fb d2, decimal64_fb d3,
    decimal128_fb ld1, decimal128_fb ld2, decimal128_fb ld3,
    decimal64_fb d4, decimal64_fb d5, decimal64_fb d6,
    decimal128_fb ld4)
{
    /* Simulate DFP operations with integer arithmetic */
    decimal128_fb result;
    /* Simple emulation - actual DFP would be more complex */
    result.w[0] = d1.lo + d2.lo + d3.lo + d4.lo + d5.lo + d6.lo;
    result.w[1] = ld1.w[0] + ld2.w[0] + ld3.w[0] + ld4.w[0];
    return result;
}
#endif

/* Complex number operations */
#if HAS_COMPLEX
static __attribute__((noinline))
long double _Complex complex_high_precision(
    long double _Complex c1, long double _Complex c2,
    long double _Complex c3, long double _Complex c4,
    long double _Complex c5, long double _Complex c6,
    long double _Complex c7, long double _Complex c8,
    long double _Complex c9, long double _Complex c10)
{
    /* Complex expression that may generate many operands */
    volatile long double _Complex result;
    result = (c1 * c2 + c3 / c4) * (c5 - c6) 
             / (c7 + c8 * c9 - c10);
    
    /* Additional complex operation */
    result = result + csqrt(c1 * c2) * cpow(c3, c4);
    
    return result;
}
#else
static __attribute__((noinline))
complex_double_fb complex_high_precision_fb(
    complex_double_fb c1, complex_double_fb c2,
    complex_double_fb c3, complex_double_fb c4,
    complex_double_fb c5, complex_double_fb c6,
    complex_double_fb c7, complex_double_fb c8,
    complex_double_fb c9, complex_double_fb c10)
{
    complex_double_fb result;
    /* Manual complex arithmetic */
    result.re = c1.re + c2.re - c3.re * c4.re + c5.re / c6.re 
                - c7.re + c8.re * c9.re - c10.re;
    result.im = c1.im + c2.im - c3.im * c4.im + c5.im / c6.im 
                - c7.im + c8.im * c9.im - c10.im;
    return result;
}
#endif

/* Vector reduction with accumulation */
#if HAS_VECTORS
static __attribute__((noinline))
double vector_reduction_accumulate(
    float64x4_t v1, float64x4_t v2, float64x4_t v3,
    float64x4_t v4, float64x4_t v5)
{
    volatile double accumulator = 0.0;
    
    /* Horizontal reduction of each vector */
    float64x4_t sum1 = v1 + v2;
    float64x4_t sum2 = v3 + v4;
    float64x4_t sum3 = sum1 + sum2 + v5;
    
    /* Extract and accumulate elements */
    double temp[4];
    memcpy(temp, &sum3, sizeof(sum3));
    
    for (int i = 0; i < 4; i++) {
        accumulator += temp[i];
    }
    
    /* Additional complex reduction */
    accumulator += temp[0] * temp[1] - temp[2] / temp[3];
    
    return accumulator;
}
#else
static __attribute__((noinline))
double vector_reduction_accumulate_fb(
    float64x4_fb v1, float64x4_fb v2, float64x4_fb v3,
    float64x4_fb v4, float64x4_fb v5)
{
    volatile double accumulator = 0.0;
    
    for (int i = 0; i < 4; i++) {
        accumulator += v1.v[i] + v2.v[i] + v3.v[i] 
                      + v4.v[i] + v5.v[i];
    }
    
    return accumulator;
}
#endif

/* Global volatile to prevent optimization */
volatile double global_sink[32];
static int sink_index = 0;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base values deterministically */
    double base_values[20];
    for (int i = 0; i < 20; i++) {
        base_values[i] = (rand() % 1000) / 100.0 + 1.0;
    }
    
    long double checksum = 0.0L;
    
    /* Main computation loop */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = iter % 2;
        
        if (condition) {
            /* Branch 1: Use helper with 11 arguments */
            long double arg1 = base_values[0] + iter;
            long double arg2 = base_values[1] * iter;
            long double arg3 = base_values[2] / (iter + 1);
            long double arg4 = base_values[3] - iter;
            long double arg5 = base_values[4] + iter * 2;
            long double arg6 = base_values[5] / (iter + 2);
            long double arg7 = base_values[6] * iter;
            long double arg8 = base_values[7] - iter * 3;
            long double arg9 = base_values[8] + iter / 2.0;
            long double arg10 = base_values[9] * (iter + 1);
            long double arg11 = base_values[10] / (iter + 3);
            
            long double result1 = helper_11_args(
                arg1, arg2, arg3, arg4, arg5,
                arg6, arg7, arg8, arg9, arg10,
                arg11);
            
            global_sink[sink_index++] = (double)result1;
            checksum += result1;
        } else {
            /* Branch 2: Use helper with 10 arguments */
            double barg1 = base_values[11] + iter;
            double barg2 = base_values[12] * iter;
            double barg3 = base_values[13] / (iter + 1);
            double barg4 = base_values[14] - iter;
            double barg5 = base_values[15] + iter * 2;
            double barg6 = base_values[16] / (iter + 2);
            double barg7 = base_values[17] * iter;
            double barg8 = base_values[18] - iter * 3;
            double barg9 = base_values[19] + iter / 2.0;
            double barg10 = base_values[0] * (iter + 1);
            
            double result2 = helper_10_args(
                barg1, barg2, barg3, barg4, barg5,
                barg6, barg7, barg8, barg9, barg10);
            
            global_sink[sink_index++] = result2;
            checksum += result2;
        }
        
        /* DFP operations (if available) */
        #if HAS_DFP
        {
            _Decimal64 d64_vals[6];
            _Decimal128 d128_vals[4];
            
            for (int i = 0; i < 6; i++) {
                d64_vals[i] = (_Decimal64)(base_values[i] + iter);
            }
            for (int i = 0; i < 4; i++) {
                d128_vals[i] = (_Decimal128)(base_values[i+6] * (iter + 1));
            }
            
            _Decimal128 dfp_result = dfp_complex_op(
                d64_vals[0], d64_vals[1], d64_vals[2],
                d128_vals[0], d128_vals[1], d128_vals[2],
                d64_vals[3], d64_vals[4], d64_vals[5],
                d128_vals[3]);
            
            global_sink[sink_index++] = (double)dfp_result;
            checksum += (long double)dfp_result;
        }
        #else
        {
            decimal64_fb d64_fb[6];
            decimal128_fb d128_fb[4];
            
            for (int i = 0; i < 6; i++) {
                d64_fb[i].lo = (uint64_t)(base_values[i] * 1000);
                d64_fb[i].hi = 0;
            }
            for (int i = 0; i < 4; i++) {
                d128_fb[i].w[0] = (uint64_t)(base_values[i+6] * 1000);
                d128_fb[i].w[1] = 0;
            }
            
            decimal128_fb dfp_result_fb = dfp_complex_op_fb(
                d64_fb[0], d64_fb[1], d64_fb[2],
                d128_fb[0], d128_fb[1], d128_fb[2],
                d64_fb[3], d64_fb[4], d64_fb[5],
                d128_fb[3]);
            
            double result_fb = (double)(dfp_result_fb.w[0] + dfp_result_fb.w[1]);
            global_sink[sink_index++] = result_fb;
            checksum += result_fb;
        }
        #endif
        
        /* Complex number operations */
        #if HAS_COMPLEX
        {
            long double _Complex c_vals[10];
            for (int i = 0; i < 10; i++) {
                c_vals[i] = (base_values[i] + iter) 
                          + (base_values[i+1] * iter) * I;
            }
            
            long double _Complex c_result = complex_high_precision(
                c_vals[0], c_vals[1], c_vals[2], c_vals[3],
                c_vals[4], c_vals[5], c_vals[6], c_vals[7],
                c_vals[8], c_vals[9]);
            
            global_sink[sink_index++] = creal(c_result);
            global_sink[sink_index++] = cimag(c_result);
            checksum += creal(c_result) + cimag(c_result);
        }
        #else
        {
            complex_double_fb c_fb[10];
            for (int i = 0; i < 10; i++) {
                c_fb[i].re = base_values[i] + iter;
                c_fb[i].im = base_values[i+1] * iter;
            }
            
            complex_double_fb c_result_fb = complex_high_precision_fb(
                c_fb[0], c_fb[1], c_fb[2], c_fb[3],
                c_fb[4], c_fb[5], c_fb[6], c_fb[7],
                c_fb[8], c_fb[9]);
            
            global_sink[sink_index++] = c_result_fb.re;
            global_sink[sink_index++] = c_result_fb.im;
            checksum += c_result_fb.re + c_result_fb.im;
        }
        #endif
        
        /* Vector operations */
        #if HAS_VECTORS
        {
            float64x4_t vecs[5];
            for (int v = 0; v < 5; v++) {
                double temp[4];
                for (int i = 0; i < 4; i++) {
                    temp[i] = base_values[v*4 + i] * (iter + v + 1);
                }
                memcpy(&vecs[v], temp, sizeof(temp));
            }
            
            double vec_result = vector_reduction_accumulate(
                vecs[0], vecs[1], vecs[2], vecs[3], vecs[4]);
            
            global_sink[sink_index++] = vec_result;
            checksum += vec_result;
        }
        #else
        {
            float64x4_fb vecs_fb[5];
            for (int v = 0; v < 5; v++) {
                for (int i = 0; i < 4; i++) {
                    vecs_fb[v].v[i] = base_values[v*4 + i] * (iter + v + 1);
                }
            }
            
            double vec_result_fb = vector_reduction_accumulate_fb(
                vecs_fb[0], vecs_fb[1], vecs_fb[2], 
                vecs_fb[3], vecs_fb[4]);
            
            global_sink[sink_index++] = vec_result_fb;
            checksum += vec_result_fb;
        }
        #endif
    }
    
    /* Final checksum computation */
    volatile long double final_checksum = 0.0L;
    for (int i = 0; i < sink_index; i++) {
        final_checksum += global_sink[i];
    }
    
    /* Mix in the computed checksum */
    final_checksum = final_checksum * 0.5L + checksum * 0.5L;
    
    printf("Final checksum: %Lf\n", final_checksum);
    printf("Sink array size used: %d\n", sink_index);
    
    return 0;
}
