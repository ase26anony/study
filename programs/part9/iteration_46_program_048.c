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

/* DFP fallback structures */
#if !HAS_DFP
typedef struct {
    uint64_t lo;
    uint64_t hi;
} decimal64_fb;

typedef struct {
    uint64_t w[2];
} decimal128_fb;
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
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

/* Helper function with 10 arguments */
static __attribute__((noinline))
double helper_10_args(
    double b1, double b2, double b3, double b4,
    double b5, double b6, double b7, double b8,
    double b9, double b10)
{
    /* Mix of operations to create complex RTL */
    return (b1 + b2) * (b3 - b4) / (b5 * b6) + 
           (b7 / b8) - (b9 * b10);
}

/* Vector reduction with accumulation */
static float vector_reduce_sum(float32x4_t v) {
    float sum = 0.0f;
    for (int i = 0; i < 4; i++) {
        sum += v[i];
    }
    return sum;
}

/* Complex number operations */
#if HAS_COMPLEX
static long double complex complex_operation(
    long double complex c1,
    long double complex c2,
    long double complex c3,
    long double complex c4)
{
    /* Complex expression that may expand to many real/imag operations */
    return (c1 * c2 + c3) / (c4 - c1 * c2);
}
#endif

/* Main computation loop */
int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    volatile long double result_accumulator = 0.0L;
    volatile double checksum = 0.0;
    
    /* Initialize base variables */
    double base_doubles[20];
    for (int i = 0; i < 20; i++) {
        base_doubles[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    long double base_long_doubles[10];
    for (int i = 0; i < 10; i++) {
        base_long_doubles[i] = (long double)rand() / RAND_MAX * 100.0L;
    }
    
    /* Initialize vectors */
    float32x4_t vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
#if HAS_COMPLEX
    /* Initialize complex numbers */
    long double complex c1 = base_long_doubles[0] + base_long_doubles[1] * I;
    long double complex c2 = base_long_doubles[2] + base_long_doubles[3] * I;
    long double complex c3 = base_long_doubles[4] + base_long_doubles[5] * I;
    long double complex c4 = base_long_doubles[6] + base_long_doubles[7] * I;
#endif
    
    /* Main computation loop - 4 iterations */
    for (int iter = 0; iter < 4; iter++) {
        /* Prevent constant folding with volatile condition */
        volatile int condition = iter % 2;
        
        if (condition) {
            /* Branch 1: Complex expressions with many operands */
            
            /* Create a complex DFP-like expression */
            long double expr1 = base_long_doubles[0] * base_long_doubles[1] +
                               base_long_doubles[2] / base_long_doubles[3] -
                               base_long_doubles[4] * base_long_doubles[5];
            
            long double expr2 = (base_long_doubles[6] + base_long_doubles[7]) *
                               (base_long_doubles[8] - base_long_doubles[9]);
            
            /* Vector operations */
            float32x4_t vec_result = vec1 * vec2 + vec1;
            float vec_sum = vector_reduce_sum(vec_result);
            
#if HAS_COMPLEX
            /* Complex arithmetic */
            long double complex c_result = complex_operation(c1, c2, c3, c4);
            long double complex c_pow = c1 * c1 + c2 * c2;
#endif
            
            /* Call helper with 11 arguments - mixing different computations */
            long double helper_result = helper_11_args(
                expr1, expr2, 
                base_long_doubles[0], base_long_doubles[1],
                base_long_doubles[2], base_long_doubles[3],
                base_long_doubles[4], base_long_doubles[5],
                (long double)vec_sum,
#if HAS_COMPLEX
                creal(c_result),
                cimag(c_result)
#else
                base_long_doubles[6],
                base_long_doubles[7]
#endif
            );
            
            result_accumulator += helper_result;
            
        } else {
            /* Branch 2: Different mix of operations */
            
            /* Another complex expression */
            double d_expr1 = (base_doubles[0] * base_doubles[1] + 
                            base_doubles[2] / base_doubles[3]) *
                           (base_doubles[4] - base_doubles[5]);
            
            double d_expr2 = base_doubles[6] * base_doubles[7] +
                           base_doubles[8] / base_doubles[9] -
                           base_doubles[10] * base_doubles[11];
            
            /* More vector operations */
            float32x4_t vec_mul = vec1 * 2.0f;
            float32x4_t vec_add = vec2 + 1.0f;
            float32x4_t vec_combined = vec_mul * vec_add;
            float vec_reduce = vector_reduce_sum(vec_combined);
            
            /* Nested expressions to increase operand count */
            double nested1 = d_expr1 + d_expr2 * 3.14159;
            double nested2 = (base_doubles[12] + base_doubles[13]) *
                           (base_doubles[14] - base_doubles[15]);
            double nested3 = base_doubles[16] / base_doubles[17] +
                           base_doubles[18] * base_doubles[19];
            
            /* Call helper with 10 arguments */
            double helper_result = helper_10_args(
                d_expr1, d_expr2,
                nested1, nested2, nested3,
                (double)vec_reduce,
                base_doubles[0], base_doubles[1],
                base_doubles[2], base_doubles[3]
            );
            
            result_accumulator += (long double)helper_result;
        }
        
        /* Modify values for next iteration to prevent optimization */
        for (int i = 0; i < 10; i++) {
            base_long_doubles[i] += 0.1L;
        }
        for (int i = 0; i < 20; i++) {
            base_doubles[i] += 0.01;
        }
        
        vec1 = vec1 * 1.1f;
        vec2 = vec2 * 0.9f;
        
#if HAS_COMPLEX
        c1 = c1 * (1.0L + 0.1L * I);
        c2 = c2 * (1.0L - 0.1L * I);
#endif
    }
    
    /* Final checksum computation */
    checksum = (double)result_accumulator;
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %.15f\n", checksum);
    
    /* Additional complex expression at the end to ensure expansion */
    {
        double final_expr = 
            base_doubles[0] * base_doubles[1] +
            base_doubles[2] / base_doubles[3] -
            base_doubles[4] * base_doubles[5] +
            base_doubles[6] / base_doubles[7] -
            base_doubles[8] * base_doubles[9] +
            base_doubles[10] / base_doubles[11];
        
        printf("Final expression value: %.15f\n", final_expr);
    }
    
    return 0;
}
