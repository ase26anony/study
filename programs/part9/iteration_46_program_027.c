#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Feature detection and fallbacks */
#ifdef __STDC_IEC_559_COMPLEX__
#include <complex.h>
#define HAS_COMPLEX 1
#else
#define HAS_COMPLEX 0
#endif

#ifdef __DECIMAL_BID_FORMAT__
#define HAS_DFP 1
#else
#define HAS_DFP 0
#endif

/* Vector type definitions */
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));

/* Fallback types for DFP */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal128_fallback;

typedef uint64_t decimal64_fallback;
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

/* Helper function with 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4, double b5,
    double b6, double b7, double b8, double b9, double b10)
{
    return (b1 + b2) * (b3 - b4) / (b5 * b6) + 
           (b7 * b8) - (b9 / b10);
}

/* Vector reduction with accumulation */
static float vector_reduce_sum(float32x4_t v)
{
    float sum = 0.0f;
    for (int i = 0; i < 4; i++) {
        sum += v[i];
    }
    return sum;
}

/* Complex number operations */
#if HAS_COMPLEX
static long double complex complex_operation(
    long double complex a,
    long double complex b,
    long double complex c,
    long double complex d)
{
    /* Complex expression that may expand to many real/imag operations */
    return (a * b) / (c - d) + (a + b) * (c * d);
}
#endif

/* DFP operations with fallback */
#if HAS_DFP
static _Decimal128 dfp_operation(
    _Decimal128 a,
    _Decimal128 b,
    _Decimal128 c,
    _Decimal128 d)
{
    /* DFP expression that may require many operands */
    return (a * b) + (c / d) - (a / b) * (c + d);
}
#else
static decimal128_fallback dfp_operation_fallback(
    decimal128_fallback a,
    decimal128_fallback b,
    decimal128_fallback c,
    decimal128_fallback d)
{
    /* Simulate DFP with integer arithmetic */
    decimal128_fallback result;
    result.lo = a.lo + b.lo + c.lo + d.lo;
    result.hi = a.hi - b.hi + c.hi - d.hi;
    return result;
}
#endif

int main(int argc, char *argv[])
{
    /* Use command line seed for deterministic behavior */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Prevent optimization */
    volatile long double volatile_result = 0.0L;
    volatile double volatile_double = 0.0;
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456789e10DL;
    _Decimal64 d64_b = 9.87654321e9DL;
    _Decimal128 d128_a = 1.2345678901234567890123456789e20DL;
    _Decimal128 d128_b = 9.8765432109876543210987654321e19DL;
    _Decimal128 d128_c = 5.5555555555555555555555555555e18DL;
    _Decimal128 d128_d = 2.2222222222222222222222222222e17DL;
#else
    decimal64_fallback d64_a = {0x123456789ABCDEF0ULL};
    decimal64_fallback d64_b = {0xFEDCBA9876543210ULL};
    decimal128_fallback d128_a = {0x123456789ABCDEF0ULL, 0x0FEDCBA987654321ULL};
    decimal128_fallback d128_b = {0xFEDCBA9876543210ULL, 0x1023456789ABCDEFULL};
    decimal128_fallback d128_c = {0x5555555555555555ULL, 0x0555555555555555ULL};
    decimal128_fallback d128_d = {0x2222222222222222ULL, 0x0222222222222222ULL};
#endif
    
#if HAS_COMPLEX
    long double complex ca = 1.0L + 2.0LI;
    long double complex cb = 3.0L - 4.0LI;
    long double complex cc = 5.0L + 6.0LI;
    long double complex cd = 7.0L - 8.0LI;
#endif
    
    /* Initialize vectors */
    float32x4_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    float32x4_t vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    
    int32x4_t ivec1 = {1, 2, 3, 4};
    int32x4_t ivec2 = {5, 6, 7, 8};
    
    /* Loop with conditional execution */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Use volatile condition to prevent constant folding */
        volatile int condition = iteration % 2;
        
        if (condition) {
            /* Branch 1: Complex DFP and vector operations */
            
            /* DFP arithmetic - may expand to many operands */
#if HAS_DFP
            _Decimal128 dfp_result = dfp_operation(d128_a, d128_b, d128_c, d128_d);
            volatile_double = (double)dfp_result;
#else
            decimal128_fallback dfp_result = dfp_operation_fallback(d128_a, d128_b, d128_c, d128_d);
            volatile_double = (double)dfp_result.lo;
#endif
            
            /* Complex arithmetic */
#if HAS_COMPLEX
            long double complex cl_result = complex_operation(ca, cb, cc, cd);
            volatile_result += creall(cl_result) + cimagl(cl_result);
#endif
            
            /* Vector reduction with accumulation */
            float32x4_t vec_sum = vec1 + vec2 * vec3;
            float vec_reduced = vector_reduce_sum(vec_sum);
            volatile_result += vec_reduced;
            
            /* Call helper with 11 arguments */
            long double helper1_result = helper_11_args(
                (long double)iteration + 1.0L,
                (long double)iteration + 2.0L,
                (long double)iteration + 3.0L,
                (long double)iteration + 4.0L,
                (long double)iteration + 5.0L,
                (long double)iteration + 6.0L,
                (long double)iteration + 7.0L,
                (long double)iteration + 8.0L,
                (long double)iteration + 9.0L,
                (long double)iteration + 10.0L,
                (long double)iteration + 11.0L
            );
            volatile_result += helper1_result;
            
        } else {
            /* Branch 2: Different mix of operations */
            
            /* More complex DFP expressions */
#if HAS_DFP
            _Decimal128 dfp_expr = (d128_a * d128_b) + 
                                  (d128_c / d128_d) - 
                                  (d128_a / d128_b) * 
                                  (d128_c + d128_d);
            volatile_double += (double)dfp_expr;
#endif
            
            /* Vector operations */
            int32x4_t ivec_result = ivec1 * ivec2 + ivec1 - ivec2;
            for (int i = 0; i < 4; i++) {
                volatile_result += ivec_result[i];
            }
            
            /* Call helper with 10 arguments */
            double helper2_result = helper_10_args(
                (double)iteration * 1.1,
                (double)iteration * 2.2,
                (double)iteration * 3.3,
                (double)iteration * 4.4,
                (double)iteration * 5.5,
                (double)iteration * 6.6,
                (double)iteration * 7.7,
                (double)iteration * 8.8,
                (double)iteration * 9.9,
                (double)iteration * 10.1
            );
            volatile_result += helper2_result;
            
            /* Additional complex expression */
#if HAS_COMPLEX
            long double complex cl_expr = (ca * cb) / (cc - cd) + 
                                         (ca + cb) * (cc * cd);
            volatile_result += creall(cl_expr) * cimagl(cl_expr);
#endif
        }
        
        /* Mix operations across iterations */
        vec1 = vec1 + vec2;
        vec2 = vec2 * 1.5f;
        
#if HAS_COMPLEX
        ca = ca * 1.1L;
        cb = cb / 1.1L;
#endif
        
#if HAS_DFP
        d128_a = d128_a * 1.01DL;
        d128_b = d128_b / 1.01DL;
#endif
    }
    
    /* Compute checksum */
    long double checksum = volatile_result + volatile_double;
    
    /* Print result (prevents dead code elimination) */
    printf("Result checksum: %Lf\n", checksum);
    
    return 0;
}
