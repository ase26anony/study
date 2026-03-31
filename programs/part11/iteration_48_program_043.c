/* fixed-point-test.c */
#include <stdio.h>

/* Prevent constant folding and inlining */
volatile int iter = 100;
volatile int seed = 1;
volatile int control = 0;

/* Global side effects to prevent dead code elimination */
int global_result = 0;

/* Helper function 1: Focus on _Fract types with comparisons */
__attribute__((noinline, noipa))
int test_fract_comparisons(int base) {
    /* Use various _Fract types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    signed _Fract f1, f2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Binary: 0.0001100110011... */
    sf2 = -0.125r;       /* Binary: -0.001 exactly */
    uf1 = 0.5ur;         /* Binary: 0.1 exactly */
    f1 = 0.333r;         /* Binary: 0.0101010101... */
    
    /* Introduce uncertainty using the input parameter */
    sf3 = (signed short _Fract)((base & 0xFF) * 0.00392156862745098r); /* ~1/255 */
    uf2 = (unsigned short _Fract)((base % 100) * 0.01ur);
    f2 = (signed _Fract)(base * 0.001r);
    
    /* Arithmetic that could saturate/overflow */
    sf1 = sf1 + sf2;      /* 0.1 - 0.125 = -0.025 */
    uf1 = uf1 + uf2;      /* Could approach 1.0 */
    f1 = f1 * f2;         /* Multiplication with varying scale */
    
    /* CRITICAL: Comparisons that should trigger range analysis */
    int result = 0;
    
    /* Comparison with constant 0.5 (important for max_r/max_s bounds) */
    if (sf1 > 0.5r) {
        result |= 1;
    }
    
    /* Equality comparison with 0 */
    if (uf1 == 0.0ur) {
        result |= 2;
    }
    
    /* Comparison after type conversion */
    if ((signed _Fract)sf3 < f2) {
        result |= 4;
    }
    
    /* Complex comparison chain similar to uncovered condition */
    signed short _Fract temp = sf1 * 2.0r;
    if (temp > 0.75r || (temp == 0.75r && uf1 > 0.25ur)) {
        result |= 8;
    }
    
    /* Additional comparison to trigger min_s initialization */
    if (f1 < -0.5r) {
        result |= 16;
    }
    
    return result;
}

/* Helper function 2: Focus on _Accum types with saturation */
__attribute__((noinline, noipa))
int test_accum_saturation(int base) {
    /* Use _Accum types which have more bits for range analysis */
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1, ua2;
    
    /* Initialize with constants */
    sa1 = 0.1k;           /* Binary representation matters */
    sa2 = -0.125k;
    ua1 = 0.5uk;
    
    /* Introduce uncertainty */
    sa3 = (signed _Accum)(base * 0.01k);
    ua2 = (unsigned _Accum)((base % 50) * 0.02uk);
    
    /* Arithmetic that could overflow the representable range */
    sa1 = sa1 + sa2 + sa3;    /* Multiple additions */
    ua1 = ua1 * ua2;          /* Multiplication that could overflow */
    
    /* Cast between types with different scaling - may trigger saturation */
    signed short _Fract sf_from_acc = (signed short _Fract)sa1;
    unsigned _Accum ua_from_frac = (unsigned _Accum)ua2;
    
    /* CRITICAL: Comparisons for range analysis */
    int result = 0;
    
    /* Comparison against bounds that might trigger max_r/max_s logic */
    if (sa1 > 0.99k) {
        result |= 32;
    }
    
    /* Equality comparison important for a_high == max_r path */
    if (ua1 == 0.0uk) {
        result |= 64;
    }
    
    /* Complex condition similar to uncovered lines */
    if (sa3 < -0.5k || (sa3 == -0.5k && ua2 > 0.75uk)) {
        result |= 128;
    }
    
    /* Comparison after shift-like operations (simulating alshift) */
    signed _Accum shifted = sa1 * 4.0k;  /* Equivalent to left shift */
    if (shifted > 1.5k) {
        result |= 256;
    }
    
    /* Use results to prevent elimination */
    global_result += (int)(sf_from_acc * 1000r);
    global_result += (int)(ua_from_frac);
    
    return result;
}

/* Helper function 3: Mixed types and explicit saturation context */
__attribute__((noinline, noipa))
int test_mixed_saturation(int base) {
    /* Mix different fixed-point types */
    signed short _Fract sf;
    unsigned _Accum ua;
    signed long _Fract lf;
    
    /* Initialize with values near boundaries */
    sf = (base % 2) ? 0.99r : -0.99r;
    ua = (base % 3) ? 0.01uk : 0.99uk;
    lf = 0.5lr;
    
    /* Operations that could saturate */
    sf = sf + 0.1r;      /* Could saturate for positive case */
    ua = ua * 2.0uk;     /* Could overflow for 0.99 case */
    lf = lf - 0.6lr;     /* Could go negative */
    
    /* Type conversions that require saturation */
    signed _Accum sa_from_sf = (signed _Accum)sf;
    unsigned short _Fract uf_from_ua = (unsigned short _Fract)ua;
    
    /* CRITICAL: Multiple comparisons to trigger range analysis */
    int result = 0;
    
    /* These comparisons should exercise the uncovered condition */
    if (sf > 0.5r) {
        result |= 512;
    }
    
    if (ua < 0.25uk) {
        result |= 1024;
    }
    
    if (lf == -0.1lr || lf == 0.0lr) {
        result |= 2048;
    }
    
    /* Complex condition with AND/OR similar to uncovered code */
    if (sa_from_sf > 0.0k && uf_from_ua > 0.5ur) {
        result |= 4096;
    }
    
    return result;
}

int main() {
    int total = 0;
    
    /* Loop with volatile control to prevent constant propagation */
    for (int i = 0; i < iter; i++) {
        /* Vary the input to create different range possibilities */
        int base_val = seed + i;
        
        /* Call all test functions to exercise different paths */
        total += test_fract_comparisons(base_val);
        total += test_accum_saturation(base_val);
        total += test_mixed_saturation(base_val);
        
        /* Modify control to affect subsequent iterations */
        control = (control + 1) % 7;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (global: %d)\n", total, global_result);
    
    return total != 0 ? 0 : 1;
}
