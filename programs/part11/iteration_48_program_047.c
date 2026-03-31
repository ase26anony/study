/* Test program targeting uncovered lines 264-277 in fixed-value.cc */
/* Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and IPA */
__attribute__((noinline, noipa))
int test_saturating_accum_comparisons(volatile int seed) {
    /* Use various fixed-point types */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    signed short _Fract sf1, sf2;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sa1 = 0.1rk;          /* Will have precise fractional representation */
    sf1 = -0.125r;        /* Power of two fraction */
    uf1 = 0.5r;           /* Common fraction */
    
    /* Introduce uncertainty through volatile seed */
    sa2 = (signed _Accum)seed * 0.25rk;
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.0625rk;
    
    /* Arithmetic that could overflow/saturate */
    sa1 = sa1 + sa2;      /* May overflow signed _Accum range */
    ua1 = ua1 * ua1;      /* May overflow unsigned _Accum range */
    
    /* Critical comparisons - these should trigger range analysis */
    int result = 0;
    
    /* Comparison 1: Direct comparison with constant */
    if (sa1 > 0.75rk) {
        result |= 1;
    }
    
    /* Comparison 2: Equality check */
    if (ua1 == 0.0rk) {
        result |= 2;
    }
    
    /* Comparison 3: Complex comparison chain */
    sf2 = sf1 + 0.0625r;
    if (sf2 < -0.0625r) {
        result |= 4;
    }
    
    /* Cast between types with different scaling */
    signed _Fract cast_result = (signed _Fract)sa1;
    if (cast_result > 0.1r) {
        result |= 8;
    }
    
    /* Additional arithmetic that may saturate */
    signed _Accum sa3 = sa1 * sa2;
    if (sa3 < -0.5rk || sa3 > 0.5rk) {
        result |= 16;
    }
    
    /* Mixed-type comparison */
    if ((signed short _Fract)ua1 < sf1) {
        result |= 32;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_fract_range_boundaries(volatile int iter) {
    signed long _Fract slf1, slf2;
    unsigned _Fract uf1, uf2;
    
    /* Initialize with values near boundaries */
    slf1 = 0.999999r;     /* Near maximum */
    slf2 = -0.999999r;    /* Near minimum */
    uf1 = 0.000001r;      /* Near zero */
    uf2 = 0.999999r;      /* Near maximum */
    
    /* Operations that could trigger saturation logic */
    for (int i = 0; i < iter; i++) {
        /* These may overflow depending on values */
        slf1 = slf1 + 0.000001r;
        slf2 = slf2 - 0.000001r;
        uf1 = uf1 * 1.1r;
        uf2 = uf2 / 0.9r;
        
        /* Comparisons that should use range analysis */
        if (slf1 > 0.5r) {
            global_result += i;
        }
        if (slf2 == -0.5r) {
            global_result -= i;
        }
        if (uf1 < 0.25r) {
            global_result ^= i;
        }
        if (uf2 >= 0.75r) {
            global_result |= i;
        }
    }
    
    /* Final boundary checks */
    int result = 0;
    if (slf1 >= 1.0r || slf1 <= -1.0r) {
        result |= 1;
    }
    if (uf2 <= 0.0r || uf2 >= 1.0r) {
        result |= 2;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_mixed_width_comparisons(volatile int seed) {
    /* Mix different fixed-point types and widths */
    signed _Accum sa;
    unsigned short _Accum usa;
    signed long _Fract slf;
    unsigned _Fract uf;
    
    /* Initialize with seed-dependent values */
    sa = (signed _Accum)(seed - 50) * 0.02rk;
    usa = (unsigned short _Accum)(seed * 2) * 0.01rk;
    slf = (signed long _Fract)seed * 0.001r;
    uf = (unsigned _Fract)(seed % 100) * 0.01r;
    
    /* Arithmetic operations */
    sa = sa * sa;
    usa = usa + 0.5rk;
    slf = slf - 0.1r;
    uf = uf / 0.3r;
    
    /* Multiple comparisons - should trigger the uncovered condition */
    int result = 0;
    
    /* Chain of comparisons similar to the uncovered code */
    if (sa > 0.8rk) {
        result |= 1;
    } else if (sa == 0.8rk && usa > 0.4rk) {
        result |= 2;
    }
    
    if (slf < -0.3r) {
        result |= 4;
    } else if (slf == -0.3r && uf < 0.6r) {
        result |= 8;
    }
    
    /* Cross-type comparisons after casting */
    if ((signed _Fract)sa > (signed _Fract)slf) {
        result |= 16;
    }
    
    if ((unsigned short _Fract)usa == (unsigned short _Fract)uf) {
        result |= 32;
    }
    
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int total_result = 0;
    
    /* Call test functions in a loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        total_result ^= test_saturating_accum_comparisons(seed + i);
        total_result += test_fract_range_boundaries(i % 10 + 1);
        total_result |= test_mixed_width_comparisons(seed + i * 3);
        
        /* Modify seed to create varying values */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Result: %d (global side effect: %d)\n", total_result, global_result);
    
    return total_result != 0 ? 0 : 1;
}
