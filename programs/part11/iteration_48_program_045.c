/* fixed-point-test.c */
#include <stdio.h>

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
static signed long _Fract process_fract(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract usf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Binary: 0.0001100110011... */
    sf2 = -0.125r;       /* Binary: -0.001 exactly */
    usf1 = 0.5ur;        /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty through volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;     /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf1;     /* Multiplication with varying value */
    
    /* Critical comparisons - similar to uncovered condition */
    if (sf1 > 0.0r) {
        /* This branch may not be taken, but forces comparison */
        sf3 = sf3 + 0.25r;
    }
    
    if (sf3 == -0.0625r) {
        /* Exact comparison with binary-friendly constant */
        sf3 = sf3 * 2.0r;
    }
    
    /* Comparison after type conversion */
    if ((signed _Fract)usf1 < sf2) {
        sf3 = -sf3;
    }
    
    return (signed long _Fract)sf3;
}

__attribute__((noinline, noipa))
static signed _Accum process_accum(volatile int seed) {
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with different scaling */
    sa1 = 0.1k;           /* 0.1 in accum format */
    sa2 = -0.125k;        /* -0.125 */
    
    /* Use seed to create varying values */
    ua1 = (unsigned _Accum)(seed & 0xFFF) * 0.000244140625uk;  /* 1/4096 */
    
    /* Operations that could saturate */
    sa1 = sa1 + sa2;      /* -0.025 */
    ua1 = ua1 * ua1;      /* Square - could overflow */
    
    /* Multiple comparisons to trigger range analysis */
    if (sa1 > 0.5k) {
        ua1 = ua1 + 0.25uk;
    }
    
    if (ua1 == 0.0uk) {
        sa1 = sa1 * 2.0k;
    }
    
    /* Complex comparison chain */
    if (sa1 < -0.5k || sa1 > 0.5k) {
        ua1 = ua1 / 2.0uk;
    }
    
    /* Cast between types with different scaling */
    signed short _Fract temp = (signed short _Fract)sa1;
    if (temp > 0.0r) {
        ua1 = ua1 + 0.0009765625uk;  /* 1/1024 */
    }
    
    return (signed _Accum)ua1 + sa1;
}

__attribute__((noinline, noipa))
static long _Fract mixed_operations(volatile int seed) {
    /* Mix different fixed-point types */
    signed _Accum sa = (signed _Accum)seed * 0.001k;
    unsigned short _Fract usf = 0.75ur;
    signed long _Fract slf = 0.0lr;
    
    /* Chain of operations that could saturate */
    for (int i = 0; i < 3; i++) {
        sa = sa + 0.1k;
        usf = usf * 0.9ur;
        
        /* Comparisons that might trigger the uncovered logic */
        if (sa > 0.5k && usf < 0.5ur) {
            slf = slf + (signed long _Fract)sa * (signed long _Fract)usf;
        }
        
        if (sa == -0.25k || sa == 0.25k) {
            slf = slf - 0.125lr;
        }
    }
    
    /* Final comparison with potential overflow */
    if (slf > 0.999lr || slf < -0.999lr) {
        slf = 0.0lr;
    }
    
    return slf;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    long _Fract total = 0.0lr;
    
    printf("Starting fixed-point saturation tests...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all fixed-point processing functions */
        signed long _Fract f1 = process_fract(seed % 100);
        signed _Accum a1 = process_accum(seed % 1000);
        long _Fract m1 = mixed_operations(seed);
        
        /* Use results to prevent dead code elimination */
        total = total + f1 + (long _Fract)a1 + m1;
        
        /* Additional comparisons in main */
        if (f1 > 0.5lr && a1 < 0.0k) {
            total = total * 0.9lr;
        }
    }
    
    /* Convert to double for output */
    double result = (double)total;
    printf("Final result: %f\n", result);
    
    return (result > 0.0) ? 0 : 1;
}
