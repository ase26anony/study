/* Test program for fixed-point range calculation coverage in fixed-value.cc */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-value-test.c */

#include <stdio.h>
#include <stdint.h>

/* Force compile-time evaluation */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Accum ua;
    signed long _Accum sla;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum ssata;
};

/* Array with fixed-point initializers */
static const struct FixedPointData init_data[] = {
    /* Push unsigned fract to maximum */
    { .usf = 0.999999ur,
      .sf = -0.5r,
      .ua = 255.99609375uk,
      .sla = -32768.9999847412109375lk,
      .usatf = 0.999999ur,
      .ssata = 127.99609375k },
    
    /* Push signed fract to extremes */
    { .usf = 0.0ur,
      .sf = -0.999999r,
      .ua = 0.0uk,
      .sla = 32767.9999847412109375lk,
      .usatf = 0.0ur,
      .ssata = -128.0k },
    
    /* Middle values that will overflow when manipulated */
    { .usf = 0.75ur,
      .sf = 0.25r,
      .ua = 128.0uk,
      .sla = 16384.0lk,
      .usatf = 0.5ur,
      .ssata = 64.0k }
};

/* Function to trigger range calculations through conversions */
static int_fast32_t convert_fixed_to_int(unsigned _Accum val) {
    /* This conversion should trigger range checking */
    return (int_fast32_t)val;
}

/* Function using ternary with constant condition */
static unsigned short _Fract select_fract(unsigned _Sat _Fract a, 
                                          unsigned _Sat _Fract b,
                                          int choice) {
    /* Compiler must evaluate this at compile-time when inputs are constants */
    return EVAL_CONST(choice > 0 ? a + b : a - b);
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* Test 1: Direct boundary value initialization */
    printf("Test 1: Boundary values\n");
    
    /* These should trigger max/min range calculations */
    const unsigned _Fract max_uf = 0.999999ur;
    const unsigned _Fract min_uf = 0.0ur;
    const signed _Fract max_sf = 0.999999r;
    const signed _Fract min_sf = -1.0r;
    const unsigned _Accum max_ua = 255.99609375uk;  /* UQ8.8 max */
    const unsigned _Accum min_ua = 0.0uk;
    const signed _Accum max_sa = 127.99609375k;     /* Q7.8 max */
    const signed _Accum min_sa = -128.0k;
    
    /* Force use of these constants */
    result += (int)(max_uf * 1000);
    result += (int)(min_sf * 1000);
    
    /* Test 2: Saturation arithmetic that should overflow/underflow */
    printf("Test 2: Saturation arithmetic\n");
    
    unsigned _Sat _Fract sat1 = 0.999999ur;
    unsigned _Sat _Fract sat2 = 0.5ur;
    signed _Sat _Accum sat3 = 127.99609375k;
    signed _Sat _Accum sat4 = 1.0k;
    
    /* These additions should saturate */
    unsigned _Sat _Fract sat_sum = sat1 + sat2;      /* Should saturate to max */
    signed _Sat _Accum sat_sum2 = sat3 + sat4;       /* Should saturate to max */
    
    /* These subtractions should underflow */
    unsigned _Sat _Fract sat_diff = min_uf - sat2;   /* Should saturate to min */
    signed _Sat _Accum sat_diff2 = min_sa - sat4;    /* Should saturate to min */
    
    result += (int)(sat_sum * 1000);
    result += (int)(sat_sum2);
    
    /* Test 3: Compile-time constant folding with ternary */
    printf("Test 3: Constant folding\n");
    
    /* These should be evaluated at compile-time */
    static const unsigned short _Fract const_fract = 
        select_fract(0.999999ur, 0.5ur, 1);
    
    static const signed long _Accum const_accum = 
        EVAL_CONST(32767.9999847412109375lk > 0.0lk ? 
                   32767.9999847412109375lk : -32768.9999847412109375lk);
    
    result += (int)(const_fract * 1000);
    result += (int)const_accum;
    
    /* Test 4: Loop with fixed-point operations */
    printf("Test 4: Loop operations\n");
    
    unsigned _Accum accum = 0.0uk;
    signed _Fract fract = 0.5r;
    
    for (int i = 0; i < 4; i++) {
        /* Operations that might overflow */
        accum = accum + 64.0uk;  /* Will overflow on last iteration for UQ8.8 */
        fract = fract * 0.999999r;
        
        /* Conditional based on fixed-point comparison */
        if (accum > 128.0uk) {
            fract = -fract;
        }
        
        /* Convert to integer (triggers range check) */
        result += convert_fixed_to_int(accum);
        result += (int)(fract * 1000);
    }
    
    /* Test 5: Mixed-type expressions and conversions */
    printf("Test 5: Mixed-type conversions\n");
    
    /* Convert from different fixed-point types */
    float from_fract = (float)max_sf;
    double from_accum = (double)max_ua;
    int from_sat = (int)sat_sum2;
    
    /* Convert to fixed-point from other types */
    unsigned _Fract to_fract = (unsigned _Fract)0.999999;
    signed _Accum to_accum = (signed _Accum)127.999;
    
    result += (int)(from_fract * 1000);
    result += (int)from_accum;
    result += from_sat;
    result += (int)(to_fract * 1000);
    result += (int)to_accum;
    
    /* Test 6: Array indexing with fixed-point derived index */
    printf("Test 6: Array indexing\n");
    
    /* Use fixed-point to compute array index */
    unsigned _Fract index_fract = 0.75ur;
    int array[4] = {10, 20, 30, 40};
    
    /* Convert fixed-point to index (0-3) */
    int idx = (int)(index_fract * 4);
    if (idx >= 0 && idx < 4) {
        result += array[idx];
    }
    
    /* Test 7: Struct operations */
    printf("Test 7: Struct operations\n");
    
    struct FixedPointData data = init_data[0];
    
    /* Perform operations on struct members */
    data.usatf = data.usatf + 0.25ur;  /* Should saturate */
    data.ssata = data.ssata * 2.0k;    /* Should saturate */
    
    /* Mixed-type operation within struct */
    data.ua = (unsigned _Accum)((float)data.sf * 256.0f);
    
    result += (int)(data.usatf * 1000);
    result += (int)data.ssata;
    result += (int)data.ua;
    
    /* Test 8: Shift operations (using integer-like behavior) */
    printf("Test 8: Shift-like operations\n");
    
    /* Simulate left shift by multiplication */
    unsigned _Accum shifted = 1.0uk;
    for (int i = 0; i < 8; i++) {
        shifted = shifted * 2.0uk;  /* Like left shift */
    }
    /* shifted should now be 256.0, which overflows UQ8.8 */
    
    result += (int)shifted;
    
    /* Final output to prevent optimization */
    printf("Final result: %d\n", result);
    
    return 0;
}

/* Compile-time conditional blocks */
#if defined(__OPTIMIZE__)
/* This block only exists when optimizing */
static const unsigned _Sat _Fract compile_time_sat = 
    0.999999ur + 0.999999ur;  /* Definitely saturates */

/* Array sized by fixed-point computation */
static char fixed_size_array[(int)(0.999999ur * 100) + 1];
#endif

/* Additional test with __builtin_constant_p */
static unsigned _Fract test_builtin(void) {
    unsigned _Fract x = 0.999999ur;
    unsigned _Fract y = 0.5ur;
    
    if (__builtin_constant_p(x + y)) {
        /* Compiler knows this is constant */
        return x + y;
    }
    return x;
}
