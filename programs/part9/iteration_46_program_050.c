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
    #define VECTOR_SIZE 8
    typedef int32_t int32x8_t __attribute__((vector_size(32)));
    typedef float float32x8_t __attribute__((vector_size(32)));
    #define HAS_VECTORS 1
#else
    #define VECTOR_SIZE 8
    #define HAS_VECTORS 0
#endif

/* DFP fallback using unions for software emulation */
#if !HAS_DFP
typedef union {
    unsigned long long parts[2];
    double approximate;
} decimal64_fb;

typedef union {
    unsigned long long parts[4];
    long double approximate;
} decimal128_fb;

#define DECIMAL64(x) ((decimal64_fb){{(x), 0}})
#define DECIMAL128(x) ((decimal128_fb){{(x), 0, 0, 0}})
#define decimal64_add(a,b) DECIMAL64(a.approximate + b.approximate)
#define decimal64_mul(a,b) DECIMAL64(a.approximate * b.approximate)
#define decimal128_add(a,b) DECIMAL128(a.approximate + b.approximate)
#define decimal128_mul(a,b) DECIMAL128(a.approximate * b.approximate)
#define decimal128_div(a,b) DECIMAL128(a.approximate / b.approximate)
#endif

/* Complex fallback */
#if !HAS_COMPLEX
typedef struct {
    long double real;
    long double imag;
} complex_fb;

#define complex_add(a,b) ((complex_fb){a.real + b.real, a.imag + b.imag})
#define complex_mul(a,b) ((complex_fb){a.real*b.real - a.imag*b.imag, \
                                       a.real*b.imag + a.imag*b.real})
#define complex_div(a,b) { \
    long double denom = b.real*b.real + b.imag*b.imag; \
    ((complex_fb){(a.real*b.real + a.imag*b.imag)/denom, \
                  (a.imag*b.real - a.real*b.imag)/denom}) \
}
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
    complex_fb c1, complex_fb c2,
    #endif
    #if HAS_VECTORS
    int32x8_t v1, float32x8_t v2,
    #else
    int32_t v1[VECTOR_SIZE], float v2[VECTOR_SIZE],
    #endif
    int accum1, long double accum2)
{
    long double result = accum2;
    
    /* Process DFP values */
    #if HAS_DFP
    result += (long double)d1 + (long double)d2;
    result += (long double)d3 + (long double)d4;
    #else
    result += d1.approximate + d2.approximate;
    result += d3.approximate + d4.approximate;
    #endif
    
    /* Process complex values */
    #if HAS_COMPLEX
    result += creall(c1) + cimagl(c1) + creall(c2) + cimagl(c2);
    #else
    result += c1.real + c1.imag + c2.real + c2.imag;
    #endif
    
    /* Process vector values */
    #if HAS_VECTORS
    for (int i = 0; i < 8; i++) {
        result += v1[i] + v2[i];
    }
    #else
    for (int i = 0; i < VECTOR_SIZE; i++) {
        result += v1[i] + v2[i];
    }
    #endif
    
    result += accum1;
    return result;
}

/* Another helper with 10 arguments */
static __attribute__((noinline))
long double helper_10_args(
    #if HAS_DFP
    _Decimal128 a1, _Decimal128 a2, _Decimal128 a3, _Decimal128 a4,
    #else
    decimal128_fb a1, decimal128_fb a2, decimal128_fb a3, decimal128_fb a4,
    #endif
    #if HAS_COMPLEX
    long double _Complex b1, long double _Complex b2,
    long double _Complex b3, long double _Complex b4,
    #else
    complex_fb b1, complex_fb b2, complex_fb b3, complex_fb b4,
    #endif
    long double scale)
{
    long double result = scale;
    
    #if HAS_DFP
    result += (long double)a1 * (long double)a2;
    result += (long double)a3 / (long double)a4;
    #else
    result += a1.approximate * a2.approximate;
    result += a3.approximate / a4.approximate;
    #endif
    
    #if HAS_COMPLEX
    result += creall(b1 * b2) + cimagl(b3 * b4);
    #else
    complex_fb tmp1 = complex_mul(b1, b2);
    complex_fb tmp2 = complex_mul(b3, b4);
    result += tmp1.real + tmp2.imag;
    #endif
    
    return result;
}

/* Volatile global to prevent optimization */
volatile long double global_accumulator[10];

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
    #if HAS_DFP
    _Decimal64 d64_a = 1.23456789e10DL;
    _Decimal64 d64_b = 9.87654321e9DL;
    _Decimal128 d128_a = 1.2345678901234567890123456789e30DL;
    _Decimal128 d128_b = 9.8765432109876543210987654321e29DL;
    _Decimal128 d128_c = 5.5555555555555555555555555555e28DL;
    _Decimal128 d128_d = 2.2222222222222222222222222222e27DL;
    #else
    decimal64_fb d64_a = DECIMAL64(1.23456789e10);
    decimal64_fb d64_b = DECIMAL64(9.87654321e9);
    decimal128_fb d128_a = DECIMAL128(1.2345678901234567890123456789e30);
    decimal128_fb d128_b = DECIMAL128(9.8765432109876543210987654321e29);
    decimal128_fb d128_c = DECIMAL128(5.5555555555555555555555555555e28);
    decimal128_fb d128_d = DECIMAL128(2.2222222222222222222222222222e27);
    #endif
    
    #if HAS_COMPLEX
    long double _Complex ca = 1.0L + 2.0LI;
    long double _Complex cb = 3.0L - 4.0LI;
    long double _Complex cc = -5.0L + 6.0LI;
    long double _Complex cd = 7.0L - 8.0LI;
    #else
    complex_fb ca = {1.0L, 2.0L};
    complex_fb cb = {3.0L, -4.0L};
    complex_fb cc = {-5.0L, 6.0L};
    complex_fb cd = {7.0L, -8.0L};
    #endif
    
    #if HAS_VECTORS
    int32x8_t vec_int = {1, 2, 3, 4, 5, 6, 7, 8};
    float32x8_t vec_float = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f};
    #else
    int32_t vec_int[VECTOR_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};
    float vec_float[VECTOR_SIZE] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f};
    #endif
    
    int accum_int = 0;
    long double accum_ld = 0.0L;
    int result_index = 0;
    
    /* Main computation loop - fixed 3 iterations */
    for (int iter = 0; iter < 3; iter++) {
        volatile int condition = seed + iter; /* Prevent constant folding */
        
        if (condition % 2 == 0) {
            /* Complex DFP expression that may expand to many operands */
            #if HAS_DFP
            _Decimal128 d128_result = d128_a * d128_b + d128_c / d128_d;
            /* Use DFP builtins if available */
            d128_result = __builtin_dadd(d128_result, d128_a);
            #else
            decimal128_fb d128_result = decimal128_mul(d128_a, d128_b);
            decimal128_fb tmp = decimal128_div(d128_c, d128_d);
            d128_result = decimal128_add(d128_result, tmp);
            d128_result = decimal128_add(d128_result, d128_a);
            #endif
            
            /* Complex arithmetic with library calls */
            #if HAS_COMPLEX
            long double _Complex cl_result = (ca * cb) / (cc - cd);
            /* Additional complex operations */
            cl_result = cl_result * cl_result + ca;
            #ifdef __STDC_IEC_559_COMPLEX__
            cl_result = csqrt(cl_result);
            #endif
            #else
            complex_fb cl_result = complex_div(complex_mul(ca, cb), 
                                              complex_add(cc, (complex_fb){-cd.real, -cd.imag}));
            cl_result = complex_add(complex_mul(cl_result, cl_result), ca);
            #endif
            
            /* Vector reduction with accumulation */
            long double vec_sum = 0.0L;
            #if HAS_VECTORS
            for (int i = 0; i < 8; i++) {
                vec_sum += vec_int[i] + vec_float[i];
            }
            /* Shuffle vector elements to create more complex patterns */
            vec_int = vec_int + 1;
            vec_float = vec_float * 1.1f;
            #else
            for (int i = 0; i < VECTOR_SIZE; i++) {
                vec_sum += vec_int[i] + vec_float[i];
                vec_int[i] += 1;
                vec_float[i] *= 1.1f;
            }
            #endif
            
            accum_int += (int)vec_sum;
            accum_ld += vec_sum;
            
            /* Call helper with 11 arguments */
            long double res1 = helper_11_args(
                #if HAS_DFP
                d64_a, d64_b, d128_result, d128_a,
                #else
                d64_a, d64_b, d128_result, d128_a,
                #endif
                #if HAS_COMPLEX
                ca, cl_result,
                #else
                ca, cl_result,
                #endif
                #if HAS_VECTORS
                vec_int, vec_float,
                #else
                vec_int, vec_float,
                #endif
                accum_int, accum_ld);
            
            global_accumulator[result_index++] = res1;
        } else {
            /* Alternative path with different operations */
            #if HAS_DFP
            _Decimal128 d128_tmp1 = d128_b * d128_c;
            _Decimal128 d128_tmp2 = d128_d / d128_a;
            _Decimal128 d128_combined = d128_tmp1 + d128_tmp2;
            #else
            decimal128_fb d128_tmp1 = decimal128_mul(d128_b, d128_c);
            decimal128_fb d128_tmp2 = decimal128_div(d128_d, d128_a);
            decimal128_fb d128_combined = decimal128_add(d128_tmp1, d128_tmp2);
            #endif
            
            #if HAS_COMPLEX
            long double _Complex c_tmp1 = cb * cc;
            long double _Complex c_tmp2 = cd / ca;
            long double _Complex c_combined = c_tmp1 - c_tmp2;
            #ifdef __STDC_IEC_559_COMPLEX__
            c_combined = cpow(c_combined, 2.0L);
            #endif
            #else
            complex_fb c_tmp1 = complex_mul(cb, cc);
            complex_fb c_tmp2 = complex_div(cd, ca);
            complex_fb c_combined = complex_add(c_tmp1, (complex_fb){-c_tmp2.real, -c_tmp2.imag});
            #endif
            
            /* Call helper with 10 arguments */
            long double res2 = helper_10_args(
                #if HAS_DFP
                d128_combined, d128_a, d128_b, d128_c,
                #else
                d128_combined, d128_a, d128_b, d128_c,
                #endif
                #if HAS_COMPLEX
                c_combined, ca, cb, cc,
                #else
                c_combined, ca, cb, cc,
                #endif
                accum_ld);
            
            global_accumulator[result_index++] = res2;
            
            /* Update accumulators */
            accum_int += iter * 7;
            accum_ld += res2 * 0.5L;
        }
        
        /* Small fixed iteration inner loop */
        for (int inner = 0; inner < 2; inner++) {
            /* Repeated complex expression to increase operand count */
            #if HAS_DFP
            d128_a = d128_a * 1.1DL + d128_b;
            #else
            d128_a.approximate = d128_a.approximate * 1.1L + d128_b.approximate;
            #endif
            
            #if HAS_COMPLEX
            ca = ca * 1.1L + cb;
            #else
            ca.real = ca.real * 1.1L + cb.real;
            ca.imag = ca.imag * 1.1L + cb.imag;
            #endif
        }
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < result_index; i++) {
        checksum += global_accumulator[i];
    }
    
    /* Print deterministic result */
    printf("Checksum: %.15Lf\n", checksum);
    printf("Result count: %d\n", result_index);
    
    return 0;
}
