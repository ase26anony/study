/* Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-test.c -o fixed-test */

#include <stdio.h>

/* Fixed-point type declarations covering various combinations */
static const unsigned short _Fract usf_max = 0.999999hr;
static const signed short _Fract ssf_min = -0.999999hr;
static const unsigned _Fract uf_half = 0.5r;
static const signed _Fract sf_quarter = 0.25r;
static const unsigned long _Accum ula_max = 255.999999999999999999ulr;
static const signed long _Accum sla_min = -32768.000000000000000000lr;
static const unsigned long long _Accum ulla_overflow = 9223372036854775807.999999999999999999ullr;
static const signed long long _Accum slla_underflow = -9223372036854775808.000000000000000000llr;

/* Saturation types */
static const unsigned _Sat _Fract usf_sat = 0.999999r;
static const signed _Sat _Accum ssa_sat = 0.999999999999999999r;

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned long _Accum ula;
    signed long long _Accum slla;
    unsigned _Sat _Fract usf_sat;
};

/* Array initialized with fixed-point constants */
static const struct FixedPointData fp_array[] = {
    {0.999999hr, -0.999999r, 255.999999999999999999ulr, -9223372036854775808.000000000000000000llr, 0.999999r},
    {0.5hr, 0.25r, 128.5ulr, 4611686018427387904.000000000000000000llr, 0.5r},
    {0.0hr, 0.0r, 0.0ulr, 0.0llr, 0.0r}
};

/* Force compile-time evaluation with __builtin_constant_p */
#define EVAL_AT_COMPILE_TIME(expr) \
    (__builtin_constant_p(expr) ? (expr) : (expr))

/* Function to trigger range checks through conversions */
static int convert_and_check(unsigned _Fract val) {
    /* These conversions should trigger range checking */
    int as_int = (int)val;
    float as_float = (float)val;
    unsigned _Sat _Fract as_sat = (unsigned _Sat _Fract)val;
    
    /* Complex expression that requires range analysis */
    return as_int + (int)(as_float * 100) + (int)(as_sat * 100);
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Direct operations that may overflow/underflow */
    unsigned _Fract uf1 = usf_max;
    signed _Fract sf1 = ssf_min;
    
    /* Multiplication near limits - may trigger overflow checks */
    uf1 = uf1 * uf_half * 2.0r;  /* Should be ~max */
    sf1 = sf1 * sf_quarter * 4.0r; /* Should be ~min */
    
    /* 2. Saturation arithmetic operations */
    unsigned _Sat _Fract usf_sat_result = usf_sat;
    signed _Sat _Accum ssa_sat_result = ssa_sat;
    
    /* These should trigger saturation logic and range checks */
    usf_sat_result = usf_sat_result + usf_sat_result;  /* Overflow to max */
    ssa_sat_result = ssa_sat_result - ssa_sat_result - ssa_sat_result; /* Underflow */
    
    /* 3. Loop with fixed-point operations (will be unrolled) */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        unsigned _Fract temp;
        if (uf1 > 0.75r) {
            temp = uf1 * 0.9r;
        } else {
            temp = uf1 * 1.1r;
        }
        
        /* Mix with array values */
        temp = temp + fp_array[i].usf;
        
        /* Convert and accumulate result */
        result += convert_and_check(temp);
        
        /* Operations that may trigger the uncovered range comparison */
        signed long long _Accum slla_temp = fp_array[i].slla;
        
        /* Shift-like operation through multiplication */
        slla_temp = slla_temp * 2.0llr;  /* May overflow for extreme values */
        
        /* Ternary with constant condition forces compile-time eval */
        const int use_max = 1;
        unsigned long _Accum ula_temp = use_max ? ula_max : fp_array[i].ula;
        
        /* This multiplication should trigger range checks for max_r/max_s */
        ula_temp = ula_temp * 1.000000000000000001ulr;
    }
    
    /* 4. Compile-time constant expressions using preprocessor */
#if 1
    /* These will be evaluated at compile-time and should trigger the logic */
    const unsigned _Fract compile_time_uf = 0.999999r * 0.999999r;
    const signed long _Accum compile_time_sla = sla_min * 0.999999999999999999lr;
    
    /* Force use in a constant expression */
    const int idx = (int)(compile_time_uf * 100);
    if (idx < sizeof(fp_array)/sizeof(fp_array[0])) {
        result += (int)fp_array[idx].usf;
    }
#endif
    
    /* 5. Explicit overflow/underflow attempts */
    /* These should trigger the uncovered comparison lines directly */
    unsigned _Fract overflow_attempt = 1.0r;  /* Will be clamped */
    signed _Accum underflow_attempt = -1.0r;  /* Will be clamped */
    
    /* Convert extreme values - triggers range checking */
    unsigned long long _Accum ulla_test = ulla_overflow;
    signed long long _Accum slla_test = slla_underflow;
    
    /* Cast to smaller types - requires range checks */
    unsigned _Fract uf_from_ulla = (unsigned _Fract)ulla_test;
    signed _Fract sf_from_slla = (signed _Fract)slla_test;
    
    /* 6. Use EVAL_AT_COMPILE_TIME macro */
    const unsigned _Fract compile_eval = EVAL_AT_COMPILE_TIME(0.999999r * 1.000001r);
    result += (int)(compile_eval * 1000);
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
