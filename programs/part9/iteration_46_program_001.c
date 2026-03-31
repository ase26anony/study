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

/* Vector type fallbacks */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #define VECTOR_SIZE 8
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef double float64x4_t __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define HAS_VECTORS 0
    typedef struct { int32_t data[8]; } int32x8_t;
    typedef struct { double data[4]; } float64x4_t;
#endif

/* DFP fallback using integer arrays */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t parts[2];
} decimal128_fb;

#define DEC64_ZERO {0, 0}
#define DEC128_ZERO {{0, 0}}
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double real;
    long double imag;
} long_double_complex;
#endif

/* Helper function with 11 arguments - marked noinline to prevent optimization */
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
    long_double_complex c1, long_double_complex c2,
    #endif
    int32x8_t v1, float64x4_t v2,
    long double scalar1, long double scalar2,
    int modifier)
{
    long double result = 0.0L;
    
    /* Convert/process DFP values */
    #if HAS_DFP
    result += (long double)d1 + (long double)d2;
    #else
    result += (long double)d1.lo + (long double)d2.lo;
    #endif
    
    /* Process complex values */
    #if HAS_COMPLEX
    result += creall(c1) + cimagl(c2);
    #else
    result += c1.real + c2.imag;
    #endif
    
    /* Process vectors */
    #if HAS_VECTORS
    for (int i = 0; i < 8; i++) result += v1[i];
    for (int i = 0; i < 4; i++) result += v2[i];
    #else
    for (int i = 0; i < 8; i++) result += v1.data[i];
    for (int i = 0; i < 4; i++) result += v2.data[i];
    #endif
    
    result += scalar1 * scalar2;
    result *= (modifier % 7) + 1;
    
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
long double helper_10_args(
    #if HAS_DFP
    _Decimal64 d1, _Decimal64 d2, _Decimal64 d3, _Decimal128 d4,
    #else
    decimal64_fb d1, decimal64_fb d2, decimal64_fb d3, decimal128_fb d4,
    #endif
    #if HAS_COMPLEX
    long double _Complex c1, long double _Complex c2,
    #else
    long_double_complex c1, long_double_complex c2,
    #endif
    int32x8_t v1, float64x4_t v2,
    long double scalar)
{
    long double result = scalar;
    
    /* Complex multiplication simulation */
    #if HAS_COMPLEX
    long double _Complex cprod = c1 * c2;
    result += creall(cprod) + cimagl(cprod);
    #else
    long_double_complex cprod;
    cprod.real = c1.real * c2.real - c1.imag * c2.imag;
    cprod.imag = c1.real * c2.imag + c1.imag * c2.real;
    result += cprod.real + cprod.imag;
    #endif
    
    return result;
}

/* DFP arithmetic operations */
#if HAS_DFP
static _Decimal128 dfp_operation(_Decimal128 a, _Decimal128 b, 
                                 _Decimal128 c, _Decimal128 d,
                                 volatile int* cond)
{
    /* Complex expression that may expand to many operands */
    if (*cond > 0) {
        return a * b + c / d - (_Decimal128)3.14;
    } else {
        return (a + b) * (c - d) / (_Decimal128)2.0;
    }
}
#else
static decimal128_fb dfp_operation_fb(decimal128_fb a, decimal128_fb b,
                                      decimal128_fb c, decimal128_fb d,
                                      volatile int* cond)
{
    /* Simulate DFP with integer arithmetic */
    decimal128_fb result;
    if (*cond > 0) {
        /* a * b + c / d - 3.14 approximation */
        result.parts[0] = a.parts[0] * b.parts[0] + c.parts[0] / (d.parts[0] + 1);
        result.parts[1] = a.parts[1] * b.parts[1] + c.parts[1] / (d.parts[1] + 1);
        result.parts[0] -= 3;
        result.parts[1] -= 14;
    } else {
        result.parts[0] = (a.parts[0] + b.parts[0]) * (c.parts[0] - d.parts[0]) / 2;
        result.parts[1] = (a.parts[1] + b.parts[1]) * (c.parts[1] - d.parts[1]) / 2;
    }
    return result;
}
#endif

/* Complex number operations */
#if HAS_COMPLEX
static long double _Complex complex_operation(long double _Complex a,
                                             long double _Complex b,
                                             long double _Complex c,
                                             long double _Complex d,
                                             volatile int* cond)
{
    /* Nested complex expressions */
    if (*cond % 2 == 0) {
        return (a * b) / (c - d) + (a + b) * (c * d);
    } else {
        return (a / b) + (c * d) - (a - b) / (c + d);
    }
}
#else
static long_double_complex complex_operation_fb(long_double_complex a,
                                               long_double_complex b,
                                               long_double_complex c,
                                               long_double_complex d,
                                               volatile int* cond)
{
    long_double_complex result;
    if (*cond % 2 == 0) {
        /* (a * b) / (c - d) + (a + b) * (c * d) */
        long_double_complex ab, cd_diff, cd_prod, ab_sum, final_prod;
        
        ab.real = a.real * b.real - a.imag * b.imag;
        ab.imag = a.real * b.imag + a.imag * b.real;
        
        cd_diff.real = c.real - d.real;
        cd_diff.imag = c.imag - d.imag;
        
        cd_prod.real = c.real * d.real - c.imag * d.imag;
        cd_prod.imag = c.real * d.imag + c.imag * d.real;
        
        ab_sum.real = a.real + b.real;
        ab_sum.imag = a.imag + b.imag;
        
        /* Complex division: ab / cd_diff */
        long double denom = cd_diff.real * cd_diff.real + cd_diff.imag * cd_diff.imag;
        result.real = (ab.real * cd_diff.real + ab.imag * cd_diff.imag) / denom;
        result.imag = (ab.imag * cd_diff.real - ab.real * cd_diff.imag) / denom;
        
        /* Add (ab_sum * cd_prod) */
        final_prod.real = ab_sum.real * cd_prod.real - ab_sum.imag * cd_prod.imag;
        final_prod.imag = ab_sum.real * cd_prod.imag + ab_sum.imag * cd_prod.real;
        
        result.real += final_prod.real;
        result.imag += final_prod.imag;
    } else {
        /* (a / b) + (c * d) - (a - b) / (c + d) */
        long_double_complex a_div_b, cd_prod, a_minus_b, c_plus_d, second_div;
        
        /* a / b */
        long double denom = b.real * b.real + b.imag * b.imag;
        a_div_b.real = (a.real * b.real + a.imag * b.imag) / denom;
        a_div_b.imag = (a.imag * b.real - a.real * b.imag) / denom;
        
        /* c * d */
        cd_prod.real = c.real * d.real - c.imag * d.imag;
        cd_prod.imag = c.real * d.imag + c.imag * d.real;
        
        /* a - b */
        a_minus_b.real = a.real - b.real;
        a_minus_b.imag = a.imag - b.imag;
        
        /* c + d */
        c_plus_d.real = c.real + d.real;
        c_plus_d.imag = c.imag + d.imag;
        
        /* (a - b) / (c + d) */
        denom = c_plus_d.real * c_plus_d.real + c_plus_d.imag * c_plus_d.imag;
        second_div.real = (a_minus_b.real * c_plus_d.real + a_minus_b.imag * c_plus_d.imag) / denom;
        second_div.imag = (a_minus_b.imag * c_plus_d.real - a_minus_b.real * c_plus_d.imag) / denom;
        
        result.real = a_div_b.real + cd_prod.real - second_div.real;
        result.imag = a_div_b.imag + cd_prod.imag - second_div.imag;
    }
    return result;
}
#endif

/* Vector reduction */
static long double vector_reduction(int32x8_t vi, float64x4_t vf, volatile int* cond)
{
    long double result = 0.0L;
    
    if (*cond > 0) {
        #if HAS_VECTORS
        /* Horizontal reduction */
        for (int i = 0; i < 8; i++) result += vi[i];
        for (int i = 0; i < 4; i++) result += vf[i];
        
        /* Additional operations to increase operand count */
        result += vi[0] * vi[1] + vi[2] * vi[3] + vi[4] * vi[5] + vi[6] * vi[7];
        result += vf[0] * vf[1] + vf[2] * vf[3];
        #else
        for (int i = 0; i < 8; i++) result += vi.data[i];
        for (int i = 0; i < 4; i++) result += vf.data[i];
        
        result += vi.data[0] * vi.data[1] + vi.data[2] * vi.data[3] + 
                  vi.data[4] * vi.data[5] + vi.data[6] * vi.data[7];
        result += vf.data[0] * vf.data[1] + vf.data[2] * vf.data[3];
        #endif
    } else {
        #if HAS_VECTORS
        /* Different reduction pattern */
        result = vi[0];
        for (int i = 1; i < 8; i++) result *= vi[i];
        for (int i = 0; i < 4; i++) result /= (vf[i] + 1.0);
        #else
        result = vi.data[0];
        for (int i = 1; i < 8; i++) result *= vi.data[i];
        for (int i = 0; i < 4; i++) result /= (vf.data[i] + 1.0);
        #endif
    }
    
    return result;
}

int main(int argc, char* argv[])
{
    /* Use command line seed for deterministic behavior */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize volatile condition to prevent constant folding */
    volatile int cond = seed;
    
    /* Initialize DFP values */
    #if HAS_DFP
    _Decimal64 d64_a = 1.5dl;
    _Decimal64 d64_b = 2.75dl;
    _Decimal64 d64_c = 3.14159dl;
    _Decimal128 d128_a = 123456789.123456789dl;
    _Decimal128 d128_b = 987654321.987654321dl;
    _Decimal128 d128_c = 555555555.555555555dl;
    _Decimal128 d128_d = 111111111.111111111dl;
    #else
    decimal64_fb d64_a = {150, 0};
    decimal64_fb d64_b = {275, 0};
    decimal64_fb d64_c = {314159, 0};
    decimal128_fb d128_a = {{123456789, 123456789}};
    decimal128_fb d128_b = {{987654321, 987654321}};
    decimal128_fb d128_c = {{555555555, 555555555}};
    decimal128_fb d128_d = {{111111111, 111111111}};
    #endif
    
    /* Initialize complex values */
    #if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L - 4.0LI;
    long double _Complex cc = 5.0L + 6.0LI;
    long double _Complex cd = 7.0L - 8.0LI;
    #else
    long_double_complex ca = {1.0L, 2.0L};
    long_double_complex cb = {3.0L, -4.0L};
    long_double_complex cc = {5.0L, 6.0L};
    long_double_complex cd = {7.0L, -8.0L};
    #endif
    
    /* Initialize vectors */
    int32x8_t vi;
    float64x4_t vf;
    
    #if HAS_VECTORS
    vi = (int32x8_t){1, 2, 3, 4, 5, 6, 7, 8};
    vf = (float64x4_t){1.1, 2.2, 3.3, 4.4};
    #else
    for (int i = 0; i < 8; i++) vi.data[i] = i + 1;
    for (int i = 0; i < 4; i++) vf.data[i] = (i + 1) * 1.1;
    #endif
    
    /* Storage for results to prevent dead code elimination */
    volatile long double results[5] = {0};
    int result_idx = 0;
    
    /* Main computation loop */
    for (int iter = 0; iter < 3; iter++) {
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* DFP operations */
        #if HAS_DFP
        _Decimal128 dfp_result = dfp_operation(d128_a, d128_b, d128_c, d128_d, &cond);
        results[result_idx++] = (long double)dfp_result;
        #else
        decimal128_fb dfp_result_fb = dfp_operation_fb(d128_a, d128_b, d128_c, d128_d, &cond);
        results[result_idx++] = (long double)dfp_result_fb.parts[0] + dfp_result_fb.parts[1];
        #endif
        
        /* Complex operations */
        #if HAS_COMPLEX
        long double _Complex c_result = complex_operation(ca, cb, cc, cd, &cond);
        results[result_idx++] = creall(c_result) + cimagl(c_result);
        #else
        long_double_complex c_result_fb = complex_operation_fb(ca, cb, cc, cd, &cond);
        results[result_idx++] = c_result_fb.real + c_result_fb.imag;
        #endif
        
        /* Vector reduction */
        long double vec_result = vector_reduction(vi, vf, &cond);
        results[result_idx++] = vec_result;
        
        /* Call helper with 11 arguments */
        long double helper_result = helper_11_args(
            #if HAS_DFP
            d64_a, d64_b, d128_a, d128_b,
            #else
            d64_a, d64_b, d128_a, d128_b,
            #endif
            #if HAS_COMPLEX
            ca, cb,
            #else
            ca, cb,
            #endif
            vi, vf,
            3.14159L, 2.71828L,
            cond);
        results[result_idx++] = helper_result;
        
        /* Call helper with 10 arguments */
        long double helper_result2 = helper_10_args(
            #if HAS_DFP
            d64_a, d64_b, d64_c, d128_c,
            #else
            d64_a, d64_b, d64_c, d128_c,
            #endif
            #if HAS_COMPLEX
            cc, cd,
            #else
            cc, cd,
            #endif
            vi, vf,
            vec_result);
        results[result_idx++] = helper_result2;
        
        /* Modify inputs for next iteration */
        #if HAS_DFP
        d64_a += 0.5dl;
        d128_a += 100.0dl;
        #else
        d64_a.lo += 50;
        d128_a.parts[0] += 100;
        #endif
        
        #if HAS_COMPLEX
        ca += 0.1L + 0.2LI;
        #else
        ca.real += 0.1L;
        ca.imag += 0.2L;
        #endif
        
        #if HAS_VECTORS
        vi += 1;
        vf += 0.5;
        #else
        for (int i = 0; i < 8; i++) vi.data[i] += 1;
        for (int i = 0; i < 4; i++) vf.data[i] += 0.5;
        #endif
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    printf("Result checksum: %Lf\n", checksum);
    printf("Number of results computed: %d\n", result_idx);
    
    return 0;
}
