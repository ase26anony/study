/* 
 * Test program targeting GCC optabs.cc lines 8254-8263
 * Designed to trigger internal function expansion with 10-11 operands
 * through DFP, complex, and vector operations.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Feature detection and fallbacks */
#ifdef __DECIMAL_BID_FORMAT__
    #define HAS_DFP 1
    #include <dfp.h>
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

/* DFP fallback types if not supported */
#if !HAS_DFP
typedef struct {
    uint64_t hi;
    uint64_t lo;
} decimal64_fb;

typedef struct {
    uint64_t w[2];
} decimal128_fb;
#endif

/* Vector fallback */
#if !HAS_VECTOR
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef double float64x2_t __attribute__((vector_size(16)));
#endif

/* Helper function with 11 arguments - targeting case 11 */
static __attribute__((noinline))
long double helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex expression that may expand to many operands */
    return ((a1 * a2) + (a3 / a4) - (a5 * a6) + 
            (a7 - a8) * (a9 + a10) / a11);
}

/* Helper function with 10 arguments - targeting case 10 */
static __attribute__((noinline))
long double helper_10_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10)
{
    return (a1 + a2) * (a3 - a4) / (a5 * a6) + 
           (a7 / a8) - (a9 * a10);
}

/* Vector reduction with accumulation */
static long double vector_reduce_accumulate(float64x2_t vec, long double acc)
{
    /* Horizontal reduction */
    float64x2_t temp = vec + __builtin_shuffle(vec, (int64x2_t){1, 0});
    return acc + temp[0];
}

/* Complex number operations */
#if HAS_COMPLEX
static long double _Complex complex_operation(
    long double _Complex a,
    long double _Complex b,
    long double _Complex c,
    long double _Complex d)
{
    /* Complex expression that may generate many operands */
    return (a * b) / (c - d) + (a + b) * (c * d);
}
#endif

/* DFP operations with fallback */
static void dfp_operations(
#if HAS_DFP
    _Decimal64 *d64_result,
    _Decimal128 *d128_result,
    _Decimal64 d64_a,
    _Decimal64 d64_b,
    _Decimal64 d64_c,
    _Decimal128 d128_a,
    _Decimal128 d128_b,
    _Decimal128 d128_c
#else
    decimal64_fb *d64_result,
    decimal128_fb *d128_result,
    decimal64_fb d64_a,
    decimal64_fb d64_b,
    decimal64_fb d64_c,
    decimal128_fb d128_a,
    decimal128_fb d128_b,
    decimal128_fb d128_c
#endif
)
{
#if HAS_DFP
    /* DFP arithmetic that may expand to multi-operand patterns */
    *d64_result = d64_a * d64_b + d64_c / d64_a;
    *d128_result = (d128_a * d128_b) + (d128_c / d128_a) - 
                   (d128_b * d128_c) + (d128_a / d128_b);
#else
    /* Fallback using integer arithmetic */
    d64_result->hi = d64_a.hi * d64_b.hi + d64_c.hi;
    d64_result->lo = d64_a.lo * d64_b.lo + d64_c.lo;
    
    d128_result->w[0] = d128_a.w[0] * d128_b.w[0] + d128_c.w[0];
    d128_result->w[1] = d128_a.w[1] * d128_b.w[1] + d128_c.w[1];
#endif
}

/* Volatile storage to prevent optimization */
volatile long double g_result_storage[10];
volatile int g_result_index = 0;

int main(int argc, char *argv[])
{
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize base variables */
    long double ld_vals[12];
    for (int i = 0; i < 12; i++) {
        ld_vals[i] = (long double)(rand() % 1000) / 100.0L;
    }
    
#if HAS_DFP
    _Decimal64 d64_a = 1.23456dd;
    _Decimal64 d64_b = 2.34567dd;
    _Decimal64 d64_c = 3.45678dd;
    _Decimal64 d64_result;
    
    _Decimal128 d128_a = 1.234567890123456dl;
    _Decimal128 d128_b = 2.345678901234567dl;
    _Decimal128 d128_c = 3.456789012345678dl;
    _Decimal128 d128_result;
#else
    decimal64_fb d64_a = {123456, 0};
    decimal64_fb d64_b = {234567, 0};
    decimal64_fb d64_c = {345678, 0};
    decimal64_fb d64_result;
    
    decimal128_fb d128_a = {{1234567890, 123456}};
    decimal128_fb d128_b = {{2345678901, 234567}};
    decimal128_fb d128_c = {{3456789012, 345678}};
    decimal128_fb d128_result;
#endif
    
#if HAS_COMPLEX
    long double _Complex ca = ld_vals[0] + ld_vals[1] * I;
    long double _Complex cb = ld_vals[2] + ld_vals[3] * I;
    long double _Complex cc = ld_vals[4] + ld_vals[5] * I;
    long double _Complex cd = ld_vals[6] + ld_vals[7] * I;
    long double _Complex cresult;
#endif
    
    /* Vector initialization */
    float64x2_t vec1 = {ld_vals[8], ld_vals[9]};
    float64x2_t vec2 = {ld_vals[10], ld_vals[11]};
    long double vec_acc = 0.0L;
    
    /* Main computation loop - 4 iterations */
    for (int iter = 0; iter < 4; iter++) {
        volatile int condition = rand() % 2;
        long double loop_result = 0.0L;
        
        /* Conditional execution to prevent constant folding */
        if (condition) {
            /* DFP operations */
            dfp_operations(&d64_result, &d128_result, 
                          d64_a, d64_b, d64_c,
                          d128_a, d128_b, d128_c);
            
#if HAS_DFP
            /* Convert DFP to long double for helper functions */
            long double ld_d64 = (long double)d64_result;
            long double ld_d128 = (long double)d128_result;
#else
            long double ld_d64 = (long double)d64_result.hi;
            long double ld_d128 = (long double)d128_result.w[0];
#endif
            
            /* Complex operations */
#if HAS_COMPLEX
            cresult = complex_operation(ca, cb, cc, cd);
            long double creal = creal(cresult);
            long double cimag = cimag(cresult);
#else
            long double creal = ld_vals[0] * ld_vals[2] - ld_vals[1] * ld_vals[3];
            long double cimag = ld_vals[0] * ld_vals[3] + ld_vals[1] * ld_vals[2];
#endif
            
            /* Vector reduction */
            vec_acc = vector_reduce_accumulate(vec1 + vec2, vec_acc);
            
            /* Call helper with 11 arguments - targeting case 11 */
            loop_result = helper_11_args(
                ld_d64, ld_d128, creal, cimag, vec_acc,
                ld_vals[0], ld_vals[1], ld_vals[2],
                ld_vals[3], ld_vals[4], ld_vals[5]);
        } else {
            /* Alternative path with 10-argument helper */
            vec_acc = vector_reduce_accumulate(vec1 * vec2, vec_acc);
            
#if HAS_COMPLEX
            cresult = (ca * cb) / (cc - cd);
            long double creal = creal(cresult);
            long double cimag = cimag(cresult);
#else
            long double creal = (ld_vals[0] * ld_vals[2] + ld_vals[1] * ld_vals[3]) /
                               (ld_vals[4] - ld_vals[6]);
            long double cimag = (ld_vals[0] * ld_vals[3] - ld_vals[1] * ld_vals[2]) /
                               (ld_vals[5] - ld_vals[7]);
#endif
            
            /* Call helper with 10 arguments - targeting case 10 */
            loop_result = helper_10_args(
                ld_vals[6], ld_vals[7], ld_vals[8],
                ld_vals[9], ld_vals[10], ld_vals[11],
                creal, cimag, vec_acc, (long double)iter);
        }
        
        /* Store result to prevent dead code elimination */
        if (g_result_index < 10) {
            g_result_storage[g_result_index++] = loop_result;
        }
        
        /* Modify some values for next iteration */
        for (int i = 0; i < 12; i++) {
            ld_vals[i] += 0.1L;
        }
        
#if HAS_COMPLEX
        ca += 0.1L + 0.1L * I;
        cb += 0.2L + 0.2L * I;
#endif
        
        vec1[0] += 0.1;
        vec1[1] += 0.2;
    }
    
    /* Compute checksum */
    long double checksum = 0.0L;
    for (int i = 0; i < g_result_index; i++) {
        checksum += g_result_storage[i];
    }
    
    /* Print deterministic result */
    printf("Result checksum: %Lf\n", checksum);
    printf("Iterations completed: %d\n", g_result_index);
    
    return 0;
}
