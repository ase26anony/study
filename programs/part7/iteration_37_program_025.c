/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Complex types */
volatile double _Complex vcd = 3.0 + 4.0 * I;
volatile float _Complex vcf = 1.5f + 2.5f * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_constants(void) {
    /* These should be folded by fold-const.cc */
    const double d1 = trunc(5.9);
    const double d2 = floor(4.7);
    const double d3 = ceil(3.2);
    const double d4 = round(6.5);
    const double d5 = nearbyint(2.3);
    const double d6 = rint(7.8);
    
    /* Use in array size (C99 VLA or C++ array) */
    char buf1[(int)trunc(10.5)];
    char buf2[(int)floor(9.9)];
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6) + sizeof(buf1) + sizeof(buf2);
}

/* Test 2: Nested calls to trigger recursive integer_valued_real_p */
static int test_nested_calls(void) {
    /* Nest calls to exercise depth parameter */
    double r1 = floor(ceil(3.7));      /* ceil(3.7)=4.0, floor(4.0)=4.0 */
    double r2 = trunc(round(2.6));     /* round(2.6)=3.0, trunc(3.0)=3.0 */
    double r3 = nearbyint(rint(5.1));  /* rint(5.1)=5.0, nearbyint(5.0)=5.0 */
    
    /* Deeper nesting */
    double r4 = floor(trunc(ceil(6.3)));  /* ceil(6.3)=7.0, trunc(7.0)=7.0, floor(7.0)=7.0 */
    
    return (int)(r1 + r2 + r3 + r4);
}

/* Test 3: Conditional expressions with integer-valued calls */
static int test_conditional_calls(void) {
    /* Use volatile to prevent front-end folding */
    double x = vd;
    double y = vf;
    
    /* Conditional operator with integer-valued calls */
    double r1 = (x > 3.0) ? trunc(x) : floor(y);
    double r2 = (y < 2.0) ? ceil(y) : round(x);
    
    /* Nested conditional with calls */
    double r3 = (x > y) ? floor(x) : ((x < 0) ? ceil(x) : trunc(y));
    
    return (int)(r1 + r2 + r3);
}

/* Test 4: Builtin functions with different argument counts */
static int test_builtin_calls(void) {
    /* __builtin_llround and __builtin_llrint take 1 argument */
    long long r1 = __builtin_llround(3.14);
    long long r2 = __builtin_llrint(2.71);
    
    /* Some builtins might have optional arguments - test with 0,1,2 args */
    double r3 = __builtin_trunc(vd);
    double r4 = __builtin_floor(vf);
    
    return (int)(r1 + r2 + r3 + r4);
}

/* Test 5: Complex number real/imag parts */
static int test_complex_parts(void) {
    /* __real__ and __imag__ on complex integer types */
    double _Complex cd = 3.5 + 4.5 * I;
    float _Complex cf = 1.2f + 3.4f * I;
    
    /* These extract real/imag parts as integer-valued real expressions */
    double r1 = __real__ cd;
    double r2 = __imag__ cd;
    float r3 = __real__ cf;
    float r4 = __imag__ cf;
    
    /* Combine with other integer-valued functions */
    double r5 = trunc(__real__ cd);
    double r6 = floor(__imag__ cd);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6);
}

/* Test 6: Calls in arithmetic expressions */
static int test_arithmetic_expressions(void) {
    double x = vd;
    double y = vf;
    
    /* Arithmetic with integer-valued calls */
    double r1 = (trunc(x) * 2.0) / floor(y);
    double r2 = ceil(x) + round(y) - nearbyint(x);
    double r3 = rint(x) * trunc(y);
    
    /* Comparisons that might be folded */
    int cmp1 = (ceil(x) > floor(y)) ? 1 : 0;
    int cmp2 = (trunc(x) == round(y)) ? 1 : 0;
    
    return (int)(r1 + r2 + r3) + cmp1 + cmp2;
}

/* Test 7: Type casts and conversions */
static int test_type_conversions(void) {
    double x = vd;
    float y = vf;
    
    /* Explicit casts of integer-valued calls */
    int i1 = (int)trunc(x);
    int i2 = (int)floor(y);
    long l1 = (long)ceil(x);
    long l2 = (long)round(y);
    
    /* Cast to different floating types */
    float f1 = (float)trunc(x);
    double d1 = (double)floor(y);
    
    return i1 + i2 + (int)(l1 + l2) + (int)(f1 + d1);
}

/* Test 8: Large and edge case values */
static int test_edge_cases(void) {
    /* Exact integers */
    double r1 = floor(4.0);
    double r2 = ceil(-3.0);
    double r3 = trunc(0.0);
    
    /* Negative values */
    double r4 = floor(-2.7);   /* -3.0 */
    double r5 = ceil(-2.7);    /* -2.0 */
    double r6 = round(-1.5);   /* -2.0 */
    
    /* Large values */
    double r7 = trunc(1e10 + 0.5);
    double r8 = floor(1e10 - 0.5);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8);
}

/* Test 9: In template contexts (C++ only) */
#ifdef __cplusplus
template<typename T>
constexpr T template_test(T x) {
    return floor(ceil(x)) + trunc(round(x));
}

static int test_template_context(void) {
    constexpr double r1 = template_test(3.7);
    constexpr float r2 = template_test(2.3f);
    return (int)(r1 + r2);
}
#else
static int test_template_context(void) {
    return 0;
}
#endif

/* Test 10: Static assertions and compile-time checks */
static int test_static_asserts(void) {
    /* These force compile-time evaluation */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(4.7) == 4, "floor failed");
    static_assert(ceil(3.2) == 4, "ceil failed");
    static_assert(round(6.5) == 7, "round failed");
    
    /* Use in enum */
    enum { 
        VAL1 = (int)trunc(10.5),
        VAL2 = (int)floor(9.9),
        VAL3 = (int)ceil(8.1)
    };
    
    return VAL1 + VAL2 + VAL3;
}

/* Test 11: Mixed expressions with volatile arguments */
static int test_volatile_mixed(void) {
    /* Use volatile to ensure folding happens in middle-end, not front-end */
    volatile double v1 = 3.7;
    volatile double v2 = -2.3;
    
    double r1 = trunc(v1) + floor(v2);
    double r2 = ceil(v1) * round(v2);
    double r3 = nearbyint(v1) - rint(v2);
    
    /* Nested with volatile */
    double r4 = floor(trunc(v1 + 0.5));
    double r5 = round(ceil(v2 * 2.0));
    
    return (int)(r1 + r2 + r3 + r4 + r5);
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    checksum += test_basic_constants();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_calls();
    checksum += test_complex_parts();
    checksum += test_arithmetic_expressions();
    checksum += test_type_conversions();
    checksum += test_edge_cases();
    checksum += test_template_context();
    checksum += test_static_asserts();
    checksum += test_volatile_mixed();
    
    printf("Result: %d\n", checksum);
    
    /* Verify some results at runtime */
    if (trunc(5.9) != 5.0) {
        printf("Error: trunc(5.9) != 5.0\n");
        return 1;
    }
    if (floor(4.7) != 4.0) {
        printf("Error: floor(4.7) != 4.0\n");
        return 1;
    }
    if (ceil(3.2) != 4.0) {
        printf("Error: ceil(3.2) != 4.0\n");
        return 1;
    }
    
    return 0;
}
