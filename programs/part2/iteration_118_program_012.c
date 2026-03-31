/* Target: fixed-value.cc uncovered lines 264-277 */
/* Compile with: gcc -O3 -std=c23 -fdump-tree-optimized -o fixed_test fixed_test.c */

#include <stdio.h>

/* Fixed-point type declarations covering various combinations */
typedef short _Fract sf;
typedef _Fract f;
typedef long _Fract lf;
typedef short _Accum sa;
typedef _Accum a;
typedef long _Accum la;
typedef unsigned short _Fract usf;
typedef unsigned _Fract uf;
typedef unsigned long _Fract ulf;
typedef unsigned short _Accum usa;
typedef unsigned _Accum ua;
typedef unsigned long _Accum ula;

/* Saturated versions */
typedef _Sat short _Fract ssf;
typedef _Sat _Fract sfx;
typedef _Sat long _Fract slf;
typedef _Sat short _Accum ssa;
typedef _Sat _Accum sax;
typedef _Sat long _Accum sla;
typedef _Sat unsigned short _Fract susf;
typedef _Sat unsigned _Fract suf;
typedef _Sat unsigned long _Fract sulf;
typedef _Sat unsigned short _Accum susa;
typedef _Sat unsigned _Accum sua;
typedef _Sat unsigned long _Accum sula;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointStruct {
    sf short_fract;
    a accum;
    uf unsigned_fract;
    ua unsigned_accum;
    sfx saturated_fract;
    sua saturated_unsigned_accum;
};

/* Array initialized with fixed-point constants */
static const struct FixedPointStruct fp_array[] = {
    {0.999999hr, 255.999999k, 0.999999r, 255.999999uk, 0.999999r, 255.999999uk},
    {-0.999999hr, -255.999999k, 0.0r, 0.0uk, -0.999999r, 0.0uk},
    {0.5hr, 127.5k, 0.5r, 127.5uk, 0.5r, 127.5uk},
    {-0.5hr, -127.5k, 0.75r, 191.25uk, -0.5r, 191.25uk}
};

/* Compile-time constant expressions that force range calculations */
#define FORCE_RANGE_CHECK(expr) \
    __builtin_constant_p(expr) ? (expr) : (expr)

/* Test function that performs operations triggering range calculations */
static void test_fixed_point_range(void) {
    /* Extreme values that push against representable limits */
    const usf max_ushort_fract = 0.999999hr;  /* Almost 1.0 */
    const sf min_short_fract = -0.999999hr;   /* Almost -1.0 */
    const ua max_unsigned_accum = 255.999999uk;  /* Max for 8-bit integer part */
    const a min_accum = -255.999999k;         /* Min for 8-bit integer part */
    
    /* Saturated types with operations that will overflow/underflow */
    sfx sat_fract = 0.75r;
    sua sat_uaccum = 200.0uk;
    
    /* Force constant folding with ternary operator */
    static const int use_max = 1;
    const f folded_fract = use_max ? 0.999999r : 0.5r;
    const a folded_accum = use_max ? 255.999999k : 127.5k;
    
    /* These operations will trigger range checks in constant folder */
    volatile f result1 = FORCE_RANGE_CHECK(folded_fract * 1.000001r);
    volatile a result2 = FORCE_RANGE_CHECK(folded_accum + 0.000001k);
    
    /* Saturation arithmetic that forces bounds checking */
    sat_fract = sat_fract + 0.5r;  /* Will saturate for _Sat _Fract */
    sat_uaccum = sat_uaccum + 100.0uk;  /* Will saturate for _Sat unsigned _Accum */
    
    /* Mixed-type conversions triggering range calculations */
    int int_from_fract = (int)(folded_fract * 256.0r);
    float float_from_accum = (float)folded_accum;
    
    /* Array indexing with fixed-point derived index */
    int idx = (int)(folded_fract * 3.0r);
    if (idx >= 0 && idx < 4) {
        volatile sf array_val = fp_array[idx].short_fract;
        (void)array_val;
    }
    
    /* Prevent dead code elimination */
    (void)result1;
    (void)result2;
    (void)int_from_fract;
    (void)float_from_accum;
}

int main(void) {
    /* Declare and initialize fixed-point variables */
    sf var_sf = 0.75hr;
    f var_f = -0.25r;
    lf var_lf = 0.875lr;
    sa var_sa = 63.75hk;
    a var_a = -127.999999k;
    la var_la = 511.999999lk;
    
    usf var_usf = 0.999999hr;
    uf var_uf = 0.5r;
    ulf var_ulf = 0.999999lr;
    usa var_usa = 127.999999uhk;
    ua var_ua = 255.5uk;
    ula var_ula = 1023.999999ulk;
    
    /* Saturated types */
    ssf var_ssf = 0.5hr;
    sfx var_sfx = -0.75r;
    slf var_slf = 0.999999lr;
    ssa var_ssa = 63.5hk;
    sax var_sax = 200.0k;
    sla var_sla = -511.5lk;
    
    susf var_susf = 0.999999hr;
    suf var_suf = 0.75r;
    sulf var_sulf = 0.5lr;
    susa var_susa = 127.5uhk;
    sua var_sua = 255.999999uk;
    sula var_sula = 1023.5ulk;
    
    /* Fixed iteration loop to allow unrolling and constant propagation */
    for (int i = 0; i < 3; i++) {
        /* Operations that may overflow/underflow */
        var_sf = var_sf * 1.5hr;  /* May overflow for short _Fract */
        var_f = var_f - 0.8r;     /* May underflow */
        var_a = var_a + 128.0k;   /* May overflow */
        var_la = var_la * 2.0lk;  /* May overflow */
        
        var_usf = var_usf + 0.1hr;  /* May saturate for unsigned */
        var_ua = var_ua - 300.0uk;  /* May underflow */
        
        /* Saturated arithmetic - will trigger saturation logic */
        var_sfx = var_sfx + 0.5r;
        var_sax = var_sax * 2.0k;
        var_sua = var_sua + 100.0uk;
        var_sla = var_sla - 600.0lk;
        
        /* Conditional assignments based on fixed-point comparisons */
        if (var_f > 0.0r) {
            var_sf = 0.9hr;
        } else {
            var_sf = -0.9hr;
        }
        
        if (var_ua > 128.0uk) {
            var_uf = 0.8r;
        } else {
            var_uf = 0.2r;
        }
        
        /* Mixed-type operations */
        int int_val = (int)(var_a * 2.0k);
        float float_val = (float)var_lf;
        
        /* Use values to prevent elimination */
        volatile int dummy = int_val + (int)(float_val * 100.0f);
        (void)dummy;
    }
    
    /* Cast results to integers and print to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", 
           (int)(var_sf * 1000.0hr),
           (int)(var_a),
           (int)(var_ua),
           (int)(var_sax));
    
    /* Call function that tests compile-time constant evaluation */
    test_fixed_point_range();
    
    /* Additional complex expression that should trigger range checking */
    constexpr _Accum compile_time_accum = 255.999999k;
    constexpr _Fract compile_time_fract = 0.999999r;
    
    /* This expression should be evaluated at compile time and trigger
       the range checking logic for overflow detection */
    volatile _Accum test_overflow = compile_time_accum * 1.000001k;
    volatile _Fract test_underflow = compile_time_fract - 1.000001r;
    
    (void)test_overflow;
    (void)test_underflow;
    
    return 0;
}
