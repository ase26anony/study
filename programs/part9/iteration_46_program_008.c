#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <decimalfp.h>
#else
    #define HAS_DFP 0
#endif

#ifdef __STDC_IEC_559_COMPLEX__
    #include <complex.h>
    #define HAS_COMPLEX 1
#else
    #define HAS_COMPLEX 0
#endif

/* Vector type definitions */
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));

/* Helper function with 11 arguments - marked noinline to prevent optimization */
static __attribute__((noinline)) long double 
multi_arg_func_11(long double a1, long double a2, long double a3,
                  long double a4, long double a5, long double a6,
                  long double a7, long double a8, long double a9,
                  long double a10, long double a11) {
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + (a7 / a8) - 
            (a9 * a10) + (a11 * 2.0L)) / 7.0L;
}

/* Another helper with 10 arguments */
static __attribute__((noinline)) double
multi_arg_func_10(double a1, double a2, double a3, double a4,
                  double a5, double a6, double a7, double a8,
                  double a9, double a10) {
    return a1 + a2 - a3 * a4 + a5 / a6 - a7 * a8 + a9 / a10;
}

/* DFP fallback using integer arithmetic */
typedef union {
    uint64_t bits[2];
    struct {
        uint64_t lo;
        uint64_t hi;
    };
} dfp128_fallback_t;

static dfp128_fallback_t dfp128_add(dfp128_fallback_t a, dfp128_fallback_t b) {
    dfp128_fallback_t result;
    result.lo = a.lo + b.lo;
    result.hi = a.hi + b.hi + (result.lo < a.lo);
    return result;
}

static dfp128_fallback_t dfp128_mul(dfp128_fallback_t a, dfp128_fallback_t b) {
    /* Simplified multiplication for demonstration */
    dfp128_fallback_t result;
    uint64_t a0 = a.lo & 0xFFFFFFFF;
    uint64_t a1 = a.lo >> 32;
    uint64_t a2 = a.hi & 0xFFFFFFFF;
    uint64_t a3 = a.hi >> 32;
    
    uint64_t b0 = b.lo & 0xFFFFFFFF;
    uint64_t b1 = b.lo >> 32;
    uint64_t b2 = b.hi & 0xFFFFFFFF;
    uint64_t b3 = b.hi >> 32;
    
    /* Cross multiplication terms */
    uint64_t t0 = a0 * b0;
    uint64_t t1 = a0 * b1 + a1 * b0;
    uint64_t t2 = a0 * b2 + a1 * b1 + a2 * b0;
    uint64_t t3 = a0 * b3 + a1 * b2 + a2 * b1 + a3 * b0;
    
    result.lo = t0 + (t1 << 32);
    result.hi = (t1 >> 32) + t2 + (t3 << 32);
    return result;
}

/* Vector reduction with accumulation */
static float vector_reduce_sum(float32x4_t v) {
    float32x4_t temp = v;
    /* Horizontal addition - may expand to multiple operations */
    temp = temp + __builtin_shufflevector(temp, temp, 2, 3, 0, 1);
    temp = temp + __builtin_shufflevector(temp, temp, 1, 0, 3, 2);
    return temp[0];
}

static double vector_reduce_sum_double(float64x2_t v) {
    return v[0] + v[1];
}

/* Main computation function */
static volatile double global_accumulator = 0.0;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize various types of variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.234567dd;
    _Decimal64 d64_b = 9.876543dd;
    _Decimal64 d64_c = 5.432198dd;
    _Decimal64 d64_d = 3.141592dd;
    
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 9.8765432109876543dl;
    _Decimal128 d128_c = 5.4321987654321987dl;
    _Decimal128 d128_d = 3.1415926535897932dl;
#else
    /* Fallback to integer representation */
    dfp128_fallback_t d128_a = {{12345678901234567ULL, 0}};
    dfp128_fallback_t d128_b = {{98765432109876543ULL, 0}};
    dfp128_fallback_t d128_c = {{54321987654321987ULL, 0}};
    dfp128_fallback_t d128_d = {{31415926535897932ULL, 0}};
#endif

#if HAS_COMPLEX
    long double _Complex cl_a = 1.5L + 2.5L * I;
    long double _Complex cl_b = 3.5L + 4.5L * I;
    long double _Complex cl_c = 5.5L + 6.5L * I;
    long double _Complex cl_d = 7.5L + 8.5L * I;
#else
    long double cl_a_real = 1.5L, cl_a_imag = 2.5L;
    long double cl_b_real = 3.5L, cl_b_imag = 4.5L;
    long double cl_c_real = 5.5L, cl_c_imag = 6.5L;
    long double cl_d_real = 7.5L, cl_d_imag = 8.5L;
#endif

    /* Initialize vectors */
    float32x4_t vec_f32 = {1.0f, 2.0f, 3.0f, 4.0f};
    float64x2_t vec_f64 = {5.0, 6.0};
    int32x4_t vec_i32 = {7, 8, 9, 10};
    
    /* Storage for results to prevent optimization */
    volatile double results[10];
    int result_idx = 0;
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        double iter_result = 0.0;
        
        /* Complex DFP-like computation */
#if HAS_DFP
        /* This complex DFP expression may expand to many operands */
        _Decimal128 d128_temp;
        if (iter % 2 == 0) {
            /* Complex expression with many operands */
            d128_temp = (d128_a * d128_b + d128_c / d128_d) * 
                       (d128_b - d128_c + d128_d / d128_a) +
                       (d128_c * d128_d - d128_a / d128_b);
        } else {
            d128_temp = (d128_d * d128_a + d128_b / d128_c) *
                       (d128_a - d128_d + d128_c / d128_b) +
                       (d128_b * d128_c - d128_d / d128_a);
        }
        iter_result += (double)d128_temp;
#else
        /* Fallback DFP computation */
        dfp128_fallback_t d128_temp;
        if (iter % 2 == 0) {
            d128_temp = dfp128_add(
                dfp128_mul(d128_a, d128_b),
                dfp128_add(
                    dfp128_mul(d128_c, d128_d),
                    dfp128_add(d128_a, d128_b)
                )
            );
        } else {
            d128_temp = dfp128_add(
                dfp128_mul(d128_b, d128_c),
                dfp128_add(
                    dfp128_mul(d128_d, d128_a),
                    dfp128_add(d128_c, d128_d)
                )
            );
        }
        iter_result += (double)d128_temp.lo + (double)d128_temp.hi;
#endif

        /* Complex number operations */
#if HAS_COMPLEX
        long double _Complex cl_temp;
        if (iter > 0) {
            /* Complex expression that may expand to many real/imag operations */
            cl_temp = (cl_a * cl_b) / (cl_c - cl_d) +
                     (cl_b * cl_c) / (cl_d - cl_a) +
                     (cl_c * cl_d) / (cl_a - cl_b);
            
            /* Call to complex function - may expand further */
            cl_temp = cl_temp * cl_temp + cl_temp;
        } else {
            cl_temp = cl_a + cl_b + cl_c + cl_d;
        }
        iter_result += creall(cl_temp) + cimagl(cl_temp);
#else
        /* Manual complex arithmetic */
        long double real_temp, imag_temp;
        if (iter > 0) {
            /* (a*b)/(c-d) */
            long double real1 = cl_a_real * cl_b_real - cl_a_imag * cl_b_imag;
            long double imag1 = cl_a_real * cl_b_imag + cl_a_imag * cl_b_real;
            long double denom_real = cl_c_real - cl_d_real;
            long double denom_imag = cl_c_imag - cl_d_imag;
            long double denom_sq = denom_real * denom_real + denom_imag * denom_imag;
            
            real_temp = (real1 * denom_real + imag1 * denom_imag) / denom_sq;
            imag_temp = (imag1 * denom_real - real1 * denom_imag) / denom_sq;
        } else {
            real_temp = cl_a_real + cl_b_real + cl_c_real + cl_d_real;
            imag_temp = cl_a_imag + cl_b_imag + cl_c_imag + cl_d_imag;
        }
        iter_result += real_temp + imag_temp;
#endif

        /* Vector reduction with accumulation */
        float vec_sum_f32 = vector_reduce_sum(vec_f32);
        double vec_sum_f64 = vector_reduce_sum_double(vec_f64);
        
        /* Integer vector reduction */
        int32x4_t vec_temp = vec_i32;
        vec_temp = vec_temp + __builtin_shufflevector(vec_temp, vec_temp, 2, 3, 0, 1);
        vec_temp = vec_temp + __builtin_shufflevector(vec_temp, vec_temp, 1, 0, 3, 2);
        int vec_sum_i32 = vec_temp[0];
        
        iter_result += vec_sum_f32 + vec_sum_f64 + vec_sum_i32;
        
        /* Call helper functions with many arguments */
        double helper_result;
        if (iter == 2) {
            /* Call 11-argument function with complex expression results */
            helper_result = multi_arg_func_11(
                iter_result, vec_sum_f32, vec_sum_f64,
                (double)vec_sum_i32, iter_result * 0.5,
                vec_sum_f32 * 2.0, vec_sum_f64 / 3.0,
                (double)vec_sum_i32 + 1.0, iter_result - 1.0,
                vec_sum_f32 + vec_sum_f64, vec_sum_f64 - vec_sum_f32
            );
        } else {
            /* Call 10-argument function */
            helper_result = multi_arg_func_10(
                iter_result, vec_sum_f32, vec_sum_f64,
                (double)vec_sum_i32, iter_result * 0.5,
                vec_sum_f32 * 2.0, vec_sum_f64 / 3.0,
                (double)vec_sum_i32 + 1.0, iter_result - 1.0,
                vec_sum_f32 + vec_sum_f64
            );
        }
        
        iter_result += helper_result;
        
        /* Store result to prevent optimization */
        results[result_idx++] = iter_result;
        global_accumulator += iter_result;
        
        /* Modify vectors for next iteration */
        vec_f32[0] += 0.5f;
        vec_f32[1] += 0.25f;
        vec_f32[2] += 0.125f;
        vec_f32[3] += 0.0625f;
        
        vec_f64[0] *= 1.1;
        vec_f64[1] *= 0.9;
        
        vec_i32[0] += 1;
        vec_i32[1] += 2;
        vec_i32[2] += 3;
        vec_i32[3] += 4;
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Global accumulator: %f\n", (double)global_accumulator);
    
    return 0;
}
