/* Test program for fixed-point range calculation coverage in fixed-value.cc */
/* Compile with: gcc -O2 -std=c23 -fdump-tree-original -fdump-tree-optimized -o fixed_test fixed_test.c */

#include <stdio.h>

/* Force compile-time evaluation with constexpr-style constructs */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Accum ua;
    signed long _Accum sla;
    _Sat signed _Fract ssf;
    _Sat unsigned _Accum sua;
};

/* Array initialization with fixed-point constants at boundaries */
static const struct FixedPointData fp_array[] = {
    /* Test maximum representable values */
    { .usf = 0.999999hr,    /* Max unsigned short fract */
      .sf = 0.999999r,      /* Max signed fract */
      .ua = 65535.999984uk, /* Max unsigned accum (16.16) */
      .sla = 32767.999999999lk, /* Max signed long accum (32.32) */
      .ssf = 0.999999r,
      .sua = 65535.999984uk },
    
    /* Test minimum representable values */
    { .usf = 0.000000hr,    /* Min unsigned */
      .sf = -1.000000r,     /* Min signed fract */
      .ua = 0.000000uk,
      .sla = -32768.000000000lk, /* Min signed long accum */
      .ssf = -1.000000r,
      .sua = 0.000000uk },
    
    /* Test near-boundary values */
    { .usf = 0.5hr,
      .sf = -0.5r,
      .ua = 32767.5uk,
      .sla = 16383.5lk,
      .ssf = 0.75r,
      .sua = 49151.5uk }
};

/* Function to trigger range calculations through conversions */
static int convert_and_check(unsigned _Accum val) {
    /* These conversions will invoke range checking */
    int as_int = (int)val;
    float as_float = (float)val;
    signed _Accum as_signed = (signed _Accum)val;
    
    /* Use results to prevent optimization */
    volatile int dummy = as_int;
    (void)as_float;
    (void)as_signed;
    
    return dummy;
}

int main(void) {
    /* Declare fixed-point variables with boundary values */
    
    /* Unsigned fract at maximum */
    const unsigned _Fract max_uf = 0.999999r;
    
    /* Signed fract at minimum */
    const signed _Fract min_sf = -1.000000r;
    
    /* Accum types with various precisions */
    const unsigned _Accum max_ua = 65535.999984uk;  /* 16.16 format max */
    const signed _Accum min_sa = -32768.000000k;    /* 16.16 format min */
    
    /* Long accum with extreme values */
    const signed long _Accum max_sla = 32767.999999999lk;
    const signed long _Accum min_sla = -32768.000000000lk;
    
    /* Saturated types that will trigger saturation logic */
    _Sat unsigned _Fract sat_uf = 0.5r;
    _Sat signed _Accum sat_sa = 0.0k;
    
    /* Force compile-time evaluation of boundary comparisons */
    /* This should trigger the uncovered range checking code */
#if 1
    /* Test 1: Operations that might overflow */
    const unsigned _Accum test1 = EVAL_CONST(max_ua * 2.0uk);
    const signed _Accum test2 = EVAL_CONST(min_sa * 2.0k);
    
    /* Test 2: Boundary comparisons */
    const int cmp1 = EVAL_CONST(max_uf > 0.999998r);
    const int cmp2 = EVAL_CONST(min_sf < -0.999999r);
    
    /* Test 3: Mixed-type expressions */
    const signed _Accum test3 = EVAL_CONST((signed _Accum)max_uf + min_sa);
#endif
    
    /* Loop with fixed iterations to allow unrolling and constant propagation */
    volatile unsigned _Accum accum_result = 0.0uk;
    volatile signed _Fract fract_result = 0.0r;
    
    for (int i = 0; i < 4; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        if (max_uf > 0.5r) {
            sat_uf = sat_uf + 0.5r;  /* This will saturate for unsigned fract */
        }
        
        if (min_sf < -0.5r) {
            sat_sa = sat_sa - 10000.0k;  /* This will underflow for signed accum */
        }
        
        /* Mix different fixed-point types */
        accum_result = accum_result + (_Accum)max_uf;
        fract_result = fract_result * 0.9r;
        
        /* Array indexing with fixed-point derived index */
        int idx = (int)(max_uf * 3);
        if (idx < 3) {
            /* Access array - triggers range checks for conversions */
            convert_and_check(fp_array[idx].ua);
        }
    }
    
    /* Saturation arithmetic that definitely overflows/underflows */
    _Sat unsigned _Fract saturated_max = 0.999999r;
    saturated_max = saturated_max + 0.5r;  /* Should saturate to 0.999999r */
    
    _Sat signed _Accum saturated_min = -32768.000000k;
    saturated_min = saturated_min - 1.0k;  /* Should saturate to -32768.000000k */
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("  sat_uf = %f\n", (double)sat_uf);
    printf("  sat_sa = %f\n", (double)sat_sa);
    printf("  accum_result = %f\n", (double)accum_result);
    printf("  fract_result = %f\n", (double)fract_result);
    printf("  saturated_max = %f\n", (double)saturated_max);
    printf("  saturated_min = %f\n", (double)saturated_min);
    
    /* Use compile-time evaluated constants */
    volatile int use_cmp1 = cmp1;
    volatile int use_cmp2 = cmp2;
    (void)use_cmp1;
    (void)use_cmp2;
    
    /* Test shift operations which may trigger range calculations */
    /* Note: Fixed-point shifts are implemented as multiplications by powers of 2 */
    const signed _Accum shifted = min_sa * 4.0k;  /* Equivalent to left shift by 2 */
    volatile double show_shifted = (double)shifted;
    (void)show_shifted;
    
    /* Complex expression with multiple conversions */
    long long int big_int = (long long int)(max_sla * min_sla);
    volatile long long int show_big = big_int;
    (void)show_big;
    
    return 0;
}
