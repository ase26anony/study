/* Test program for GCC fixed-point arithmetic range calculations */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-test.c -o fixed-test */

#include <stdio.h>

/* Fixed-point type definitions covering various modes */
static const unsigned short _Fract usf_max = 0.999999ur;
static const unsigned short _Fract usf_min = 0.0ur;
static const _Sat unsigned short _Fract usf_sat = 0.5ur;

static const short _Fract sf_max = 0.999999r;
static const short _Fract sf_min = -1.0r;
static const _Sat short _Fract sf_sat = -0.5r;

static const unsigned _Fract uf_max = 0.9999999ur;
static const unsigned _Fract uf_min = 0.0ur;
static const _Sat unsigned _Fract uf_sat = 0.75ur;

static const _Fract f_max = 0.9999999r;
static const _Fract f_min = -1.0r;
static const _Sat _Fract f_sat = 0.25r;

static const long _Accum la_max = 9223372036854775.807lk;
static const long _Accum la_min = -9223372036854775.808lk;
static const _Sat long _Accum la_sat = 1000.0lk;

static const unsigned long _Accum ula_max = 18446744073709551.615ulk;
static const unsigned long _Accum ula_min = 0.0ulk;
static const _Sat unsigned long _Accum ula_sat = 5000.0ulk;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointStruct {
    unsigned short _Fract usf;
    short _Fract sf;
    unsigned _Fract uf;
    _Fract f;
    long _Accum la;
    unsigned long _Accum ula;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointStruct fp_array[] = {
    {0.5ur, -0.5r, 0.25ur, 0.75r, 1000.0lk, 5000.0ulk},
    {0.999999ur, 0.999999r, 0.9999999ur, 0.9999999r, 
     9223372036854775.807lk, 18446744073709551.615ulk},
    {0.0ur, -1.0r, 0.0ur, -1.0r, 
     -9223372036854775.808lk, 0.0ulk}
};

/* Compile-time conditional using __builtin_constant_p */
#define CHECK_CONSTANT(expr) \
    (__builtin_constant_p(expr) ? (expr) : 0)

/* Force constant folding with ternary operator */
#define FOLD_CONSTANT(x, y) \
    (__builtin_constant_p(x > y) ? (x + y) : (x - y))

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* Test 1: Direct operations that may trigger range checks */
    unsigned short _Fract usf1 = usf_max;
    usf1 = usf1 * usf_max;  /* May overflow for non-sat types */
    
    short _Fract sf1 = sf_min;
    sf1 = sf1 - sf_max;  /* Underflow check */
    
    /* Test 2: Saturation arithmetic forcing boundary checks */
    _Sat unsigned short _Fract usf_sat_result = usf_sat + usf_max;
    _Sat short _Fract sf_sat_result = sf_sat - sf_max;
    _Sat _Fract f_sat_result = f_sat * f_max;
    
    /* Test 3: Mixed-type conversions triggering range calculations */
    int int_from_fract = (int)(f_max * 100);
    float float_from_accum = (float)la_max;
    long _Accum accum_from_int = (long _Accum)0x7FFFFFFFFFFFFFFF;
    
    /* Test 4: Loop with fixed-point operations (will be unrolled) */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        unsigned _Fract temp_uf;
        if (fp_array[i].uf > 0.5ur) {
            temp_uf = fp_array[i].uf * 0.9ur;
        } else {
            temp_uf = fp_array[i].uf / 0.5ur;
        }
        
        long _Accum temp_la;
        if (fp_array[i].la > 0.0lk) {
            temp_la = fp_array[i].la + la_sat;
        } else {
            temp_la = fp_array[i].la - la_sat;
        }
        
        /* Convert to integer and accumulate */
        result += (int)(temp_uf * 100);
        result += (int)(temp_la / 1000);
    }
    
    /* Test 5: Compile-time constant expressions */
    const unsigned _Fract compile_time_uf = 
        CHECK_CONSTANT(uf_max * uf_max) ? uf_max : uf_min;
    
    const long _Accum compile_time_la = 
        FOLD_CONSTANT(la_max, la_sat);
    
    /* Test 6: Shift-like operations through multiplication */
    unsigned long _Accum ula_shifted = ula_sat * 2.0ulk;  /* Effectively left shift */
    long _Accum la_shifted = la_sat * 4.0lk;  /* May overflow */
    
    /* Test 7: Extreme boundary cases */
    /* These should trigger the uncovered range comparison code */
    const _Sat unsigned long _Accum ula_boundary = 
        ula_max + ula_max;  /* Will saturate at max */
    
    const _Sat long _Accum la_boundary = 
        la_min - la_sat;  /* Will saturate at min */
    
    /* Test 8: Complex expression with multiple conversions */
    double complex_expr = 
        (double)((unsigned _Fract)((int)f_max)) + 
        (double)((long _Accum)((float)la_max)) / 2.0;
    
    /* Prevent optimization from removing computations */
    result += (int)(usf1 * 100);
    result += (int)(sf1 * 100);
    result += (int)(usf_sat_result * 100);
    result += (int)(sf_sat_result * 100);
    result += (int)(f_sat_result * 100);
    result += int_from_fract;
    result += (int)float_from_accum;
    result += (int)accum_from_int;
    result += (int)(compile_time_uf * 100);
    result += (int)(compile_time_la / 1000);
    result += (int)(ula_shifted / 1000);
    result += (int)(la_shifted / 1000);
    result += (int)(ula_boundary / 1000000);
    result += (int)(la_boundary / 1000);
    result += (int)complex_expr;
    
    printf("Result: %d\n", result);
    
    return 0;
}
