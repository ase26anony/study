/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int global_counter = 0;

/* Core fixed-point operations that should trigger range analysis */
__attribute__((noinline, noipa))
static signed long _Fract fixed_range_test_1(volatile int seed) {
    /* Use various fixed-point types to trigger different code paths */
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    unsigned _Accum ua1;
    signed _Accum sa1;
    
    /* Initialize with volatile to prevent constant folding */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.5rk;
    sa1 = (signed _Accum)((seed % 64) - 32) * 0.25rk;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;  /* 0.1 + (-0.125) = -0.025 */
    ua1 = ua1 * ua1;  /* Square - could overflow */
    
    /* Critical comparisons that should trigger the uncovered condition */
    if (sf1 > 0.5r) {
        /* This comparison involves fixed-point constants */
        sa1 = sa1 - 0.125rk;
    }
    
    if (ua1 == 0.0rk) {
        /* Equality comparison with zero */
        sf2 = 0.0r;
    }
    
    /* Cast between types with different scaling */
    signed _Fract sf3 = (signed _Fract)sa1;
    
    /* Another comparison after cast */
    if (sf3 < sf2) {
        ua1 = ua1 / 2rk;
    }
    
    /* Return a value to prevent dead code elimination */
    return (signed long _Fract)(sf1 + (signed _Fract)ua1 + sf3);
}

__attribute__((noinline, noipa))
static unsigned short _Fract fixed_range_test_2(int base) {
    unsigned short _Fract uf1 = 0.75r;
    unsigned _Accum ua2;
    signed _Accum sa2;
    
    /* Create values that might hit saturation bounds */
    ua2 = (unsigned _Accum)(base % 256) * 0.1rk;
    sa2 = (signed _Accum)((base % 128) - 64) * 0.05rk;
    
    /* Operations that could saturate */
    ua2 = ua2 + 0.9rk;  /* Could overflow for unsigned */
    sa2 = sa2 * 10rk;   /* Could overflow signed */
    
    /* Multiple comparisons to trigger range analysis */
    if (ua2 > 0.5rk) {
        uf1 = uf1 - 0.25r;
    }
    
    if (sa2 == 0.0rk) {
        uf1 = 0.0r;
    } else if (sa2 < 0.0rk) {
        uf1 = 1.0r - uf1;
    }
    
    /* Cast with potential saturation */
    unsigned short _Fract uf2 = (unsigned short _Fract)ua2;
    
    /* Comparison involving the cast result */
    if (uf2 > uf1) {
        return uf2;
    }
    
    return uf1;
}

__attribute__((noinline, noipa))
static void fixed_range_test_3(volatile int iter) {
    signed _Accum sa3 = 0.0rk;
    signed long _Fract slf1 = 0.0r;
    
    /* Loop with fixed-point accumulation that could overflow */
    for (int i = 0; i < iter; i++) {
        signed _Accum increment = (signed _Accum)(i % 10) * 0.1rk;
        sa3 = sa3 + increment;
        
        /* Comparison inside loop - value changes each iteration */
        if (sa3 > 0.5rk) {
            slf1 = slf1 + 0.01r;
        } else if (sa3 < -0.5rk) {
            slf1 = slf1 - 0.01r;
        }
        
        /* Cast and compare */
        signed _Fract temp = (signed _Fract)sa3;
        if (temp == 0.0r) {
            global_counter++;
        }
    }
    
    /* Final comparison that might trigger the specific condition */
    if (sa3 > 0.99rk || sa3 < -0.99rk) {
        slf1 = 0.0r;
    }
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    signed long _Fract total_result = 0.0r;
    
    printf("Starting fixed-point range analysis tests...\n");
    
    /* Call test functions in a loop to ensure execution */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Test 1: Signed/unsigned mixed operations */
        signed long _Fract res1 = fixed_range_test_1(seed);
        total_result = total_result + res1;
        
        /* Test 2: Unsigned operations with saturation potential */
        unsigned short _Fract res2 = fixed_range_test_2(seed);
        total_result = total_result + (signed long _Fract)res2;
        
        /* Test 3: Loop-based accumulation */
        fixed_range_test_3(10 + (seed % 10));
        
        /* Prevent everything from being optimized away */
        if (global_counter > 1000) {
            global_counter = 0;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result (as integer): %d\n", (int)(total_result * 1000r));
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
