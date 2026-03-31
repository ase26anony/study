/* Test program for fixed-point range calculation coverage in fixed-value.cc
 * Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original -fdump-tree-optimized -o fixed_test fixed_test.c
 */

#include <stdio.h>

/* Force compile-time evaluation with constexpr-like behavior */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Fract ssatf;
    _Accum acc;
    long _Sat _Accum lsatacc;
};

/* Array initialized with fixed-point constants at boundaries */
static const struct FixedPointData test_data[] = {
    /* Max values for unsigned fract */
    { .usf = 0.999999ur,
      .sf = 0.999999r,
      .usatf = 0.999999ur,
      .ssatf = 0.999999r,
      .acc = 255.999999k,
      .lsatacc = 32767.999999lk },
    
    /* Min values for signed fract */
    { .usf = 0.0ur,
      .sf = -1.0r,
      .usatf = 0.0ur,
      .ssatf = -1.0r,
      .acc = -256.0k,
      .lsatacc = -32768.0lk },
    
    /* Boundary crossing values */
    { .usf = 0.5ur,
      .sf = -0.5r,
      .usatf = 0.5ur,
      .ssatf = -0.5r,
      .acc = 127.5k,
      .lsatacc = -16384.5lk }
};

/* Function to trigger range checks through conversions */
static int convert_and_check(unsigned _Sat _Fract val) {
    /* This conversion should trigger range checking */
    int int_val = (int)val;
    float float_val = (float)val;
    
    /* Use both results to prevent optimization */
    return int_val + (int)(float_val * 1000);
}

int main(void) {
    volatile int result = 0; /* volatile to prevent dead code elimination */
    
    /* Test 1: Direct boundary value initialization */
    const unsigned short _Fract max_ushort_fract = 0.999999ur;
    const signed _Fract min_signed_fract = -1.0r;
    const _Accum max_accum = 255.999999k;
    const long _Accum min_long_accum = -32768.0lk;
    
    /* These initializations should trigger range calculations */
    result += (int)(max_ushort_fract * 1000);
    result += (int)(min_signed_fract * 1000);
    result += (int)max_accum;
    result += (int)min_long_accum;
    
    /* Test 2: Arithmetic operations that may overflow */
    const unsigned _Sat _Fract sat_fract1 = 0.75ur;
    const unsigned _Sat _Fract sat_fract2 = 0.5ur;
    
    /* This addition should trigger saturation logic */
    unsigned _Sat _Fract sum = sat_fract1 + sat_fract2;
    result += (int)(sum * 1000);
    
    /* Test 3: Multiplication at boundaries */
    const signed _Sat _Fract s1 = 0.999999r;
    const signed _Sat _Fract s2 = -0.999999r;
    signed _Sat _Fract product = s1 * s2;
    result += (int)(product * 1000);
    
    /* Test 4: Compile-time conditional with fixed-point */
    #if __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    /* Use fixed-point in conditional compilation */
    const _Accum threshold = 128.5k;
    const _Accum test_val = 256.0k;  /* This exceeds max for _Accum */
    
    /* This comparison should trigger range checking */
    if (EVAL_CONST(test_val > threshold)) {
        result += 1000;
    }
    #endif
    
    /* Test 5: Loop with fixed-point operations */
    for (int i = 0; i < 3; i++) {
        /* Use values from test_data array */
        unsigned short _Fract temp = test_data[i].usf;
        
        /* Operations that might trigger range checks */
        temp = temp * temp;  /* Square the value */
        
        /* Conditional based on fixed-point comparison */
        if (temp > 0.25ur) {
            result += i * 100;
        } else {
            result += i * 10;
        }
        
        /* Convert to integer (triggers range checking) */
        result += (int)(temp * 1000);
    }
    
    /* Test 6: Complex expression with multiple conversions */
    long _Sat _Accum complex_expr = 0.0lk;
    for (int i = 0; i < 2; i++) {
        /* Build up a value that approaches boundaries */
        complex_expr = complex_expr + 16384.0lk;
        
        /* Mix with integer arithmetic */
        complex_expr = complex_expr * (i + 1);
    }
    
    /* Final conversion that should trigger range checks */
    result += (int)complex_expr;
    
    /* Test 7: Shift-like behavior through multiplication */
    const signed _Fract fract_power = 0.5r;
    signed _Fract shifted = fract_power;
    
    /* Simulate shifting by repeated multiplication */
    for (int i = 0; i < 4; i++) {
        shifted = shifted * fract_power;
        result += (int)(shifted * 10000);
    }
    
    /* Test 8: Boundary value comparisons */
    const unsigned _Fract boundary1 = 0.999999ur;
    const unsigned _Fract boundary2 = 0.000001ur;
    
    /* These comparisons should use the uncovered range checking code */
    if (EVAL_CONST(boundary1 > boundary2)) {
        result += 5000;
    }
    
    if (EVAL_CONST(boundary1 == 0.999999ur)) {
        result += 3000;
    }
    
    /* Test 9: Overflow through addition of large values */
    unsigned _Sat _Accum sat_accum1 = 255.999999k;
    unsigned _Sat _Accum sat_accum2 = 0.000001k;
    
    /* This should saturate at the maximum value */
    unsigned _Sat _Accum accum_sum = sat_accum1 + sat_accum2;
    result += (int)accum_sum;
    
    /* Test 10: Underflow through subtraction */
    signed _Sat _Fract signed_sat = -0.999999r;
    signed _Sat _Fract decrement = 0.000001r;
    
    /* This should stay at minimum or near minimum */
    signed _Sat _Fract diff = signed_sat - decrement;
    result += (int)(diff * 10000);
    
    printf("Result: %d\n", result);
    
    /* Use __builtin_constant_p to verify compile-time evaluation */
    if (__builtin_constant_p(max_ushort_fract > 0.5ur)) {
        printf("Compile-time constant evaluation worked\n");
    }
    
    return 0;
}
