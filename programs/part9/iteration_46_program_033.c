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

/* Vector types if supported */
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__) || defined(__aarch64__))
    #define HAS_VECTORS 1
    typedef int32_t v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
#else
    #define HAS_VECTORS 0
#endif

/* DFP fallback using unions for 64/128-bit decimal simulation */
#if !HAS_DFP
typedef union {
    uint64_t i;
    double f;
} decimal64_sim;

typedef union {
    struct {
        uint64_t lo;
        uint64_t hi;
    } parts;
    long double ld;
} decimal128_sim;

#define DECIMAL64(x) ((decimal64_sim){.f = (x)})
#define DECIMAL128(x) ((decimal128_sim){.ld = (x)})
#define decimal64_add(a, b) DECIMAL64(a.f + b.f)
#define decimal64_mul(a, b) DECIMAL64(a.f * b.f)
#define decimal128_add(a, b) DECIMAL128(a.ld + b.ld)
#define decimal128_mul(a, b) DECIMAL128(a.ld * b.ld)
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double complex helper_11_args(
    #if HAS_DFP
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3,
    _Decimal128 d4, _Decimal128 d5,
    #else
    decimal64_sim d1, decimal64_sim d2, decimal64_sim d3,
    decimal128_sim d4, decimal128_sim d5,
    #endif
    long double complex c1, long double complex c2,
    long double complex c3, long double complex c4,
    #if HAS_VECTORS
    v8si v1, v4df v2
    #else
    int64_t v1, double v2
    #endif
) {
    /* Combine all arguments into a complex result */
    long double real_part = 0.0;
    long double imag_part = 0.0;
    
    #if HAS_DFP
    real_part += (long double)d1 + (long double)d2 + (long double)d3;
    real_part += (long double)d4 + (long double)d5;
    #else
    real_part += d1.f + d2.f + d3.f;
    real_part += d4.ld + d5.ld;
    #endif
    
    real_part += creal(c1) + creal(c2) + creal(c3) + creal(c4);
    imag_part += cimag(c1) + cimag(c2) + cimag(c3) + cimag(c4);
    
    #if HAS_VECTORS
    /* Sum vector elements */
    for (int i = 0; i < 8; i++) real_part += v1[i];
    for (int i = 0; i < 4; i++) imag_part += v2[i];
    #else
    real_part += v1;
    imag_part += v2;
    #endif
    
    return real_part + imag_part * I;
}

/* Another helper with 10 arguments for different case */
static __attribute__((noinline))
#if HAS_DFP
_Decimal128 helper_10_args_dfp(
    _Decimal64 a1, _Decimal64 a2, _Decimal64 a3, _Decimal64 a4,
    _Decimal128 b1, _Decimal128 b2, _Decimal128 b3, _Decimal128 b4,
    _Decimal128 b5, _Decimal128 b6
) {
    /* Complex DFP expression that may expand to many operands */
    return ((a1 * a2) + (a3 * a4)) / (b1 + b2) * (b3 - b4) + (b5 * b6);
}
#else
decimal128_sim helper_10_args_dfp(
    decimal64_sim a1, decimal64_sim a2, decimal64_sim a3, decimal64_sim a4,
    decimal128_sim b1, decimal128_sim b2, decimal128_sim b3, decimal128_sim b4,
    decimal128_sim b5, decimal128_sim b6
) {
    /* Simulated DFP operation */
    decimal128_sim result;
    result.ld = ((a1.f * a2.f) + (a3.f * a4.f)) / (b1.ld + b2.ld) 
                * (b3.ld - b4.ld) + (b5.ld * b6.ld);
    return result;
}
#endif

/* Vector reduction with accumulation */
#if HAS_VECTORS
static __attribute__((noinline))
double vector_reduce_accumulate(v4df vec, double accumulator) {
    /* Horizontal reduction that may create many temporary operands */
    v4df temp = vec + __builtin_shufflevector(vec, vec, 2, 3, 0, 1);
    temp = temp + __builtin_shufflevector(temp, temp, 1, 0, 3, 2);
    return accumulator + temp[0] + temp[1];
}
#else
static double vector_reduce_accumulate(double vec[4], double accumulator) {
    return accumulator + vec[0] + vec[1] + vec[2] + vec[3];
}
#endif

int main(int argc, char *argv[]) {
    /* Use command line seed for deterministic behavior */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Prevent optimization */
    volatile long double complex global_accumulator = 0.0 + 0.0 * I;
    volatile double checksum = 0.0;
    
    /* Initialize DFP values */
    #if HAS_DFP
    _Decimal64 d64_vals[4] = {1.0dd, 2.0dd, 3.0dd, 4.0dd};
    _Decimal128 d128_vals[6] = {
        1.0dl, 2.0dl, 3.0dl, 4.0dl, 5.0dl, 6.0dl
    };
    #else
    decimal64_sim d64_vals[4] = {
        DECIMAL64(1.0), DECIMAL64(2.0), 
        DECIMAL64(3.0), DECIMAL64(4.0)
    };
    decimal128_sim d128_vals[6] = {
        DECIMAL128(1.0), DECIMAL128(2.0), DECIMAL128(3.0),
        DECIMAL128(4.0), DECIMAL128(5.0), DECIMAL128(6.0)
    };
    #endif
    
    /* Initialize complex values */
    long double complex c_vals[4];
    for (int i = 0; i < 4; i++) {
        c_vals[i] = (i + 1) + (i + 2) * I;
    }
    
    /* Initialize vectors */
    #if HAS_VECTORS
    v8si int_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4df double_vec = {1.0, 2.0, 3.0, 4.0};
    #else
    int64_t int_vec = 36; /* Sum of 1..8 */
    double double_arr[4] = {1.0, 2.0, 3.0, 4.0};
    #endif
    
    /* Main computation loop - 3 iterations */
    for (int iter = 0; iter < 3; iter++) {
        /* Conditional execution based on volatile variable */
        volatile int condition = seed + iter;
        
        if (condition % 2 == 0) {
            /* Complex DFP arithmetic that may expand to many operands */
            #if HAS_DFP
            _Decimal128 dfp_result = 
                (d128_vals[0] * d128_vals[1] + d128_vals[2] / d128_vals[3]) *
                (d128_vals[4] - d128_vals[5]) / (d64_vals[0] + d64_vals[1]);
            #else
            decimal128_sim dfp_result = decimal128_mul(
                decimal128_add(
                    decimal128_mul(d128_vals[0], d128_vals[1]),
                    DECIMAL128(d128_vals[2].ld / d128_vals[3].ld)
                ),
                DECIMAL128(
                    (d128_vals[4].ld - d128_vals[5].ld) / 
                    (d64_vals[0].f + d64_vals[1].f)
                )
            );
            #endif
            
            /* Complex number operations with high precision */
            long double complex c_result = 
                (c_vals[0] * c_vals[1]) / (c_vals[2] - c_vals[3]);
            
            #if HAS_COMPLEX && defined(csqrt)
            /* Additional complex function that may expand */
            c_result = c_result + csqrt(c_vals[0] * c_vals[1]);
            #endif
            
            /* Vector reduction */
            #if HAS_VECTORS
            double vec_result = vector_reduce_accumulate(double_vec, 0.0);
            #else
            double vec_result = vector_reduce_accumulate(double_arr, 0.0);
            #endif
            
            /* Call helper with 11 arguments - potentially triggering 11-operand case */
            long double complex helper_result = helper_11_args(
                #if HAS_DFP
                d64_vals[0], d64_vals[1], d64_vals[2],
                d128_vals[0], d128_vals[1],
                #else
                d64_vals[0], d64_vals[1], d64_vals[2],
                d128_vals[0], d128_vals[1],
                #endif
                c_vals[0], c_vals[1], c_vals[2], c_vals[3],
                #if HAS_VECTORS
                int_vec, double_vec
                #else
                int_vec, vec_result
                #endif
            );
            
            global_accumulator += helper_result;
            checksum += creal(helper_result) + cimag(helper_result);
        } else {
            /* Alternative path with DFP helper (10 arguments) */
            #if HAS_DFP
            _Decimal128 dfp_helper_result = helper_10_args_dfp(
                d64_vals[0], d64_vals[1], d64_vals[2], d64_vals[3],
                d128_vals[0], d128_vals[1], d128_vals[2],
                d128_vals[3], d128_vals[4], d128_vals[5]
            );
            global_accumulator += (long double)dfp_helper_result;
            checksum += (double)dfp_helper_result;
            #else
            decimal128_sim dfp_helper_result = helper_10_args_dfp(
                d64_vals[0], d64_vals[1], d64_vals[2], d64_vals[3],
                d128_vals[0], d128_vals[1], d128_vals[2],
                d128_vals[3], d128_vals[4], d128_vals[5]
            );
            global_accumulator += dfp_helper_result.ld;
            checksum += (double)dfp_helper_result.ld;
            #endif
        }
        
        /* Modify values for next iteration to prevent constant folding */
        for (int i = 0; i < 4; i++) {
            #if HAS_DFP
            d64_vals[i] += 0.5dd;
            #else
            d64_vals[i].f += 0.5;
            #endif
            c_vals[i] += 0.25 + 0.25 * I;
        }
        
        #if HAS_VECTORS
        for (int i = 0; i < 8; i++) int_vec[i] += 1;
        for (int i = 0; i < 4; i++) double_vec[i] += 0.1;
        #else
        int_vec += 8;
        for (int i = 0; i < 4; i++) double_arr[i] += 0.1;
        #endif
    }
    
    /* Final checksum computation */
    printf("Checksum: %f\n", (double)checksum);
    printf("Real accumulator: %Lf\n", creal(global_accumulator));
    printf("Imag accumulator: %Lf\n", cimag(global_accumulator));
    
    return 0;
}
