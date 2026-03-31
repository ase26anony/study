/* fixed-point-test.c
 * Tests GCC's fixed-point arithmetic range calculations
 * Compile with: gcc -O3 -std=c23 -fdump-tree-original -fdump-tree-optimized fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Force compile-time evaluation with constexpr */
#define EVAL_CONST(expr) __builtin_constant_p(expr) ? (expr) : (expr)

/* Struct with mixed fixed-point types to test initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 0.999999k, 0.999999lk},
    {0.5hr, -0.5r, 0.5ur, -0.5k, -0.5lk},
    {0.0hr, 0.0r, 0.0ur, 0.0k, 0.0lk}
};

/* Function to trigger range checks through conversions */
static inline int check_range(_Accum val) {
    /* This conversion should trigger range checking */
    return (int)val;
}

int main(void) {
    /* Declare variables with extreme values */
    const unsigned _Fract max_uf = 0.999999r;
    const unsigned _Fract min_uf = 0.0r;
    const signed _Fract max_sf = 0.999999r;
    const signed _Fract min_sf = -1.0r;
    
    /* Saturated types that will trigger boundary checks */
    unsigned _Sat _Fract usat1 = 0.999999ur;
    unsigned _Sat _Fract usat2 = 0.999999ur;
    signed _Sat _Accum sata1 = 0.999999k;
    signed _Sat _Accum sata2 = -0.999999k;
    
    /* Accum types for wider range */
    _Accum a1 = 0.5k;
    _Accum a2 = -0.5k;
    long _Accum la1 = 0.999999lk;
    long _Accum la2 = -0.999999lk;
    
    /* Force compile-time evaluation of extreme expressions */
#if 1
    /* These should trigger max_r/max_s/min_r/min_s calculations */
    const unsigned _Fract test_uf = EVAL_CONST(0.999999r + 0.000001r);
    const signed _Fract test_sf = EVAL_CONST(-1.0r - 0.000001r);
    const _Accum test_acc = EVAL_CONST(0.999999k * 2.0k);
#endif
    
    /* Perform arithmetic that may overflow */
    usat1 = usat1 + 0.5ur;  /* Should saturate for unsigned _Sat _Fract */
    usat2 = usat2 - 0.5ur;  /* Should not underflow */
    sata1 = sata1 + 0.5k;   /* Should saturate for signed _Sat _Accum */
    sata2 = sata2 - 0.5k;   /* Should saturate negative */
    
    /* Loop with fixed iterations for unrolling */
    volatile int result = 0;
    for (int i = 0; i < 3; i++) {
        /* Use conditional assignments based on fixed-point comparisons */
        _Accum temp;
        if (a1 > 0.25k) {
            temp = a1 * init_data[i].la;
        } else {
            temp = a2 * init_data[i].la;
        }
        
        /* Cast to integer - triggers conversion with range check */
        result += check_range(temp);
        
        /* More complex expression with mixing types */
        if (usat1 > 0.75ur) {
            result += (int)(usat1 * 100.0r);
        }
    }
    
    /* Test conversions at boundaries */
    int int_from_uf = (int)max_uf;      /* Should be 0 */
    int int_from_sf = (int)max_sf;      /* Should be 0 */
    float float_from_uf = (float)max_uf; /* Should be ~1.0 */
    float float_from_sf = (float)min_sf; /* Should be ~-1.0 */
    
    /* Use volatile to prevent dead code elimination */
    volatile int v1 = int_from_uf;
    volatile int v2 = int_from_sf;
    volatile float v3 = float_from_uf;
    volatile float v4 = float_from_sf;
    volatile unsigned _Sat _Fract v5 = usat1;
    volatile signed _Sat _Accum v6 = sata1;
    
    /* Print to prevent optimization */
    printf("Results: %d %d %f %f\n", v1, v2, v3, v4);
    
    /* Additional boundary tests with shifts (simulated via multiplication) */
    short _Fract sf1 = 0.999999hr;
    short _Fract sf2 = -0.999999hr;
    
    /* These multiplications may trigger the uncovered comparison logic */
    sf1 = sf1 * 2.0hr;  /* Overflow for short _Fract */
    sf2 = sf2 * 2.0hr;  /* Underflow for short _Fract */
    
    /* Final mixed expression that uses all types */
    long _Accum final_val = la1 + (_Accum)sf1 + (_Accum)usat1 + la2;
    
    /* Convert to integer with explicit cast */
    int final_int = (int)final_val;
    printf("Final: %d\n", final_int);
    
    return 0;
}
