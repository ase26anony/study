/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    long _Accum la;
    unsigned long long _Sat _Accum ullsata;
};

/* Global constants to force compile-time evaluation */
static const unsigned _Sat _Fract MAX_UFRACT = 0.999999r;
static const signed _Fract MIN_SFRACT = -0.999999r;
static const long _Accum MAX_LACCUM = 0.999999999999999999lk;
static const long _Accum MIN_LACCUM = -0.999999999999999999lk;

/* Function to create complex compile-time expressions */
static inline long _Accum compile_time_expression(long _Accum a, long _Accum b) {
    /* Use ternary with constant condition to force constant folding */
    return (__builtin_constant_p(a) && a > 0.5lk) ? 
           (b * 1.5lk) : 
           (b * 0.5lk);
}

int main(void) {
    /* Test 1: Extreme fixed-point literals */
    unsigned short _Fract usf1 = 0.9999hr;
    signed _Fract sf1 = -0.9999r;
    unsigned _Sat _Fract usatf1 = 0.999999r;
    long _Accum la1 = 0.999999999999999999lk;
    long long _Accum lla1 = -0.9999999999999999999999999llk;
    
    /* Test 2: Aggregate initialization with mixed types */
    struct FixedPointData data = {
        .usf = 0.9999hr,
        .sf = -0.9999r,
        .usatf = 0.999999r,
        .la = 0.999999999999999999lk,
        .ullsata = 0.9999999999999999999999999ullk
    };
    
    /* Test 3: Operations that may overflow */
    volatile unsigned _Sat _Fract result1; /* volatile to prevent elimination */
    volatile signed _Fract result2;
    volatile long _Accum result3;
    
    /* Loop with fixed iteration for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Multiplication near limits - may trigger overflow checks */
        usatf1 = usatf1 * 1.1r;
        sf1 = sf1 * (-1.1r);
        la1 = la1 * 1.000000000000000001lk;
        
        /* Conditional based on fixed-point comparison */
        if (usf1 > 0.5hr) {
            usf1 = usf1 - 0.6hr;
        } else {
            usf1 = usf1 + 0.4hr;
        }
        
        /* Cast to integer - triggers conversion range checks */
        int int_val = (int)(la1 * 1000lk);
        
        /* Use in expression with different types */
        result1 = usatf1;
        result2 = sf1;
        result3 = la1 + (long _Accum)int_val / 1000lk;
    }
    
    /* Test 4: Compile-time constant expressions */
    #if __STDC_VERSION__ >= 202311L
    /* Use preprocessor to create compile-time paths */
    constexpr long _Accum ct_accum = MAX_LACCUM * 0.999999999999999999lk;
    constexpr unsigned _Sat _Fract ct_usatf = MAX_UFRACT + 0.000001r;
    #endif
    
    /* Test 5: Explicit overflow/underflow with saturation */
    unsigned _Sat _Fract sat_test1 = 0.8r;
    unsigned _Sat _Fract sat_test2 = 0.9r;
    unsigned _Sat _Fract sat_sum = sat_test1 + sat_test2; /* Should saturate to 1.0r */
    
    signed _Sat _Fract sat_test3 = -0.8r;
    signed _Sat _Fract sat_test4 = -0.9r;
    signed _Sat _Fract sat_sum2 = sat_test3 + sat_test4; /* Should saturate to -1.0r */
    
    /* Test 6: Shift operations (using integer-like behavior) */
    short _Accum sa = 0.5hk;
    /* Simulate shift by converting to integer and back */
    int sa_int = (int)(sa * 256); /* Assuming 8 fractional bits */
    sa_int = sa_int << 2; /* Left shift */
    sa = (short _Accum)(sa_int / 256.0);
    
    /* Test 7: Complex expression with builtin constant check */
    long _Accum complex_expr = compile_time_expression(0.7lk, 0.3lk);
    
    /* Test 8: Array indexing with fixed-point conversion */
    int array[10] = {0};
    unsigned _Fract index_fract = 0.7r;
    int index = (int)(index_fract * 10);
    if (index >= 0 && index < 10) {
        array[index] = 42;
    }
    
    /* Test 9: Boundary value testing */
    /* These should trigger the range comparison logic */
    const unsigned _Fract boundary1 = 0.999999r;
    const unsigned _Fract boundary2 = 0.000001r;
    const signed _Fract boundary3 = -0.999999r;
    const signed _Fract boundary4 = 0.999999r;
    
    /* Mixed-type arithmetic */
    long _Accum mixed1 = la1 + (long _Accum)sf1;
    unsigned _Sat _Fract mixed2 = usatf1 + (unsigned _Sat _Fract)boundary2;
    
    /* Prevent dead code elimination */
    volatile int output = 0;
    output += (int)(result1 * 1000);
    output += (int)(result2 * 1000);
    output += (int)(result3 * 1000);
    output += (int)(sat_sum * 1000);
    output += (int)(sat_sum2 * 1000);
    output += (int)(complex_expr * 1000);
    output += array[index];
    
    printf("Result: %d\n", output);
    
    return 0;
}
