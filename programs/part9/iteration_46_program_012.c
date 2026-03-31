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

#ifdef __VECTOR_TYPES_SUPPORTED__
    #define HAS_VECTOR 1
#else
    #define HAS_VECTOR 0
#endif

/* DFP fallback structures */
#if !HAS_DFP
typedef struct {
    unsigned long long hi;
    unsigned long long lo;
} decimal128_fb;

typedef struct {
    unsigned long long value;
} decimal64_fb;
#endif

/* Vector types */
#if HAS_VECTOR
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef double float64x4_t __attribute__((vector_size(32)));
#else
typedef struct {
    int32_t data[8];
} int32x8_t;
typedef struct {
    double data[4];
} float64x4_t;
#endif

/* Global volatile to prevent optimization */
volatile double global_sink = 0.0;

/* Helper function with 11 arguments - designed to trigger case 11 */
static double __attribute__((noinline))
helper_11_args(double a1, double a2, double a3, double a4, double a5,
               double a6, double a7, double a8, double a9, double a10,
               double a11) {
    /* Complex expression that might expand to many operands */
    return (((a1 * a2) + (a3 / a4)) - 
            ((a5 + a6) * (a7 - a8)) + 
            (a9 * a10 * a11)) / 1000.0;
}

/* Helper function with 10 arguments - designed to trigger case 10 */
static double __attribute__((noinline))
helper_10_args(double a1, double a2, double a3, double a4, double a5,
               double a6, double a7, double a8, double a9, double a10) {
    return ((a1 + a2 + a3 + a4 + a5) * 
            (a6 + a7 + a8 + a9 + a10)) / 500.0;
}

#if HAS_DFP
/* DFP operations that may expand to many operands */
static _Decimal128 __attribute__((noinline))
dfp_complex_op(_Decimal128 a, _Decimal128 b, _Decimal128 c, 
               _Decimal128 d, _Decimal128 e) {
    /* Complex DFP expression that might need many temporary operands */
    return ((a * b) + (c / d) - (e * 2.0DD)) * 3.1415926535DD;
}

static _Decimal64 __attribute__((noinline))
dfp64_mixed_op(_Decimal64 a, _Decimal64 b, _Decimal64 c,
               _Decimal64 d, _Decimal64 e, _Decimal64 f) {
    return (((a + b) * (c - d)) / (e * f)) * 1.5DD;
}
#endif

#if HAS_COMPLEX
/* Complex number operations */
static long double complex __attribute__((noinline))
complex_power(long double complex a, long double complex b,
              long double complex c, long double complex d) {
    /* Complex power operations that may expand */
    long double complex t1 = a * b;
    long double complex t2 = c / d;
    long double complex t3 = t1 + t2;
    return cpow(t3, 2.0L + I * 0.5L);
}
#endif

/* Vector reduction with accumulation */
static double __attribute__((noinline))
vector_reduce_accumulate(float64x4_t v1, float64x4_t v2, 
                         float64x4_t v3, float64x4_t v4) {
#if HAS_VECTOR
    /* Horizontal reduction that might expand to many operations */
    float64x4_t sum1 = v1 + v2;
    float64x4_t sum2 = v3 + v4;
    float64x4_t total = sum1 + sum2;
    
    /* Manual horizontal sum */
    double result = 0.0;
    for (int i = 0; i < 4; i++) {
        result += total[i];
    }
    return result;
#else
    double result = 0.0;
    for (int i = 0; i < 4; i++) {
        result += v1.data[i] + v2.data[i] + v3.data[i] + v4.data[i];
    }
    return result;
#endif
}

/* Main computation loop */
int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Initialize base variables */
    double base_values[20];
    for (int i = 0; i < 20; i++) {
        base_values[i] = (double)(rand() % 1000) / 100.0 + 1.0;
    }
    
#if HAS_DFP
    /* Initialize DFP values */
    _Decimal128 d128_a = 1234567890123456.0DD;
    _Decimal128 d128_b = 9876543210987654.0DD;
    _Decimal128 d128_c = 5555555555555555.0DD;
    _Decimal128 d128_d = 2222222222222222.0DD;
    _Decimal128 d128_e = 3333333333333333.0DD;
    
    _Decimal64 d64_a = 1234567.0DD;
    _Decimal64 d64_b = 7654321.0DD;
    _Decimal64 d64_c = 1111111.0DD;
    _Decimal64 d64_d = 9999999.0DD;
    _Decimal64 d64_e = 4444444.0DD;
    _Decimal64 d64_f = 8888888.0DD;
#endif
    
#if HAS_COMPLEX
    /* Initialize complex values */
    long double complex ca = 1.5L + 2.5L * I;
    long double complex cb = 3.0L - 1.0L * I;
    long double complex cc = 0.5L + 4.0L * I;
    long double complex cd = 2.0L + 3.0L * I;
#endif
    
    /* Initialize vectors */
#if HAS_VECTOR
    float64x4_t vec1 = {base_values[0], base_values[1], 
                        base_values[2], base_values[3]};
    float64x4_t vec2 = {base_values[4], base_values[5], 
                        base_values[6], base_values[7]};
    float64x4_t vec3 = {base_values[8], base_values[9], 
                        base_values[10], base_values[11]};
    float64x4_t vec4 = {base_values[12], base_values[13], 
                        base_values[14], base_values[15]};
#else
    float64x4_t vec1 = {{base_values[0], base_values[1], 
                         base_values[2], base_values[3]}};
    float64x4_t vec2 = {{base_values[4], base_values[5], 
                         base_values[6], base_values[7]}};
    float64x4_t vec3 = {{base_values[8], base_values[9], 
                         base_values[10], base_values[11]}};
    float64x4_t vec4 = {{base_values[12], base_values[13], 
                         base_values[14], base_values[15]}};
#endif
    
    /* Storage for results to prevent dead code elimination */
    double results[5] = {0};
    
    /* Main computation loop - 3 iterations */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = iter % 2; /* Prevent constant folding */
        double result = 0.0;
        
        if (condition) {
            /* Branch 1: Complex DFP operations */
#if HAS_DFP
            _Decimal128 d128_result = ((d128_a * d128_b) + 
                                      (d128_c / d128_d) - 
                                      (d128_e * 2.0DD)) * 
                                      (d128_a + d128_b - d128_c);
            
            _Decimal64 d64_result = (((d64_a + d64_b) * 
                                     (d64_c - d64_d)) / 
                                     (d64_e * d64_f)) * 
                                     (d64_a / d64_b + d64_c);
            
            /* Convert DFP to double for helper functions */
            result += (double)d128_result / 1e30;
            result += (double)d64_result / 1e14;
#endif
        } else {
            /* Branch 2: Complex number operations */
#if HAS_COMPLEX
            long double complex cl_result = (ca * cb) / (cc - cd);
            long double complex cl_pow = cpow(cl_result, 2.0L + I);
            
            result += creal(cl_pow) + cimag(cl_pow);
#endif
        }
        
        /* Always execute vector reduction */
        double vec_result = vector_reduce_accumulate(vec1, vec2, vec3, vec4);
        result += vec_result;
        
        /* Call helper functions with many arguments */
        double helper_result;
        if (condition) {
            /* 11 arguments - should trigger case 11 */
            helper_result = helper_11_args(
                base_values[0] + iter, base_values[1] * iter,
                base_values[2] - iter, base_values[3] / (iter + 1),
                base_values[4] + result, base_values[5] - result,
                base_values[6] * 2.0, base_values[7] / 3.0,
                base_values[8] + vec_result, base_values[9] - vec_result,
                result * 0.5
            );
        } else {
            /* 10 arguments - should trigger case 10 */
            helper_result = helper_10_args(
                base_values[10], base_values[11],
                base_values[12], base_values[13],
                base_values[14], base_values[15],
                base_values[16], base_values[17],
                base_values[18], base_values[19]
            );
        }
        
        result += helper_result;
        
        /* Store result to prevent optimization */
        results[iter] = result;
        global_sink += result;
        
        /* Modify some values for next iteration */
        for (int i = 0; i < 10; i++) {
            base_values[i] += 0.1;
        }
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < 3; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %.15f\n", checksum);
    printf("Global sink: %.15f\n", (double)global_sink);
    
    return 0;
}
