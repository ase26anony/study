#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

/* Vector type fallbacks */
#if defined(__VECTOR_TYPES_SUPPORTED__) || defined(__GNUC__)
    #define HAS_VECTORS 1
    typedef int32_t int32x4_t __attribute__((vector_size(16)));
    typedef float float32x4_t __attribute__((vector_size(16)));
    typedef double float64x2_t __attribute__((vector_size(16)));
#else
    #define HAS_VECTORS 0
    typedef struct { int32_t v[4]; } int32x4_t;
    typedef struct { float v[4]; } float32x4_t;
    typedef struct { double v[2]; } float64x2_t;
#endif

/* DFP fallback using unions */
#if !HAS_DFP
typedef union {
    uint64_t u64;
    uint64_t u128[2];
} decimal64_fb;

typedef union {
    uint64_t u128[2];
} decimal128_fb;

#define DECIMAL64(x) ((decimal64_fb){.u64 = (x)})
#define DECIMAL128(x, y) ((decimal128_fb){.u128 = {(x), (y)}})
#endif

/* Helper function with 11 arguments - marked noinline to prevent simplification */
static __attribute__((noinline)) 
long double complex helper_11_args(
    long double a1, long double a2, long double a3,
    long double a4, long double a5, long double a6,
    long double a7, long double a8, long double a9,
    long double a10, long double a11)
{
    /* Complex operation that may expand to many RTL operands */
    long double complex c1 = a1 + a2 * I;
    long double complex c2 = a3 + a4 * I;
    long double complex c3 = a5 + a6 * I;
    
    /* This complex expression should create many intermediate values */
    return (c1 * c2 + c3) / (a7 + a8 * I) * (a9 + a10 * I) + a11;
}

/* Another helper with 10 mixed-type arguments */
static __attribute__((noinline))
double helper_10_mixed(
    double d1, double d2, double d3, double d4, double d5,
    float f1, float f2, float f3, float f4, float f5)
{
    /* Mixed precision operations that may require conversions */
    return (d1 * d2 + d3 / d4 - d5) * 
           ((double)f1 + (double)f2 - (double)f3 * (double)f4 / (double)f5);
}

/* Vector reduction helper */
static __attribute__((noinline))
int32_t vector_reduce_sum(int32x4_t v)
{
#if HAS_VECTORS
    /* This horizontal reduction may expand to multiple operations */
    int32_t sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += v[i];
    }
    return sum;
#else
    return v.v[0] + v.v[1] + v.v[2] + v.v[3];
#endif
}

/* Main computation with many operands */
static volatile double global_accumulator[10];

int main(int argc, char *argv[])
{
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize various types of variables */
    double d1 = 1.1 + (rand() % 100) * 0.01;
    double d2 = 2.2 + (rand() % 100) * 0.01;
    double d3 = 3.3 + (rand() % 100) * 0.01;
    double d4 = 4.4 + (rand() % 100) * 0.01;
    double d5 = 5.5 + (rand() % 100) * 0.01;
    
    float f1 = 1.1f + (rand() % 100) * 0.01f;
    float f2 = 2.2f + (rand() % 100) * 0.01f;
    float f3 = 3.3f + (rand() % 100) * 0.01f;
    float f4 = 4.4f + (rand() % 100) * 0.01f;
    float f5 = 5.5f + (rand() % 100) * 0.01f;
    
#if HAS_COMPLEX
    long double complex c1 = d1 + d2 * I;
    long double complex c2 = d3 + d4 * I;
    long double complex c3 = d5 + f1 * I;
#endif
    
#if HAS_VECTORS
    int32x4_t vec1 = {seed, seed + 1, seed + 2, seed + 3};
    float32x4_t vec2 = {f1, f2, f3, f4};
#else
    int32x4_t vec1 = {{seed, seed + 1, seed + 2, seed + 3}};
    float32x4_t vec2 = {{f1, f2, f3, f4}};
#endif
    
    /* DFP operations if available */
#if HAS_DFP
    _Decimal64 dd1 = 1.1dd;
    _Decimal64 dd2 = 2.2dd;
    _Decimal128 dl1 = 3.3dl;
    _Decimal128 dl2 = 4.4dl;
    
    /* Complex DFP expression - may expand to many operands */
    volatile _Decimal128 dfp_result;
    dfp_result = dl1 * dl2 + (_Decimal128)dd1 / (_Decimal128)dd2;
#else
    /* Fallback: simulate with doubles */
    decimal128_fb dfp_result = DECIMAL128(0, 0);
    double dfp_temp = d1 * d2 + d3 / d4;
#endif
    
    /* Main loop with conditional execution */
    volatile int condition = seed % 2;
    double checksum = 0.0;
    
    for (int iter = 0; iter < 3; iter++) {
        double result = 0.0;
        
        /* Conditional block to prevent constant folding */
        if (condition || iter > 0) {
            /* Call helper with 11 arguments - complex expressions as args */
#if HAS_COMPLEX
            long double complex complex_result = helper_11_args(
                d1 + iter, d2 - iter, d3 * iter, d4 / (iter + 1), d5,
                f1, f2, f3, f4, f5,
                (double)(seed % 100) * 0.01);
            
            /* Extract real part */
            result += creal(complex_result);
            
            /* More complex arithmetic */
            long double complex ctemp = c1 * c2;
            ctemp = ctemp / c3;
            ctemp = cpow(ctemp, 2.0);
            result += creal(ctemp);
#endif
            
            /* Call helper with 10 mixed arguments */
            double mixed_result = helper_10_mixed(
                d1, d2, d3, d4, d5,
                f1, f2, f3, f4, f5);
            result += mixed_result;
            
            /* Vector reduction */
            int32_t vec_sum = vector_reduce_sum(vec1);
            result += vec_sum;
            
            /* Complex vector-like operation */
            for (int i = 0; i < 2; i++) {
                /* Small loop to repeat without being optimized away */
                result += d1 * d2 - d3 / d4 + d5;
#if HAS_VECTORS
                result += vec2[i % 4];
#endif
            }
            
            /* Multi-step DFP-like computation */
            double temp1 = d1 * d2 + d3;
            double temp2 = d4 / d5 - f1;
            double temp3 = f2 * f3 + f4;
            double temp4 = temp1 / temp2 * temp3;
            double temp5 = temp4 - f5 + d1;
            result += temp5;
            
            /* Another complex expression chain */
            result += (((d1 + d2) * (d3 - d4) / d5) + 
                      ((f1 * f2) - (f3 / f4) + f5)) *
                     (seed % 10 + 1);
        }
        
        /* Store result to prevent elimination */
        global_accumulator[iter % 10] = result;
        checksum += result;
        
        /* Modify condition to vary execution path */
        condition = !condition;
    }
    
    /* Additional stress: nested function calls with many args */
    {
        /* Create a deep expression tree */
        double nested = 
            helper_10_mixed(
                d1, d2, d3, d4, d5,
                f1, f2, f3, f4, f5) +
            helper_10_mixed(
                d2, d3, d4, d5, d1,
                f2, f3, f4, f5, f1);
        
        checksum += nested;
        
        /* Multi-dimensional array access pattern */
        double matrix[3][3] = {
            {d1, d2, d3},
            {d4, d5, f1},
            {f2, f3, f4}
        };
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                checksum += matrix[i][j] * (i + 1) / (j + 1);
            }
        }
    }
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
