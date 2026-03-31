#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__) || defined(__aarch64__))
    #define HAS_VECTORS 1
    typedef int32_t int32x4_t __attribute__((vector_size(16)));
    typedef int64_t int64x2_t __attribute__((vector_size(16)));
    typedef double float64x2_t __attribute__((vector_size(16)));
#else
    #define HAS_VECTORS 0
#endif

/* DFP fallback using unions for software emulation */
#if !HAS_DFP
typedef union {
    unsigned long long parts[2];
    long double approx;
} decimal64_fb;

typedef union {
    unsigned long long parts[4];
    long double approx[2];
} decimal128_fb;

#define DECIMAL64(x) ((decimal64_fb){{(x), 0}})
#define DECIMAL128(x) ((decimal128_fb){{(x), 0, 0, 0}})
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double helper_11_args(
#if HAS_DFP
    _Decimal64 d1, _Decimal64 d2, _Decimal128 d3, _Decimal128 d4,
#else
    decimal64_fb d1, decimal64_fb d2, decimal128_fb d3, decimal128_fb d4,
#endif
#if HAS_COMPLEX
    long double _Complex c1, long double _Complex c2,
#else
    long double c1_real, long double c1_imag, long double c2_real, long double c2_imag,
#endif
    int64_t i1, int64_t i2,
#if HAS_VECTORS
    int32x4_t v1, float64x2_t v2
#else
    int32_t v1[4], double v2[2]
#endif
) {
    long double result = 0.0L;
    
#if HAS_DFP
    /* DFP operations that may expand to multi-operand patterns */
    result += (long double)d1 + (long double)d2;
    result += (long double)d3 + (long double)d4;
#else
    result += d1.approx + d2.approx;
    result += d3.approx[0] + d3.approx[1];
#endif

#if HAS_COMPLEX
    /* Complex operations */
    result += creall(c1) + cimagl(c1);
    result += creall(c2) + cimagl(c2);
#else
    result += c1_real + c1_imag;
    result += c2_real + c2_imag;
#endif

    result += (long double)i1 + (long double)i2;
    
#if HAS_VECTORS
    /* Vector reduction */
    for (int i = 0; i < 4; i++) result += (long double)v1[i];
    for (int i = 0; i < 2; i++) result += (long double)v2[i];
#else
    for (int i = 0; i < 4; i++) result += (long double)v1[i];
    for (int i = 0; i < 2; i++) result += (long double)v2[i];
#endif
    
    return result;
}

/* Another helper with 10 arguments for different case */
static __attribute__((noinline))
long double helper_10_args(
    long double a1, long double a2, long double a3, long double a4,
    long double a5, long double a6, long double a7, long double a8,
    long double a9, long double a10
) {
    /* Complex expression that may require many temporaries */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6)) / 
           ((a7 + a8) * (a9 - a10) + 1.0L);
}

/* Volatile storage to prevent optimization */
volatile long double g_result_store[10];
volatile int g_store_idx = 0;

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) seed = atoi(argv[1]);
    
    srand(seed);
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.23456dd;
    _Decimal64 d64_b = 7.89012dd;
    _Decimal128 d128_c = 123456789012345.678901234567890dl;
    _Decimal128 d128_d = 987654321098765.432109876543210dl;
#else
    decimal64_fb d64_a = DECIMAL64(seed * 1.23456L);
    decimal64_fb d64_b = DECIMAL64(seed * 7.89012L);
    decimal128_fb d128_c = DECIMAL128(seed * 123456.789012L);
    decimal128_fb d128_d = DECIMAL128(seed * 987654.321098L);
#endif

#if HAS_COMPLEX
    long double _Complex ca = (seed * 1.5L) + (seed * 2.5L) * I;
    long double _Complex cb = (seed * 3.5L) + (seed * 4.5L) * I;
    long double _Complex cc = (seed * 5.5L) + (seed * 6.5L) * I;
    long double _Complex cd = (seed * 7.5L) + (seed * 8.5L) * I;
#else
    long double ca_real = seed * 1.5L, ca_imag = seed * 2.5L;
    long double cb_real = seed * 3.5L, cb_imag = seed * 4.5L;
    long double cc_real = seed * 5.5L, cc_imag = seed * 6.5L;
    long double cd_real = seed * 7.5L, cd_imag = seed * 8.5L;
#endif

#if HAS_VECTORS
    int32x4_t vec_int = {seed + 1, seed + 2, seed + 3, seed + 4};
    float64x2_t vec_dbl = {seed * 1.1, seed * 2.2};
#else
    int32_t vec_int[4] = {seed + 1, seed + 2, seed + 3, seed + 4};
    double vec_dbl[2] = {seed * 1.1, seed * 2.2};
#endif

    long double checksum = 0.0L;
    
    /* Main computation loop - 4 iterations */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = iter % 2; /* Prevent constant folding */
        long double iter_result = 0.0L;
        
        if (condition) {
            /* Branch 1: Complex DFP operations */
#if HAS_DFP
            /* Multi-step DFP expression that may expand to many operands */
            _Decimal128 d128_temp1 = d128_c * d128_d;
            _Decimal128 d128_temp2 = d128_c / d128_d;
            _Decimal128 d128_result = d128_temp1 + d128_temp2 - d128_c;
            
            /* More complex DFP chain */
            _Decimal64 d64_result = d64_a * d64_b + d64_a / d64_b - d64_a;
            
            iter_result += (long double)d128_result + (long double)d64_result;
#else
            /* Software emulation of multi-precision operations */
            decimal128_fb d128_temp1, d128_temp2, d128_result;
            decimal64_fb d64_result;
            
            d128_temp1.approx[0] = d128_c.approx[0] * d128_d.approx[0];
            d128_temp1.approx[1] = d128_c.approx[1] * d128_d.approx[1];
            
            d128_temp2.approx[0] = d128_c.approx[0] / d128_d.approx[0];
            d128_temp2.approx[1] = d128_c.approx[1] / d128_d.approx[1];
            
            d128_result.approx[0] = d128_temp1.approx[0] + d128_temp2.approx[0] - d128_c.approx[0];
            d128_result.approx[1] = d128_temp1.approx[1] + d128_temp2.approx[1] - d128_c.approx[1];
            
            d64_result.approx = d64_a.approx * d64_b.approx + 
                               d64_a.approx / d64_b.approx - 
                               d64_a.approx;
            
            iter_result += d128_result.approx[0] + d128_result.approx[1] + d64_result.approx;
#endif
        } else {
            /* Branch 2: Complex number operations */
#if HAS_COMPLEX
            /* Complex expressions that may expand to many operands */
            long double _Complex c_temp1 = ca * cb;
            long double _Complex c_temp2 = cc - cd;
            long double _Complex c_result = c_temp1 / c_temp2;
            
            /* More complex operations */
            long double _Complex c_pow = cpow(ca, cb);
            long double _Complex c_sqrt = csqrt(cc);
            
            iter_result += creall(c_result) + cimagl(c_result) +
                          creall(c_pow) + cimagl(c_pow) +
                          creall(c_sqrt) + cimagl(c_sqrt);
#else
            /* Manual complex arithmetic */
            long double temp1_real = ca_real * cb_real - ca_imag * cb_imag;
            long double temp1_imag = ca_real * cb_imag + ca_imag * cb_real;
            
            long double temp2_real = cc_real - cd_real;
            long double temp2_imag = cc_imag - cd_imag;
            
            long double denom = temp2_real * temp2_real + temp2_imag * temp2_imag;
            long double c_result_real = (temp1_real * temp2_real + temp1_imag * temp2_imag) / denom;
            long double c_result_imag = (temp1_imag * temp2_real - temp1_real * temp2_imag) / denom;
            
            iter_result += c_result_real + c_result_imag;
#endif
        }
        
        /* Vector reduction with accumulation */
        long double vec_sum = 0.0L;
#if HAS_VECTORS
        /* Horizontal reduction that may expand to multiple operations */
        vec_sum += vec_int[0] + vec_int[1] + vec_int[2] + vec_int[3];
        vec_sum += vec_dbl[0] + vec_dbl[1];
        
        /* Vector update for next iteration */
        vec_int += (int32x4_t){1, 2, 3, 4};
        vec_dbl *= (float64x2_t){1.01, 1.02};
#else
        for (int i = 0; i < 4; i++) vec_sum += vec_int[i];
        for (int i = 0; i < 2; i++) vec_sum += vec_dbl[i];
        
        for (int i = 0; i < 4; i++) vec_int[i] += i + 1;
        for (int i = 0; i < 2; i++) vec_dbl[i] *= 1.01 + i * 0.01;
#endif
        
        iter_result += vec_sum;
        
        /* Call helper with many arguments */
        long double helper_result;
        if (iter % 3 == 0) {
            /* 11-argument call */
            helper_result = helper_11_args(
#if HAS_DFP
                d64_a, d64_b, d128_c, d128_d,
#else
                d64_a, d64_b, d128_c, d128_d,
#endif
#if HAS_COMPLEX
                ca, cb,
#else
                ca_real, ca_imag, cb_real, cb_imag,
#endif
                (int64_t)seed * iter, (int64_t)seed * (iter + 1),
#if HAS_VECTORS
                vec_int, vec_dbl
#else
                vec_int, vec_dbl
#endif
            );
        } else {
            /* 10-argument call with complex expression results as arguments */
            helper_result = helper_10_args(
                iter_result, vec_sum,
                (long double)seed * 1.1L, (long double)seed * 2.2L,
                (long double)seed * 3.3L, (long double)seed * 4.4L,
                (long double)seed * 5.5L, (long double)seed * 6.6L,
                (long double)seed * 7.7L, (long double)seed * 8.8L
            );
        }
        
        iter_result += helper_result;
        
        /* Store result to prevent elimination */
        if (g_store_idx < 10) {
            g_result_store[g_store_idx++] = iter_result;
            checksum += iter_result;
        }
        
        /* Update variables for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final checksum and output */
    printf("Checksum: %.15Lf\n", checksum);
    
    /* Additional forced computation to ensure all paths are used */
    long double final_check = 0.0L;
    for (int i = 0; i < g_store_idx; i++) {
        final_check += g_result_store[i];
    }
    
    if (final_check != checksum) {
        printf("Verification failed!\n");
        return 1;
    }
    
    return 0;
}
