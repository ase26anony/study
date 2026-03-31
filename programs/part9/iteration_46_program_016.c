#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    typedef int32_t v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
#else
    #define HAS_VECTORS 0
#endif

/* DFP fallback using integer arrays */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dfp64_fb;

typedef struct {
    uint64_t parts[4];
} dfp128_fb;

#define DECIMAL64_FB(x) {(x##ULL), 0}
#define DECIMAL128_FB(x) {{(x##ULL), 0, 0, 0}}
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double re;
    long double im;
} complex_ld_fb;
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double helper_11_args(
#if HAS_DFP
    _Decimal64 d1, _Decimal64 d2, _Decimal128 d3, _Decimal128 d4,
#else
    dfp64_fb d1, dfp64_fb d2, dfp128_fb d3, dfp128_fb d4,
#endif
#if HAS_COMPLEX
    long double _Complex c1, long double _Complex c2,
#else
    complex_ld_fb c1, complex_ld_fb c2,
#endif
#if HAS_VECTORS
    v8si v1, v4df v2,
#else
    int32_t v1[8], double v2[4],
#endif
    int extra1, long double extra2, int extra3)
{
    long double result = 0.0L;
    
    /* Combine all arguments in a way that uses them all */
#if HAS_DFP
    result += (long double)d1 + (long double)d2;
    result += (long double)d3 + (long double)d4;
#else
    result += (long double)d1.lo + (long double)d2.lo;
    result += (long double)d3.parts[0] + (long double)d4.parts[0];
#endif
    
#if HAS_COMPLEX
    result += creall(c1) + cimagl(c1) + creall(c2) + cimagl(c2);
#else
    result += c1.re + c1.im + c2.re + c2.im;
#endif
    
#if HAS_VECTORS
    /* Vector reduction */
    for (int i = 0; i < 8; i++) {
        result += v1[i];
    }
    for (int i = 0; i < 4; i++) {
        result += v2[i];
    }
#else
    for (int i = 0; i < 8; i++) {
        result += v1[i];
    }
    for (int i = 0; i < 4; i++) {
        result += v2[i];
    }
#endif
    
    result += extra1 + extra2 + extra3;
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
long double helper_10_args(
#if HAS_DFP
    _Decimal128 a1, _Decimal128 a2, _Decimal128 a3, _Decimal128 a4,
#else
    dfp128_fb a1, dfp128_fb a2, dfp128_fb a3, dfp128_fb a4,
#endif
#if HAS_COMPLEX
    long double _Complex b1, long double _Complex b2,
    long double _Complex b3, long double _Complex b4,
#else
    complex_ld_fb b1, complex_ld_fb b2, complex_ld_fb b3, complex_ld_fb b4,
#endif
    long double c1, long double c2)
{
    long double result = 0.0L;
    
#if HAS_DFP
    result += (long double)(a1 * a2 + a3 / a4);
#else
    result += (long double)a1.parts[0] * a2.parts[0] + 
              (long double)a3.parts[0] / a4.parts[0];
#endif
    
#if HAS_COMPLEX
    /* Complex arithmetic that may expand to many operands */
    long double _Complex temp = (b1 * b2) / (b3 - b4);
    result += creall(temp) + cimagl(temp);
#else
    complex_ld_fb temp;
    temp.re = (b1.re * b2.re - b1.im * b2.im) / (b3.re - b4.re);
    temp.im = (b1.re * b2.im + b1.im * b2.re) / (b3.im - b4.im);
    result += temp.re + temp.im;
#endif
    
    result += c1 * c2;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    volatile long double checksum = 0.0L;
    long double results[5] = {0};
    
    /* Initialize base variables */
#if HAS_DFP
    _Decimal64 d64_a = 1.234567dd;
    _Decimal64 d64_b = 9.876543dd;
    _Decimal128 d128_a = 1.2345678901234567dl;
    _Decimal128 d128_b = 9.8765432109876543dl;
    _Decimal128 d128_c = 5.5555555555555555dl;
    _Decimal128 d128_d = 2.2222222222222222dl;
#else
    dfp64_fb d64_a = DECIMAL64_FB(1234567);
    dfp64_fb d64_b = DECIMAL64_FB(9876543);
    dfp128_fb d128_a = DECIMAL128_FB(12345678901234567);
    dfp128_fb d128_b = DECIMAL128_FB(98765432109876543);
    dfp128_fb d128_c = DECIMAL128_FB(55555555555555555);
    dfp128_fb d128_d = DECIMAL128_FB(22222222222222222);
#endif
    
#if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L - 4.0LI;
    long double _Complex cc = 5.0L + 6.0LI;
    long double _Complex cd = 7.0L - 8.0LI;
#else
    complex_ld_fb ca = {1.0L, 2.0L};
    complex_ld_fb cb = {3.0L, -4.0L};
    complex_ld_fb cc = {5.0L, 6.0L};
    complex_ld_fb cd = {7.0L, -8.0L};
#endif
    
#if HAS_VECTORS
    v8si vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
    v4df vec_dbl = {1.1, 2.2, 3.3, 4.4};
#else
    int32_t vec_int[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    double vec_dbl[4] = {1.1, 2.2, 3.3, 4.4};
#endif
    
    /* Loop with conditional execution */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = seed + iter;
        
        if (condition % 2 == 0) {
            /* DFP arithmetic that may expand to many operands */
#if HAS_DFP
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            d128_result = d128_result - d128_a + d128_b * d128_c;
#else
            dfp128_fb d128_result;
            d128_result.parts[0] = d128_a.parts[0] * d128_b.parts[0] + 
                                   d128_c.parts[0] / d128_d.parts[0];
#endif
            
            /* Complex arithmetic */
#if HAS_COMPLEX
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            /* Complex power function - may expand to many operations */
            cl_result = cpow(cl_result, 2.0L + 0.0LI);
#else
            complex_ld_fb cl_result;
            cl_result.re = (ca.re * cb.re - ca.im * cb.im) / (cc.re - cd.re);
            cl_result.im = (ca.re * cb.im + ca.im * cb.re) / (cc.im - cd.im);
#endif
            
            /* Vector reduction with accumulation */
            long double vec_sum = 0.0L;
#if HAS_VECTORS
            /* Horizontal reduction */
            for (int i = 0; i < 8; i++) {
                vec_sum += vec_int[i];
            }
            for (int i = 0; i < 4; i++) {
                vec_sum += vec_dbl[i];
            }
#else
            for (int i = 0; i < 8; i++) {
                vec_sum += vec_int[i];
            }
            for (int i = 0; i < 4; i++) {
                vec_sum += vec_dbl[i];
            }
#endif
            
            /* Call helper with 11 arguments */
            results[iter] = helper_11_args(
#if HAS_DFP
                d64_a, d64_b, d128_result, d128_a,
#else
                d64_a, d64_b, d128_result, d128_a,
#endif
#if HAS_COMPLEX
                cl_result, ca,
#else
                cl_result, ca,
#endif
#if HAS_VECTORS
                vec_int, vec_dbl,
#else
                vec_int, vec_dbl,
#endif
                iter, vec_sum, seed
            );
        } else {
            /* Alternative path with different operations */
#if HAS_DFP
            _Decimal128 d128_temp = d128_b * d128_c - d128_d / d128_a;
            d128_temp = d128_temp + d128_a * d128_d;
#else
            dfp128_fb d128_temp;
            d128_temp.parts[0] = d128_b.parts[0] * d128_c.parts[0] - 
                                 d128_d.parts[0] / d128_a.parts[0];
#endif
            
#if HAS_COMPLEX
            long double _Complex c_temp = csqrt(ca * cb + cc * cd);
            c_temp = c_temp * (2.0L + 1.0LI);
#else
            complex_ld_fb c_temp;
            double mag = sqrt(ca.re * ca.re + ca.im * ca.im);
            c_temp.re = mag * 2.0L;
            c_temp.im = mag * 1.0L;
#endif
            
            /* Call helper with 10 arguments */
            results[iter] = helper_10_args(
#if HAS_DFP
                d128_temp, d128_a, d128_b, d128_c,
#else
                d128_temp, d128_a, d128_b, d128_c,
#endif
#if HAS_COMPLEX
                c_temp, cb, cc, cd,
#else
                c_temp, cb, cc, cd,
#endif
                (long double)iter, (long double)seed
            );
        }
        
        /* Update checksum to prevent optimization */
        checksum += results[iter];
        
        /* Modify some values for next iteration */
#if HAS_VECTORS
        for (int i = 0; i < 8; i++) {
            vec_int[i] += 1;
        }
        for (int i = 0; i < 4; i++) {
            vec_dbl[i] += 0.5;
        }
#else
        for (int i = 0; i < 8; i++) {
            vec_int[i] += 1;
        }
        for (int i = 0; i < 4; i++) {
            vec_dbl[i] += 0.5;
        }
#endif
    }
    
    /* Final checksum computation */
    long double final_checksum = 0.0L;
    for (int i = 0; i < 3; i++) {
        final_checksum += results[i];
    }
    
    printf("Checksum: %Lf\n", final_checksum);
    return 0;
}
