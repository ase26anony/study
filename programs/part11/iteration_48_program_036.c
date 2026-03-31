/* fixed-point-test.c */
#include <stdio.h>

/* Prevent constant folding and inlining */
volatile int iter = 100;
volatile int seed = 1;

/* Global side effect to prevent dead code elimination */
static int global_result = 0;

/* Helper function 1: Focus on _Fract types with comparisons */
__attribute__((noinline, noipa))
int test_fract_operations(int base) {
    /* Use various _Fract types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    signed _Fract f1, f2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Non-power-of-two fraction */
    sf2 = -0.125r;   /* Exact power-of-two fraction */
    uf1 = 0.5r;      /* Exact 0.5 */
    uf2 = 0.75r;     /* 0.75 = 0.5 + 0.25 */
    f1 = -0.333r;    /* Approximate 1/3 */
    
    /* Introduce uncertainty using volatile input */
    sf3 = (signed short _Fract)((base & 0xFF) / 256.0);
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;           /* 0.1 - 0.125 = -0.025 */
    uf1 = uf1 + uf2;           /* 0.5 + 0.75 = 1.25 (may saturate for unsigned short _Fract) */
    f1 = f1 * f1;              /* (-0.333)^2 = ~0.111 */
    
    /* Critical comparisons - these should trigger range analysis */
    int result = 0;
    
    /* Comparison against constants */
    if (sf1 > 0.0r) {
        result += 1;
    }
    if (sf2 < -0.1r) {
        result += 2;
    }
    if (uf1 == 1.0r) {         /* May trigger saturation bound checks */
        result += 4;
    }
    if (uf2 >= 0.5r) {
        result += 8;
    }
    
    /* Comparison between variables */
    if (sf3 > sf1) {
        result += 16;
    }
    if (f1 < 0.2r) {
        result += 32;
    }
    
    /* Cast and compare - may trigger different range analysis paths */
    if ((signed _Fract)sf3 < f1) {
        result += 64;
    }
    
    return result;
}

/* Helper function 2: Focus on _Accum types with saturation contexts */
__attribute__((noinline, noipa))
int test_accum_operations(int base) {
    /* Use various _Accum types */
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1, ua2;
    long _Accum la1;
    
    /* Initialize with mixed constants and volatile-dependent values */
    sa1 = 0.1k;                /* Non-trivial fractional part */
    sa2 = -0.5k;               /* Exact -0.5 */
    ua1 = 0.75k;               /* 0.75 */
    
    /* Create value with uncertainty - prevents constant propagation */
    sa3 = (signed _Accum)(base % 100) / 100.0k;
    ua2 = (unsigned _Accum)((base * 3) % 200) / 200.0k;
    la1 = (long _Accum)(base) * 0.01lk;
    
    /* Arithmetic operations that could overflow */
    sa1 = sa1 * sa3;           /* Multiplication may overflow */
    sa2 = sa2 + sa1;           /* Addition may cross zero */
    ua1 = ua1 + ua2;           /* Unsigned addition may saturate */
    la1 = la1 * la1;           /* Squaring may overflow */
    
    /* Critical comparisons for _Accum types */
    int result = 0;
    
    /* Comparisons that should trigger the specific uncovered condition */
    if (sa1 > 0.5k) {          /* May trigger a_high.sgt(max_r) check */
        result += 1;
    }
    if (sa2 == -0.25k) {       /* Equality comparison */
        result += 2;
    }
    if (ua1 < 1.0k) {          /* Unsigned comparison */
        result += 4;
    }
    if (la1 >= 0.0lk) {        /* Long accum comparison */
        result += 8;
    }
    
    /* Complex comparison chain */
    if (sa3 > 0.0k && sa3 < 1.0k) {
        result += 16;
    }
    
    /* Cast between different fixed-point types and compare */
    if ((signed _Accum)ua2 > sa1) {
        result += 32;
    }
    
    return result;
}

/* Helper function 3: Mixed types with explicit saturation casts */
__attribute__((noinline, noipa))
int test_saturation_operations(int base) {
    signed short _Fract sf;
    unsigned _Accum ua;
    signed _Accum sa;
    
    /* Initialize with values near boundaries */
    sf = 0.9r;                 /* Near maximum for signed short _Fract */
    ua = 0.99k;                /* Near 1.0 */
    sa = -0.99k;               /* Near -1.0 */
    
    /* Operations designed to trigger saturation logic */
    sf = sf + 0.2r;            /* Should saturate to max for signed short _Fract */
    ua = ua * 2.0k;            /* Should saturate to max for unsigned _Accum */
    sa = sa - 0.1k;            /* Should saturate to min for signed _Accum */
    
    /* Comparisons after saturation - critical for uncovered lines */
    int result = 0;
    
    if (sf == 0.999r) {        /* Max value comparison */
        result += 1;
    }
    if (ua > 1.5k) {           /* Post-saturation comparison */
        result += 2;
    }
    if (sa < -0.5k) {          /* Min value comparison */
        result += 4;
    }
    
    /* Cross-type comparisons */
    if ((signed _Fract)sa < sf) {
        result += 8;
    }
    
    return result;
}

/* Main function with loop to force dynamic analysis */
int main() {
    int total = 0;
    
    /* Loop to prevent constant propagation and force range analysis */
    for (int i = 0; i < iter; i++) {
        /* Vary the input to create different value ranges */
        int input = seed + i * 3;
        
        /* Call all test functions */
        total += test_fract_operations(input);
        total += test_accum_operations(input);
        total += test_saturation_operations(input);
        
        /* Use volatile to prevent loop unrolling */
        asm volatile("" : "+r" (total));
    }
    
    /* Use the result to prevent dead code elimination */
    global_result = total;
    
    printf("Result: %d\n", total);
    return total % 256;
}
