/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

volatile int global_counter = 0;

/* Core fixed-point operations that should trigger range analysis */
__attribute__((noinline, noipa))
static int fixed_point_operations(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5r;      /* Binary: 0.1 exactly */
    
    /* Use volatile seed to prevent constant folding */
    sa1 = (signed _Accum)seed * 0.5rk;  /* Multiplication that may saturate */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.25rk;
    
    /* Arithmetic that could overflow/saturate */
    sf3 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    sa2 = sa1 * sa1;  /* Square - could overflow */
    
    /* Critical comparisons that should trigger the uncovered condition */
    /* These comparisons involve high/low part splitting in fixed-value.cc */
    if (sf3 > 0.0r) {
        /* This branch should be taken for negative sf3 */
        global_counter += 1;
    }
    
    if (sa1 == 0.0rk) {
        global_counter += 2;
    }
    
    /* Comparison after cast - may trigger saturation bounds calculation */
    if ((signed _Fract)sa2 < sf2) {
        global_counter += 4;
    }
    
    /* More comparisons with different constants */
    if (uf1 >= 0.75r) {
        global_counter += 8;
    }
    
    /* Complex expression that may trigger range analysis */
    signed _Accum sa3 = sa1 + (signed _Accum)sf3 * 2.0rk;
    if (sa3 > 1.0rk || sa3 < -1.0rk) {
        global_counter += 16;
    }
    
    /* Return value based on comparisons to prevent dead code elimination */
    return (sf3 > 0.0r) ? 1 : 
           (sa1 == 0.0rk) ? 2 : 
           (uf1 >= 0.75r) ? 3 : 0;
}

/* Another function with different fixed-point types */
__attribute__((noinline, noipa))
static int more_fixed_point_ops(volatile int iter) {
    long _Fract lf1, lf2;
    unsigned long _Fract ulf1;
    signed long _Accum sla1;
    
    /* Initialize with values that may approach bounds */
    lf1 = 0.9999999r;  /* Close to maximum */
    lf2 = -0.9999999r; /* Close to minimum */
    ulf1 = 0.1r;
    
    /* Use iteration count to create varying values */
    sla1 = (signed long _Accum)iter * 0.01rlk;
    
    /* Operations that may saturate */
    lf1 = lf1 + 0.0000001r;  /* Could saturate to 1.0 */
    lf2 = lf2 - 0.0000001r;  /* Could saturate to -1.0 */
    
    /* Comparisons that should trigger bounds checking */
    if (lf1 > 0.5r) {
        global_counter += 32;
    }
    
    if (lf2 < -0.5r) {
        global_counter += 64;
    }
    
    /* Cast between different fixed-point types */
    if ((signed _Fract)sla1 == 0.0r) {
        global_counter += 128;
    }
    
    return (lf1 > 0.5r) ? 4 : (lf2 < -0.5r) ? 5 : 6;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int result = 0;
    
    printf("Starting fixed-point range analysis test...\n");
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFF;
        
        /* Call fixed-point operations */
        result += fixed_point_operations(seed);
        result += more_fixed_point_ops(i);
        
        /* Additional inline operations */
        {
            /* Use saturating arithmetic explicitly */
            signed _Accum tmp1 = 0.9rk;
            signed _Accum tmp2 = 0.9rk;
            signed _Accum product = tmp1 * tmp2;  /* 0.81 - safe */
            
            /* Comparison that may trigger the specific condition */
            if (product > 0.8rk) {
                global_counter += 256;
            }
            
            /* Test with values near bounds */
            unsigned short _Fract usf = 0.99r;
            usf = usf + 0.02r;  /* Should saturate to 1.0 */
            
            if (usf == 1.0r) {
                global_counter += 512;
            }
        }
    }
    
    printf("Final result: %d (global_counter: %d)\n", result, global_counter);
    printf("Test completed. If coverage was collected, check for execution of target lines.\n");
    
    return result != 0 ? 0 : 1;
}
