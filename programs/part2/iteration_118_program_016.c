/* Test program for GCC fixed-point arithmetic range calculations */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Force compile-time evaluation */
#define EVAL_CONST(expr) __builtin_constant_p(expr) ? (expr) : (expr)

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
};

int main(void) {
    /* ========== 1. Fixed-point types with extreme values ========== */
    /* These push against representable limits */
    static const unsigned short _Fract max_usf = 0.999999ur;
    static const signed _Fract min_sf = -0.999999r;
    static const unsigned _Sat _Fract usat_max = 0.999999ur;
    static const signed _Sat _Accum sat_min = -0.999999999999999k;
    static const long _Accum near_max = 0.999999999999999999lk;
    
    /* ========== 2. Force constant folding with ternary operators ========== */
    /* The compiler must evaluate these at compile-time */
    const signed _Fract folded1 = EVAL_CONST(max_usf * 0.5r);
    const unsigned _Sat _Fract folded2 = EVAL_CONST(usat_max + 0.1ur);
    const signed _Sat _Accum folded3 = EVAL_CONST(sat_min - 0.1k);
    
    /* ========== 3. Saturation arithmetic that will overflow/underflow ========== */
    unsigned _Sat _Fract sat_result1 = usat_max;
    signed _Sat _Accum sat_result2 = sat_min;
    
    /* These operations should trigger saturation bounds checking */
    for (int i = 0; i < 3; i++) {
        sat_result1 = sat_result1 + 0.5ur;  /* Will saturate at 1.0 */
        sat_result2 = sat_result2 - 0.5k;   /* Will saturate at min value */
    }
    
    /* ========== 4. Mixed-type conversions ========== */
    /* Casts that require range checking */
    int int_from_fract = (int)(max_usf * 256);  /* Scale up to test conversion */
    float float_from_accum = (float)near_max;
    signed _Fract fract_from_int = (signed _Fract)(int_from_fract);
    
    /* ========== 5. Compile-time conditional blocks ========== */
    #if __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    /* Use __builtin_constant_p to ensure compile-time evaluation */
    if (__builtin_constant_p(max_usf > 0.5ur)) {
        /* Array indexing with fixed-point conversion */
        int array[2] = {0, 1};
        int idx = (int)(max_usf * 2);
        /* This forces range analysis for the index conversion */
        volatile int array_access = array[idx & 1];
    }
    #endif
    
    /* ========== 6. Aggregate initializers ========== */
    /* Struct with mixed fixed-point initializers */
    struct FixedPointData data = {
        .usf = 0.75ur,
        .sf = -0.25r,
        .usatf = 0.999ur,
        .sata = -0.5k,
        .la = 0.999999999999999lk
    };
    
    /* Array of fixed-point values */
    signed _Fract fract_array[4] = {
        0.0r,
        0.5r,
        -0.5r,
        -0.999r  /* Near minimum */
    };
    
    /* ========== 7. Complex expressions with potential overflow ========== */
    /* Multiplication that could overflow fixed-point range */
    long _Accum product = near_max * near_max;
    
    /* Left shift simulation through multiplication */
    signed _Accum shifted = sat_min * 2.0k;  /* Effectively left shift */
    
    /* ========== 8. Loop with fixed-point operations ========== */
    signed _Fract accumulator = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        if (accumulator > 0.0r) {
            accumulator = accumulator - fract_array[i];
        } else {
            accumulator = accumulator + fract_array[i];
        }
        
        /* Force potential overflow in the loop */
        if (i == 2) {
            accumulator = accumulator * 2.0r;  /* Could overflow */
        }
    }
    
    /* ========== 9. Prevent dead code elimination ========== */
    /* Cast results to volatile or print to ensure code isn't optimized away */
    volatile unsigned short _Fract vol_usf = max_usf;
    volatile signed _Sat _Accum vol_sata = sat_result2;
    volatile long _Accum vol_la = product;
    volatile int vol_int = int_from_fract;
    volatile float vol_float = float_from_accum;
    volatile signed _Fract vol_accum = accumulator;
    
    /* Also use the struct data */
    volatile struct FixedPointData vol_data = data;
    
    /* Print some results to prevent optimization */
    printf("Results (some may be truncated by saturation):\n");
    printf("  sat_result1 (should be 1.0): %u\n", (unsigned)(sat_result1 * 256));
    printf("  int_from_fract: %d\n", int_from_fract);
    printf("  accumulator: %d\n", (int)(accumulator * 256));
    
    return 0;
}

/* ========== 10. Additional compile-time tests ========== */
/* These force evaluation during compilation */
#if defined(__OPTIMIZE__)
/* Global initializers with complex fixed-point expressions */
static const unsigned _Sat _Fract global_sat = 
    (0.8ur + 0.3ur) * 0.9ur;  /* Will saturate during compilation */

static const signed _Fract global_fract = 
    (-0.9r * 2.0r) < -0.5r ? -0.9r * 2.0r : -0.5r;

/* Array size determined by fixed-point calculation */
char fixed_size_array[(int)(0.999r * 100) + 1];
#endif
