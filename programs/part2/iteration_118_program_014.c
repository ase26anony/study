/* Test program to exercise fixed-point range calculation logic in fixed-value.cc */
#include <stdio.h>

/* Force compile-time evaluation with static const */
static const unsigned short _Fract max_uf = 0.999999ur;
static const signed _Fract min_sf = -0.999999r;
static const unsigned _Sat _Fract sat_uf = 0.5ur;
static const signed long _Accum max_sla = 0.999999999999999999lr;
static const unsigned _Sat long _Accum sat_ula = 0.75ulr;

/* Struct with mixed fixed-point types */
struct fixed_mix {
    signed short _Fract f1;
    unsigned _Accum a1;
    signed _Sat _Fract sf1;
};

/* Array initialized with fixed-point constants */
static const signed _Fract farray[] = {
    0.0r, 0.25r, -0.5r, 0.75r, -0.999999r
};

/* Compile-time conditional using __builtin_constant_p */
#define CHECK_FIXED_CONSTANT(expr) \
    (__builtin_constant_p(expr) ? (expr) : 0)

/* Function to force range calculations through complex expressions */
static signed _Sat _Fract test_range_calc(signed _Fract base) {
    /* Operations that may overflow/underflow */
    signed _Sat _Fract result = base;
    
    /* Multiplication near limits */
    result = result * 1.999998r;  /* Close to 2.0, may overflow */
    
    /* Conditional based on fixed-point comparison */
    if (result > 0.5r) {
        result = result - 0.75r;
    } else {
        result = result + 0.25r;
    }
    
    return result;
}

/* Another test with accum types */
static unsigned _Sat _Accum test_accum_range(unsigned _Accum val) {
    unsigned _Sat _Accum res = val;
    
    /* Shift-like operation through multiplication */
    res = res * 4.0uk;  /* Effectively left shift by 2 */
    
    /* Check against bounds */
    if (res < 0.25uk) {
        res = res + 0.5uk;
    }
    
    return res;
}

int main(void) {
    volatile int output = 0;  /* Prevent dead code elimination */
    
    /* 1. Basic fixed-point declarations with extreme values */
    const signed _Fract sf_min = -0.999999r;
    const signed _Fract sf_max = 0.999999r;
    const unsigned _Fract uf_max = 0.999999ur;
    
    /* 2. Saturation types with operations that should saturate */
    unsigned _Sat _Fract us1 = 0.8ur;
    unsigned _Sat _Fract us2 = 0.9ur;
    unsigned _Sat _Fract us_sum = us1 + us2;  /* Should saturate to max */
    
    signed _Sat _Fract ss1 = 0.8r;
    signed _Sat _Fract ss2 = -0.9r;
    signed _Sat _Fract ss_prod = ss1 * ss2;  /* Should stay within bounds */
    
    /* 3. Accum types with larger ranges */
    signed long _Accum sla1 = -0.999999999999999999lr;
    signed long _Accum sla2 = 0.5lr;
    signed long _Accum sla_sum = sla1 + sla2;
    
    /* 4. Fixed loop with constant propagation opportunities */
    for (int i = 0; i < 3; i++) {
        /* Use ternary with constant condition to force compile-time eval */
        signed _Fract loop_val = (i == 0) ? 0.25r : 
                                 (i == 1) ? -0.75r : 0.999999r;
        
        /* Operation that might trigger range checking */
        signed _Fract scaled = loop_val * 2.0r;  /* May exceed range */
        
        /* Convert to integer (triggers conversion checks) */
        int as_int = (int)(scaled * 1000);  /* Scale and convert */
        output += as_int;
    }
    
    /* 5. Complex compile-time expression using macro */
    const signed _Fract ct_expr = CHECK_FIXED_CONSTANT(
        (0.999999r * 0.999999r) + (-0.5r * -0.75r)
    );
    
    /* 6. Struct initialization with mixed types */
    struct fixed_mix fm = {
        .f1 = 0.99999hr,
        .a1 = 0.123456789uk,
        .sf1 = test_range_calc(0.8r)
    };
    
    /* 7. Array indexing with fixed-point derived index */
    int idx = (int)(ct_expr * 4);  /* Convert fixed-point to index */
    if (idx >= 0 && idx < 5) {
        output += (int)(farray[idx] * 1000);
    }
    
    /* 8. Explicit overflow attempts */
    unsigned _Sat _Fract overflow_test = 0.999999ur;
    overflow_test = overflow_test + 0.000001ur;  /* Should saturate */
    
    signed _Sat _Fract underflow_test = -0.999999r;
    underflow_test = underflow_test - 0.000001r;  /* Should saturate */
    
    /* 9. Test accum range function */
    unsigned _Sat _Accum accum_test = test_accum_range(0.999999uk);
    
    /* 10. Mixed-type conversions */
    float as_float = (float)sf_max;
    double as_double = (double)sla_sum;
    long as_long = (long)(sla1 * 1000000);
    
    /* Prevent optimization by using volatile */
    volatile int dummy = output;
    dummy += (int)(us_sum * 1000);
    dummy += (int)(ss_prod * 1000);
    dummy += (int)(accum_test * 1000);
    
    printf("Test completed (output suppressed for clarity)\n");
    return 0;
}
