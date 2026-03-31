/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 1;
volatile int iterations = 100;

/* Helper functions marked to prevent optimization */
__attribute__((noinline, noipa))
signed short _Fract test_saturating_comparison(int seed) {
    /* Use constants with non-trivial binary representations */
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    signed short _Fract sf3 = 0.5r;
    signed short _Fract sf4 = 0.75r;
    
    /* Introduce uncertainty through volatile seed */
    signed short _Fract sf_var = (signed short _Fract)(seed * 0.01r);
    
    /* Operations that could saturate */
    sf1 = sf1 + sf2;           /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 + sf_var;        /* Potentially overflow */
    
    /* Critical comparisons that should trigger range analysis */
    if (sf1 > 0.0r) {
        /* This branch may be taken depending on range analysis */
        sf4 = sf4 - 0.1r;
    }
    
    if (sf3 == 0.5r) {
        sf_var = sf_var * 2.0r;  /* Could saturate */
    }
    
    /* Additional comparison with constant */
    if (sf1 < -0.1r) {
        return sf2;
    }
    
    return sf1 + sf3 + sf4;
}

__attribute__((noinline, noipa))
signed _Accum test_accum_range(int seed) {
    signed _Accum acc1 = 0.5rk;
    signed _Accum acc2 = -0.125rk;
    signed _Accum acc3 = 0.999rk;  /* Near upper bound */
    
    /* Variable initialization with potential overflow */
    signed _Accum acc_var = (signed _Accum)(seed) * 0.01rk;
    
    /* Multiplication that could overflow */
    acc1 = acc1 * acc_var;
    
    /* Critical comparisons for accum types */
    if (acc1 > 0.75rk) {
        acc2 = acc2 + 0.1rk;
    }
    
    if (acc3 == 0.999rk) {
        /* This comparison should trigger the uncovered code */
        acc3 = acc3 - 0.001rk;
    }
    
    /* Cast between types with different scaling */
    signed short _Fract sf_cast = (signed short _Fract)acc1;
    if (sf_cast > 0.0r) {
        acc_var = acc_var * 1.5rk;  /* Could saturate */
    }
    
    return acc1 + acc2 + acc3;
}

__attribute__((noinline, noipa))
unsigned _Accum test_unsigned_saturation(int seed) {
    unsigned _Accum uacc1 = 0.1urk;
    unsigned _Accum uacc2 = 0.9urk;  /* Near upper bound for unsigned */
    
    /* Operations that could saturate at 1.0 */
    uacc1 = uacc1 + (unsigned _Accum)(seed * 0.01urk);
    uacc2 = uacc2 * 1.1urk;  /* Should saturate if > 1.0 */
    
    /* Comparisons that should trigger range analysis */
    if (uacc1 > 0.5urk) {
        uacc2 = uacc2 - 0.2urk;
    }
    
    if (uacc2 == 1.0urk) {
        /* This should hit saturation boundary checks */
        return 0.5urk;
    }
    
    return uacc1 + uacc2;
}

__attribute__((noinline, noipa))
void test_mixed_types_comparisons(int seed) {
    signed short _Fract sf = 0.25r;
    signed _Accum acc = 0.25rk;
    unsigned short _Fract usf = 0.5ur;
    unsigned _Accum uacc = 0.5urk;
    
    /* Initialize with volatile-dependent values */
    sf = sf + (signed short _Fract)(seed * 0.001r);
    acc = acc * (signed _Accum)(seed * 0.001rk);
    
    /* Multiple comparisons in sequence */
    if (sf > 0.3r) {
        usf = usf - 0.1ur;
    }
    
    if (acc < 0.2rk) {
        uacc = uacc + 0.3urk;
    }
    
    /* Cross-type comparisons after casting */
    if ((signed _Accum)sf == acc) {
        sf = 0.0r;
    }
    
    /* Comparison chain that might trigger the specific condition */
    if (sf > 0.5r || (sf == 0.5r && usf > 0.5ur)) {
        acc = -0.5rk;
    }
    
    /* Prevent dead code elimination */
    control += (int)(sf * 1000r) + (int)(acc * 1000rk);
}

int main() {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different value ranges */
        volatile int seed = i % 10;
        
        /* Call all test functions to exercise different paths */
        result += (int)(test_saturating_comparison(seed) * 1000r);
        result += (int)(test_accum_range(seed) * 1000rk);
        result += (int)(test_unsigned_saturation(seed) * 1000urk);
        test_mixed_types_comparisons(seed);
    }
    
    printf("Result: %d\n", result + control);
    return 0;
}
