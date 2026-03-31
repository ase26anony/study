/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int iter = 100;
volatile int seed = 1;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed short _Fract test_fract_range(int base) {
    /* Use various fixed-point types and constants */
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    unsigned short _Fract uf1 = 0.5ur;
    
    /* Introduce uncertainty through volatile arithmetic */
    signed short _Fract sf3 = (signed short _Fract)(base) * 0.01r;
    
    /* Arithmetic that could saturate */
    sf1 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    uf1 = uf1 + 0.75ur;  /* 0.5 + 0.75 = 1.25 -> may saturate to max */
    
    /* Critical comparisons with constants */
    signed short _Fract result = 0.0r;
    
    if (sf1 > 0.0r) {
        result = result + 0.1r;
    }
    
    if (sf2 < -0.1r) {
        result = result - 0.05r;
    }
    
    /* Comparison that may trigger the specific uncovered condition */
    if (sf3 == 0.0r) {
        result = 0.0r;
    }
    
    /* More comparisons with non-trivial constants */
    if (uf1 > 0.9ur) {
        result = (signed short _Fract)uf1 * 0.5r;
    }
    
    return result;
}

__attribute__((noinline, noipa))
signed _Accum test_accum_saturation(int counter) {
    signed _Accum sa1 = 0.5rk;
    signed _Accum sa2 = -0.125rk;
    
    /* Create value with uncertainty */
    signed _Accum sa3 = (signed _Accum)counter * 0.01rk;
    
    /* Multiplication that could overflow */
    sa1 = sa1 * sa1;  /* 0.25 */
    sa2 = sa2 * 2.0rk;  /* -0.25 */
    
    /* Explicit cast between types with different scaling */
    signed short _Fract sf_from_acc = (signed short _Fract)sa3;
    
    /* Comparisons that may trigger range analysis */
    signed _Accum result = 0.0rk;
    
    if (sa1 > 0.3rk) {
        result = result + 0.1rk;
    }
    
    if (sa2 < -0.2rk) {
        result = result - 0.05rk;
    }
    
    /* Complex comparison chain */
    if (sa3 == 0.0rk || sa3 > 1.0rk) {
        result = 1.0rk;
    }
    
    /* Comparison after cast */
    if ((signed _Fract)sa1 < 0.3r) {
        result = result * 0.5rk;
    }
    
    return result;
}

__attribute__((noinline, noipa))
unsigned _Accum test_unsigned_accum(int value) {
    unsigned _Accum ua1 = 0.1urk;
    unsigned _Accum ua2 = 0.9urk;
    
    /* Arithmetic near saturation bounds */
    ua1 = ua1 + 0.8urk;  /* 0.9 */
    ua2 = ua2 + 0.2urk;  /* 1.1 -> may saturate for unsigned */
    
    /* Comparisons with constants that have specific binary representations */
    unsigned _Accum result = 0.0urk;
    
    if (ua1 > 0.5urk) {
        result = result + 0.25urk;
    }
    
    if (ua2 == 1.0urk) {
        result = 1.0urk;
    }
    
    /* This comparison structure may trigger the specific uncovered code */
    if (ua1 > 0.8urk && ua2 < 1.2urk) {
        result = (ua1 + ua2) * 0.5urk;
    }
    
    return result;
}

__attribute__((noinline, noipa))
void test_mixed_types(int counter) {
    /* Mix different fixed-point types in calculations */
    signed _Accum sa = (signed _Accum)counter * 0.01rk;
    unsigned short _Fract uf = 0.75ur;
    signed short _Fract sf = -0.5r;
    
    /* Cross-type operations with explicit casts */
    sa = sa + (signed _Accum)uf;
    sf = sf * (signed short _Fract)(sa * 0.1rk);
    
    /* Multiple comparisons in sequence */
    if (sa > 0.5rk) {
        uf = uf + 0.1ur;
    }
    
    if (sf < -0.25r) {
        sa = sa - 0.1rk;
    }
    
    /* Complex condition similar to uncovered code pattern */
    if (sa > 0.0rk && (uf == 0.8ur || sf < 0.0r)) {
        volatile int dummy = 1;
        (void)dummy;  /* Prevent elimination */
    }
}

int main() {
    signed short _Fract total_fract = 0.0r;
    signed _Accum total_accum = 0.0rk;
    unsigned _Accum total_uaccum = 0.0urk;
    
    /* Loop with volatile control to prevent constant propagation */
    for (int i = 0; i < iter; i++) {
        /* Vary the input to create different range scenarios */
        int val = seed + i;
        
        /* Call all test functions */
        total_fract = total_fract + test_fract_range(val);
        total_accum = total_accum + test_accum_saturation(val);
        total_uaccum = total_uaccum + test_unsigned_accum(val);
        test_mixed_types(val);
        
        /* Modify seed to create varying patterns */
        seed = seed * 1103515245 + 12345;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: fract=%r, accum=%rk, uaccum=%urk\n", 
           total_fract, total_accum, total_uaccum);
    
    return 0;
}
