/* Target: fixed-value.cc uncovered lines 264-277 */
/* Compile with: gcc -O3 -std=c23 -Wno-psabi -fdump-tree-optimized */

#include <stdio.h>

/* Force compile-time evaluation with constexpr */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct mixing different fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
    long unsigned _Accum lua;
};

/* Array initialization with extreme fixed-point values */
static const struct FixedPointData fp_array[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 0.999999999ak, 0.999999999lk, 0.999999999ulk},
    {0.000001hr, -0.000001r, 0.000001ur, -0.999999999ak, -0.999999999lk, 0.000000001ulk},
    {0.5hr, -0.5r, 0.5ur, 0.5ak, 0.5lk, 0.5ulk}
};

/* Function to trigger range calculations through conversions */
static inline long long test_conversion(_Accum a) {
    /* This should trigger range checking during conversion */
    return (long long)(a * 2.0ak);  /* Potential overflow */
}

int main(void) {
    volatile int result = 0;  /* Prevent dead code elimination */
    
    /* 1. Extreme fixed-point literals at representable limits */
    const unsigned short _Fract max_usf = 0.999999hr;
    const signed _Fract min_sf = -0.999999r;
    const unsigned _Sat _Fract max_usatf = 0.999999ur;
    const signed _Sat _Accum max_sata = 0.999999999ak;
    const long _Accum max_la = 0.999999999lk;
    const long unsigned _Accum max_lua = 0.999999999ulk;
    
    /* 2. Compile-time constant folding with ternary operator */
    const _Accum ct_fold = EVAL_CONST(
        (max_usf > 0.5hr) ? 
        (max_sata * 1.5ak) :  /* This will overflow for saturating types */
        (min_sf * 2.0r)
    );
    
    /* 3. Saturation arithmetic that should trigger bounds checking */
    unsigned _Sat _Fract sat_result = 0.0ur;
    for (int i = 0; i < 3; i++) {
        /* Loop will be unrolled, allowing constant propagation */
        sat_result = sat_result + 0.8ur;  /* Will saturate at 1.0 */
        
        /* Mix with array values */
        sat_result = sat_result + fp_array[i].usatf;
    }
    
    /* 4. Complex conversions triggering range analysis */
    float float_from_fixed = (float)max_la;
    int int_from_fixed = (int)max_sata;
    long long ll_from_fixed = (long long)max_lua;
    
    /* 5. Arithmetic that pushes against boundaries */
    _Accum boundary_test = max_sata;
    for (int i = 0; i < 2; i++) {
        /* These operations should trigger overflow detection */
        boundary_test = boundary_test * 1.1ak;
        boundary_test = boundary_test - 0.2ak;
        
        /* Conditional based on fixed-point comparison */
        if (boundary_test > 0.5ak) {
            boundary_test = boundary_test / 0.5ak;
        }
    }
    
    /* 6. Shift-like behavior through multiplication by powers of 2 */
    long _Accum shift_test = 0.25lk;
    shift_test = shift_test * 4.0lk;  /* Effectively left shift */
    shift_test = shift_test * 8.0lk;  /* May overflow */
    
    /* 7. Use __builtin_constant_p to create conditional compilation */
    #if __GNUC__ >= 7
    if (__builtin_constant_p(max_usf)) {
        /* This array index calculation requires range checking */
        int idx = (int)(max_usf * 10.0hr);
        if (idx >= 0 && idx < 3) {
            result = fp_array[idx].usf * 1000;
        }
    }
    #endif
    
    /* 8. More boundary tests with different types */
    const signed _Fract boundary_cases[] = {
        0.999999r,
        -0.999999r,
        0.5r,
        -0.5r,
        0.000001r,
        -0.000001r
    };
    
    for (int i = 0; i < 6; i++) {
        /* Multiplication near boundaries */
        signed _Fract test = boundary_cases[i] * 1.1r;
        
        /* Cast to integer triggers range checking */
        int as_int = (int)(test * 1000000);
        result += as_int;
    }
    
    /* 9. Test with _Sat types at absolute boundaries */
    unsigned _Sat short _Fract us_sat = 0.999999uhr;
    us_sat = us_sat + 0.000001uhr;  /* Should saturate at 1.0 */
    
    signed _Sat long _Accum sl_sat = -0.999999999slk;
    sl_sat = sl_sat - 0.000000001slk;  /* Should saturate at -1.0 */
    
    /* 10. Final conversions that require range analysis */
    result += (int)(sat_result * 100);
    result += (int)(float_from_fixed * 100);
    result += int_from_fixed;
    result += (int)(ll_from_fixed & 0xFFFF);
    result += (int)(boundary_test * 100);
    result += (int)(shift_test * 100);
    result += (int)(us_sat * 100);
    result += (int)(sl_sat * -100);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
