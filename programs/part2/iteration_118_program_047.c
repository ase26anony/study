/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Force compile-time evaluation with constexpr-style usage */
#define EVAL_CONST(expr) __builtin_constant_p(expr) ? (expr) : (expr)

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    long _Accum la;
    unsigned long long _Sat _Accum ullsata;
};

int main(void) {
    /* ========== 1. Fixed-point types with extreme values ========== */
    /* Push against representable limits */
    static const unsigned _Fract uf_max = 0.999999r;  /* Max unsigned fract */
    static const signed _Fract sf_min = -0.999999r;   /* Min signed fract */
    static const long _Accum la_max = 0.999999999999999999lk;  /* Max long accum */
    static const unsigned _Sat _Fract usf_sat = 0.999999r;
    
    /* ========== 2. Force constant folding with ternary operators ========== */
    /* These force the compiler to evaluate at compile time */
    const signed _Fract sf_test = EVAL_CONST(sf_min * 0.999999r);
    const unsigned _Sat _Fract usf_test = EVAL_CONST(usf_sat + 0.000001r);
    
    /* ========== 3. Saturation arithmetic that will overflow ========== */
    unsigned _Sat _Fract us1 = 0.75r;
    unsigned _Sat _Fract us2 = 0.75r;
    unsigned _Sat _Fract us_sum = us1 + us2;  /* Should saturate to 0.999999r */
    
    signed _Sat _Fract ss1 = 0.8r;
    signed _Sat _Fract ss2 = -0.9r;
    signed _Sat _Fract ss_diff = ss1 - ss2;  /* Should approach max */
    
    /* ========== 4. Mixed conversions ========== */
    /* Fixed-point to integer with extreme values */
    int int_from_uf = (int)(uf_max * 256);  /* Scale up to trigger range checks */
    float float_from_sf = (float)sf_min;
    
    /* Integer to fixed-point at boundaries */
    const long _Accum la_from_int = (long _Accum)0x7FFFFFFF;
    
    /* ========== 5. Array initialization with fixed-point ========== */
    static const signed _Fract sf_array[4] = {
        -0.999999r,  /* Min */
        0.0r,
        0.5r,
        0.999999r    /* Max */
    };
    
    /* ========== 6. Struct initialization ========== */
    struct FixedPointData fpd = {
        .usf = 0.9999hr,  /* short fract max */
        .sf = -0.5r,
        .usatf = 0.999999r,
        .la = 255.999999999lk,
        .ullsata = 18446744073709551615.999999999999999999ulk
    };
    
    /* ========== 7. Loop with fixed-point operations ========== */
    volatile signed _Fract result = 0.0r;  /* volatile to prevent elimination */
    
    for (int i = 0; i < 3; i++) {  /* Small, fixed count for unrolling */
        signed _Fract x = sf_array[i];
        
        /* Conditional that depends on fixed-point comparison */
        if (x > 0.0r) {
            x = x * 1.1r;  /* May overflow for large values */
        } else {
            x = x * (-1.1r);  /* May underflow for negative values */
        }
        
        /* Left shift simulation through multiplication */
        x = x * 2.0r;  /* Equivalent to left shift for fixed-point */
        
        /* Cast to integer for potential overflow in conversion */
        int idx = (int)(x * 128);
        
        /* Use idx to prevent dead code (modulo to keep in bounds) */
        result = sf_array[idx & 3];
    }
    
    /* ========== 8. Compile-time conditional blocks ========== */
#if 1  /* Always true, but forces evaluation */
    /* This expression requires range analysis */
    const unsigned _Fract compile_time_uf = 
        (uf_max > 0.5r) ? 0.999999r : 0.0r;
    
    /* Array indexing with fixed-point derived index */
    int array_index = (int)(compile_time_uf * 4);
    /* Use modulo to ensure bounds */
    signed _Fract final_value = sf_array[array_index & 3];
#endif
    
    /* ========== 9. Additional overflow scenarios ========== */
    /* Multiplication near limits */
    long _Accum la1 = la_max;
    long _Accum la2 = 1.000000000000000001lk;  /* Slightly > 1 */
    long _Accum la_prod = la1 * la2;  /* Should trigger overflow check */
    
    /* Accumulate to saturation */
    unsigned _Sat _Fract accum = 0.0r;
    for (int i = 0; i < 10; i++) {
        accum += 0.2r;  /* Will saturate */
    }
    
    /* ========== 10. Print results (or assign to volatile) ========== */
    volatile int output = 0;
    output += (int)(result * 1000);
    output += (int)(us_sum * 1000);
    output += (int)(ss_diff * 1000);
    output += int_from_uf;
    output += (int)(float_from_sf * 1000);
    output += (int)(final_value * 1000);
    output += (int)(la_prod);
    output += (int)(accum * 1000);
    
    printf("Output: %d\n", output);
    
    return 0;
}

/* ========== 11. Additional compile-time tests ========== */
/* These force evaluation during compilation */
#ifdef __OPTIMIZE__
static const unsigned _Fract global_uf = 
    __builtin_constant_p(0.999999r + 0.000001r) ? 0.999999r : 0.0r;

static const signed _Fract global_sf_min = -1.0r;  /* Out of range, should clamp */
static const signed _Fract global_sf_max = 1.0r;   /* Out of range, should clamp */
#endif
