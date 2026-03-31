/* Compile with: gcc -O3 -std=c23 -fdump-tree-optimized fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Force constant folding with static const */
static const unsigned short _Fract max_uf = 0.999999uhk;
static const signed _Fract min_sf = -0.999999hr;
static const unsigned _Sat long _Accum max_ul_acc = 0.999999999999999ulr;
static const signed _Sat _Accum min_s_acc = -0.9999999k;

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed long _Fract slf;
    unsigned _Sat _Accum usa;
    signed _Sat long _Accum sslk;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData fp_array[] = {
    { .usf = 0.5uhk, .slf = -0.5lr, .usa = 0.75r, .sslk = -0.25lk },
    { .usf = 0.999uhk, .slf = -0.999lr, .usa = 0.9999r, .sslk = -0.9999lk },
    { .usf = 0.0uhk, .slf = 0.0lr, .usa = 0.0r, .sslk = 0.0lk }
};

/* Function to trigger range checks through conversions */
static inline int check_range_and_convert(signed _Accum val) {
    /* This ternary forces constant folding with range checks */
    return (val > 0.5k) ? (int)(val * 2.0k) : (int)(val / 0.5k);
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* Test 1: Direct overflow with saturation types */
    unsigned _Sat _Fract sat_f1 = 0.7r;
    unsigned _Sat _Fract sat_f2 = 0.8r;
    unsigned _Sat _Fract sat_sum = sat_f1 + sat_f2; /* Should saturate to 1.0r */
    
    /* Test 2: Underflow with signed saturation */
    signed _Sat _Fract sat_sf1 = -0.9r;
    signed _Sat _Fract sat_sf2 = -0.8r;
    signed _Sat _Fract sat_diff = sat_sf1 - sat_sf2; /* Should approach -1.0r */
    
    /* Test 3: Multiplication that may overflow */
    unsigned long _Accum ul_acc1 = max_ul_acc;
    unsigned long _Accum ul_acc2 = ul_acc1 * 1.5lk; /* May overflow */
    
    /* Test 4: Complex expression with constant folding */
    const signed _Accum const_expr = 
        (max_uf > 0.5uhk) ? (signed _Accum)(max_uf * 2.0uhk) : 
                           (signed _Accum)(min_sf / 0.25hr);
    
    /* Loop with fixed iterations for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Conditional based on fixed-point comparison */
        if (fp_array[i].usf > 0.75uhk) {
            /* Cast to integer triggers conversion with range check */
            result += (int)(fp_array[i].usf * 100.0uhk);
        } else {
            result += (int)(fp_array[i].slf * -100.0lr);
        }
        
        /* Mixed-type arithmetic */
        signed _Accum temp = (signed _Accum)fp_array[i].usa + 
                            (signed _Accum)fp_array[i].sslk;
        
        /* Use builtin to check constantness */
        if (__builtin_constant_p(fp_array[i].usf)) {
            /* This path should be taken at compile-time */
            result += check_range_and_convert((signed _Accum)fp_array[i].usf);
        }
    }
    
    /* Extreme value tests that should trigger the uncovered bounds checking */
    
    /* Test near maximum representable value */
    unsigned _Fract near_max = 0.9999999r;
    unsigned _Fract max_test = near_max + 0.0000001r;
    
    /* Test near minimum representable value */
    signed _Fract near_min = -0.9999999r;
    signed _Fract min_test = near_min - 0.0000001r;
    
    /* Conversions that require precise range analysis */
    float float_from_fixed = (float)max_ul_acc;
    int int_from_fixed = (int)min_s_acc;
    
    /* Left shift simulation through multiplication */
    signed _Accum shifted = min_s_acc * 4.0k; /* Effectively left shift */
    
    /* Print results to prevent optimization */
    printf("Results: %d %f %d %f\n", 
           result, 
           (float)sat_sum,
           int_from_fixed,
           float_from_fixed);
    
    /* Use all variables to prevent dead code elimination */
    volatile unsigned _Sat _Fract vsat = sat_sum;
    volatile signed _Sat _Fract vsdiff = sat_diff;
    volatile unsigned long _Accum vul = ul_acc2;
    volatile signed _Accum vconst = const_expr;
    volatile unsigned _Fract vmax = max_test;
    volatile signed _Fract vmin = min_test;
    volatile signed _Accum vshift = shifted;
    
    (void)vsat; (void)vsdiff; (void)vul; (void)vconst;
    (void)vmax; (void)vmin; (void)vshift;
    
    return 0;
}

/* Compile-time conditional blocks */
#if defined(__GNUC__)
/* This section tests extreme boundary conditions */
static const signed long _Fract boundary_test = 
    (max_uf == 0.999999uhk) ? -0.999999lr : 0.0lr;

/* Array index using fixed-point conversion */
static const int array_index = (int)(boundary_test * 1000.0lr);
static const char test_array[] = { 'a', 'b', 'c' };
static const char boundary_char = test_array[array_index % 3];
#endif
