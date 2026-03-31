/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force folding in static initializers */
static const double cd = 5.67;
static const float cf = 9.87f;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum TestEnum1 {
    VAL1 = (int)trunc(10.7),           /* Should be 10 */
    VAL2 = (int)floor(10.7),           /* Should be 10 */
    VAL3 = (int)ceil(10.3),            /* Should be 11 */
    VAL4 = (int)round(10.5),           /* Should be 11 */
    VAL5 = (int)nearbyint(10.2),       /* Should be 10 */
    VAL6 = (int)rint(10.8),            /* Should be 11 */
};

/* Test 2: Array sizes using integer-valued real functions */
static char buffer1[(int)trunc(20.9)];                    /* size 20 */
static char buffer2[(int)floor(15.2)];                    /* size 15 */
static char buffer3[(int)ceil(12.1)];                     /* size 13 */

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Complex number real/imag parts */
static double complex z1 = 3.0 + 4.0 * I;
static double complex z2 = -2.5 + 1.5 * I;

/* Test functions for different patterns */

/* Pattern 1: Nested calls */
static int test_nested_calls(void) {
    /* Force folding by using const variables */
    const double x = 7.89;
    const double y = -3.45;
    
    /* Nested integer-valued real calls */
    double r1 = floor(ceil(x));          /* floor(ceil(7.89)) = floor(8) = 8 */
    double r2 = trunc(round(y));         /* trunc(round(-3.45)) = trunc(-3) = -3 */
    double r3 = nearbyint(rint(x));      /* nearbyint(rint(7.89)) = nearbyint(8) = 8 */
    
    /* More complex nesting */
    double r4 = ceil(floor(trunc(10.7))); /* ceil(floor(10)) = ceil(10) = 10 */
    double r5 = round(ceil(floor(9.2)));  /* round(ceil(9)) = round(9) = 9 */
    
    return (int)(r1 + r2 + r3 + r4 + r5); /* 8 + (-3) + 8 + 10 + 9 = 32 */
}

/* Pattern 2: Calls within conditional expressions */
static int test_conditional_calls(void) {
    const double a = 12.34;
    const double b = 56.78;
    const int cond = vi > 0;
    
    /* Conditional operator with integer-valued real calls */
    double r1 = cond ? trunc(a) : floor(b);      /* trunc(12.34) = 12 */
    double r2 = !cond ? ceil(a) : round(b);      /* round(56.78) = 57 */
    
    /* Nested conditional with calls */
    double r3 = (a > b) ? floor(a) : 
                (a < 0) ? ceil(a) : 
                trunc(b);                        /* trunc(56.78) = 56 */
    
    return (int)(r1 + r2 + r3); /* 12 + 57 + 56 = 125 */
}

/* Pattern 3: Built-in functions with different argument counts */
static int test_builtin_calls(void) {
    /* __builtin_llround and __builtin_llrint take 1 argument */
    long long r1 = __builtin_llround(123.456);    /* 123 */
    long long r2 = __builtin_llrint(-456.789);    /* -457 */
    
    /* Complex part extractors */
    double complex z = 3.5 + 2.5 * I;
    double r3 = __real__(z);                      /* 3.5 */
    double r4 = __imag__(z);                      /* 2.5 */
    
    /* Integer complex */
    int complex zi = 7 + 3 * I;
    int r5 = __real__(zi);                        /* 7 */
    int r6 = __imag__(zi);                        /* 3 */
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6); /* 123 + (-457) + 3.5 + 2.5 + 7 + 3 = -318 */
}

/* Pattern 4: Calls as arguments to other calls */
static int test_nested_arguments(void) {
    const double v = 25.67;
    
    /* Calls as arguments */
    double r1 = trunc(round(v));                  /* trunc(26) = 26 */
    double r2 = floor(ceil(v));                   /* floor(26) = 26 */
    double r3 = round(trunc(v));                  /* round(25) = 25 */
    double r4 = ceil(floor(v));                   /* ceil(25) = 25 */
    
    /* Multiple levels */
    double r5 = nearbyint(rint(floor(ceil(v))));  /* nearbyint(rint(floor(26))) = 26 */
    
    return (int)(r1 + r2 + r3 + r4 + r5); /* 26 + 26 + 25 + 25 + 26 = 128 */
}

/* Pattern 5: Mixed with arithmetic operations */
static int test_mixed_arithmetic(void) {
    const double x = 10.5;
    const double y = 3.3;
    
    /* Arithmetic with integer-valued real calls */
    double r1 = (trunc(x) * 2) / floor(y);        /* (10 * 2) / 3 = 20 / 3 = 6.666... */
    double r2 = ceil(x) + floor(y);               /* 11 + 3 = 14 */
    double r3 = round(x) - trunc(y);              /* 11 - 3 = 8 */
    double r4 = nearbyint(x) * rint(y);           /* 11 * 3 = 33 */
    
    /* Comparisons that should fold */
    int cmp1 = ceil(x) > floor(y);                /* 11 > 3 = true (1) */
    int cmp2 = trunc(x) == round(y);              /* 10 == 3 = false (0) */
    
    return (int)(r1 + r2 + r3 + r4) + cmp1 + cmp2; /* 6 + 14 + 8 + 33 + 1 + 0 = 62 */
}

/* Pattern 6: Different argument types and values */
static int test_various_arguments(void) {
    /* Integer arguments */
    double r1 = floor(5);                         /* 5 */
    double r2 = trunc(2);                         /* 2 */
    
    /* Real arguments that are exact integers */
    double r3 = ceil(4.0);                        /* 4 */
    double r4 = round(6.0);                       /* 6 */
    
    /* Real arguments with fractional parts */
    double r5 = floor(4.7);                       /* 4 */
    double r6 = ceil(4.1);                        /* 5 */
    
    /* Negative values */
    double r7 = round(-2.3);                      /* -2 */
    double r8 = trunc(-2.7);                      /* -2 */
    double r9 = floor(-2.7);                      /* -3 */
    double r10 = ceil(-2.1);                      /* -2 */
    
    /* Large values */
    double r11 = trunc(1e10 + 0.7);               /* 10000000000 */
    double r12 = floor(1e10 - 0.3);               /* 9999999999 */
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12);
    /* 5 + 2 + 4 + 6 + 4 + 5 + (-2) + (-2) + (-3) + (-2) + 10000000000 + 9999999999 */
    /* = 19999999996 */
}

/* Pattern 7: Template metaprogramming (C++ only) */
#ifdef __cplusplus
template<typename T>
constexpr T template_floor(T x) {
    return floor(x);
}

template<typename T>
constexpr T template_ceil(T x) {
    return ceil(x);
}

static int test_template_calls(void) {
    constexpr double r1 = template_floor(15.8);   /* 15 */
    constexpr double r2 = template_ceil(15.2);    /* 16 */
    constexpr double r3 = template_floor(-3.7);   /* -4 */
    
    return (int)(r1 + r2 + r3); /* 15 + 16 + (-4) = 27 */
}
#endif

/* Pattern 8: Constant expressions with volatile to prevent early folding */
static int test_volatile_mix(void) {
    /* Use volatile to prevent compile-time evaluation */
    double x = vd;
    float y = vf;
    
    /* These should still be folded by fold-const pass */
    double r1 = trunc(x + 1.0);                   /* trunc(4.14159) = 4 */
    double r2 = floor(y * 2.0);                   /* floor(5.43656) = 5 */
    double r3 = ceil(x - 1.0);                    /* ceil(2.14159) = 3 */
    double r4 = round(y + 1.0);                   /* round(3.71828) = 4 */
    
    return (int)(r1 + r2 + r3 + r4); /* 4 + 5 + 3 + 4 = 16 */
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Add enum values */
    checksum += VAL1 + VAL2 + VAL3 + VAL4 + VAL5 + VAL6;
    
    /* Add array sizes */
    checksum += (int)sizeof(buffer1) + (int)sizeof(buffer2) + (int)sizeof(buffer3);
    
    /* Run test functions */
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_calls();
    checksum += test_nested_arguments();
    checksum += test_mixed_arithmetic();
    checksum += test_various_arguments();
    
#ifdef __cplusplus
    checksum += test_template_calls();
#endif
    
    checksum += test_volatile_mix();
    
    /* Add complex part extractors */
    checksum += (int)__real__(z1) + (int)__imag__(z1);
    checksum += (int)__real__(z2) + (int)__imag__(z2);
    
    printf("Result: %d\n", checksum);
    return 0;
}
