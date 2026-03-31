/* fixed-point-coverage.c
 * Targets uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-point-coverage.c -o fixed-point-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Force compile-time evaluation with constexpr */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types to test initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 255.999999k, 32767.999999lk},
    {0.5hr, -0.5r, 0.5ur, 127.999999k, -32768.0lk},
    {0.0hr, 0.0r, 0.0ur, 0.0k, 0.0lk}
};

/* Function to trigger range checks through conversions */
static int_fast32_t convert_to_int(long _Accum value) {
    /* This conversion should trigger range checking */
    return (int_fast32_t)value;
}

/* Function using ternary operator with constant folding */
static unsigned _Sat _Fract saturate_multiply(unsigned _Fract a, unsigned _Fract b) {
    /* Force constant folding with ternary */
    const unsigned _Fract threshold = 0.75r;
    return (a > threshold && b > threshold) ? 1.0ur : (a * b);
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Fixed-point types with boundary values */
    const unsigned short _Fract max_usf = 0.999999hr;  /* Max unsigned short fract */
    const signed _Fract min_sf = -1.0r;               /* Min signed fract */
    const unsigned _Sat _Fract max_usat = 1.0ur;      /* Max saturated */
    const signed _Sat _Accum min_sat = -128.0k;       /* Min signed sat accum */
    const long _Accum max_la = 32767.999999lk;        /* Near max long accum */
    
    /* 2. Arithmetic that may overflow */
    unsigned _Sat _Fract sat_result = max_usat;
    signed _Sat _Accum sat_acc = min_sat;
    
    /* Loop with fixed iteration for unrolling */
    for (int i = 0; i < 3; ++i) {
        /* Operations that trigger range calculations */
        sat_result += init_data[i].usatf;  /* May overflow to 1.0ur */
        sat_acc -= init_data[i].sata;      /* May underflow */
        
        /* Conditional based on fixed-point comparison */
        if (init_data[i].la > 0.0lk) {
            /* Multiplication near limits */
            long _Accum temp = init_data[i].la * 1.0001lk;
            result += convert_to_int(temp);
        }
        
        /* Ternary forcing constant evaluation */
        unsigned _Fract uf1 = (i == 0) ? 0.999r : 0.5r;
        unsigned _Fract uf2 = (i == 1) ? 0.999r : 0.5r;
        unsigned _Sat _Fract sat_mul = saturate_multiply(uf1, uf2);
        result += (int)(sat_mul * 1000.0r);
    }
    
    /* 3. Explicit conversions at boundaries */
    int int_from_max = (int)max_la;                    /* Conversion at max */
    int int_from_min = (int)(-32768.0lk);              /* Conversion at min */
    float float_from_fract = (float)max_usf;           /* Float conversion */
    
    /* 4. Shift operations (simulated via multiplication) */
    const signed _Accum shift_test = 64.0k;
    signed _Accum shifted = shift_test * 4.0k;         /* 256 - may overflow */
    
    /* 5. Compile-time conditional with __builtin_constant_p */
#if defined(__GNUC__)
    if (__builtin_constant_p(max_usf)) {
        /* Array indexing with fixed-point derived index */
        int idx = (int)(max_usf * 3.0r);
        if (idx >= 0 && idx < 3) {
            result += convert_to_int(init_data[idx].la);
        }
    }
#endif
    
    /* 6. Mixed expressions with different fixed-point types */
    long _Accum mixed = max_la + (long _Accum)max_usf - (long _Accum)min_sf;
    
    /* 7. Saturation boundary tests */
    unsigned _Sat _Fract sat_boundary = 0.999999ur;
    sat_boundary += 0.000001ur;  /* Should saturate to 1.0ur */
    
    signed _Sat _Accum sat_acc_boundary = -127.999999k;
    sat_acc_boundary -= 1.0k;    /* Should saturate to -128.0k */
    
    /* Use results to prevent optimization */
    result += int_from_max + int_from_min + (int)float_from_fract;
    result += (int)shifted + (int)mixed;
    result += (int)(sat_boundary * 1000.0r) + (int)sat_acc_boundary;
    
    printf("Result: %d\n", result);
    
    return 0;
}

/* Additional compile-time tests */
#ifdef __GNUC__
/* Force evaluation during compilation */
static const unsigned _Fract compile_time_fract = 
    EVAL_CONST(0.999999r * 0.999999r);  /* Near 1.0 */

/* Struct with designated initializers */
static const struct FixedPointData ct_data = {
    .usf = EVAL_CONST(0.999999hr),
    .sf = EVAL_CONST(-0.999999r),
    .usatf = EVAL_CONST(0.999999ur + 0.000001ur),  /* Should saturate */
    .sata = EVAL_CONST(-128.0k * 1.001k),          /* Should saturate */
    .la = EVAL_CONST(32767.999999lk)
};
#endif
