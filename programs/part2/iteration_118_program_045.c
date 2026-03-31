/* Target: fixed-value.cc lines 264-277 - range calculation for fixed-point types */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original test_fixed.c */

#include <stdio.h>

/* Force compile-time evaluation with static const */
static const unsigned short _Fract max_uf = 0.9999999ur;
static const signed _Fract min_sf = -0.9999999r;
static const unsigned _Sat _Fract sat_uf = 0.5ur;
static const signed _Sat long _Accum sat_sla = -0.5lk;

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Accum usa;
    signed _Sat long _Accum sla;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData fp_array[] = {
    {0.9999999ur, -0.9999999r, 0.9999999uhr, -0.9999999lk},
    {0.5ur, -0.5r, 0.5uhr, -0.5lk},
    {0.0ur, 0.0r, 0.0uhr, 0.0lk}
};

/* Function to trigger range checks through conversions */
int convert_and_check(unsigned _Fract f) {
    /* This conversion will trigger range checking */
    return (int)(f * 256);
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Basic fixed-point operations that may overflow */
    unsigned _Sat _Fract sat1 = 0.75ur;
    unsigned _Sat _Fract sat2 = 0.75ur;
    unsigned _Sat _Fract sat_sum = sat1 + sat2; /* Should saturate to 0.9999999ur */
    
    /* 2. Multiplication near limits */
    signed _Sat _Accum sa1 = 0.9999999hk;
    signed _Sat _Accum sa2 = 0.9999999hk;
    signed _Sat _Accum sa_prod = sa1 * sa2; /* May trigger overflow check */
    
    /* 3. Loop with constant propagation */
    for (int i = 0; i < 3; i++) {
        /* Use compile-time constant in conditional */
        unsigned _Fract temp = fp_array[i].usf;
        
        /* Force range comparison */
        if (temp > 0.5ur) {
            /* This addition might overflow for max value */
            temp = temp + 0.25ur;
        } else {
            temp = temp * 2.0ur;
        }
        
        /* Convert to integer - triggers range checking */
        result += convert_and_check(temp);
    }
    
    /* 4. Extreme value testing with ternary operator */
    const unsigned _Fract extreme = (max_uf > 0.999ur) ? max_uf : 0.5ur;
    
    /* 5. Shift operations (GCC extension for fixed-point) */
    signed long _Accum sla = 0.5lk;
    /* Simulate left shift through multiplication */
    signed long _Accum shifted = sla * 4.0lk; /* Equivalent to << 2 */
    
    /* 6. Mixed-type expressions */
    signed _Fract mixed = (signed _Fract)sat_sla * 0.5r;
    
    /* 7. Use __builtin_constant_p to force constant folding */
    #if __GNUC__ >= 5
    if (__builtin_constant_p(max_uf)) {
        /* This will be evaluated at compile-time */
        const int as_int = (int)(max_uf * 1000);
        result += as_int;
    }
    #endif
    
    /* 8. Direct overflow attempt */
    unsigned _Sat short _Fract ussf = 0.9999999uhr;
    ussf = ussf + 0.0000001uhr; /* Should saturate */
    
    /* 9. Underflow test */
    signed _Sat _Fract ssf = -0.9999999r;
    ssf = ssf - 0.0000001r; /* Should saturate at -1.0r */
    
    /* 10. Complex expression with multiple bounds checks */
    signed _Accum complex_expr = (sa_prod > 0.5hk) ? 
                                 (sa_prod * 1.5hk) : 
                                 (sa_prod / 2.0hk);
    
    /* Print results to prevent optimization */
    printf("Results: %d\n", result);
    printf("Saturated sum: %f\n", (double)sat_sum);
    printf("Complex expr: %f\n", (double)complex_expr);
    
    return 0;
}

/* Additional compile-time tests */
#ifdef __COMPILE_TIME_TESTS
/* These will only be compiled with specific flags */
static const unsigned _Fract compile_time_array[] = {
    0.0ur, 0.25ur, 0.5ur, 0.75ur, 0.9999999ur
};

/* Force evaluation of all array elements */
static int sum_array(void) {
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += (int)(compile_time_array[i] * 100);
    }
    return total;
}

/* Global with compile-time calculation */
const int global_sum = sum_array();
#endif
