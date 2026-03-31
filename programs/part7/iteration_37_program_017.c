/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Complex types */
volatile double _Complex vc_d = 3.0 + 4.0*I;
volatile int _Complex vc_i = 5 + 6*I;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum {
    ENUM_TRUNC = (int)trunc(5.9),
    ENUM_FLOOR = (int)floor(5.9),
    ENUM_CEIL = (int)ceil(5.1),
    ENUM_ROUND = (int)round(5.5),
    ENUM_NEARBYINT = (int)nearbyint(5.3),
    ENUM_RINT = (int)rint(5.7)
};

/* Test 2: Array sizes using integer-valued functions */
static char buffer1[(int)floor(10.5)];
static char buffer2[(int)ceil(9.1)];
static char buffer3[(int)trunc(8.9)];

/* Test 3: Static assertions */
_Static_assert(trunc(5.9) == 5, "trunc failed");
_Static_assert(floor(5.9) == 5, "floor failed");
_Static_assert(ceil(5.1) == 6, "ceil failed");
_Static_assert(round(5.5) == 6, "round failed");

/* Test 4: Global initializers with integer-valued functions */
static double g1 = trunc(7.8);
static double g2 = floor(7.8);
static double g3 = ceil(7.2);
static double g4 = round(7.5);
static double g5 = nearbyint(7.3);
static double g6 = rint(7.7);

/* Test 5: Nested calls */
static double test_nested(double x) {
    return floor(ceil(trunc(round(x))));
}

/* Test 6: Calls within conditional expressions */
static double test_conditional(double x, double y) {
    return (x > y) ? trunc(x) : floor(y);
}

/* Test 7: Calls as function arguments */
static double test_args(double x) {
    return round(trunc(floor(ceil(x))));
}

/* Test 8: Builtins with explicit integer returns */
static long long test_builtins(double x) {
    return __builtin_llround(x) + __builtin_llrint(x);
}

/* Test 9: Complex number real/imag parts */
static int test_complex_real(double _Complex c) {
    return (int)__real__ c;
}

static int test_complex_imag(double _Complex c) {
    return (int)__imag__ c;
}

/* Test 10: Mixed arithmetic with integer-valued functions */
static double test_mixed_arithmetic(double x, double y) {
    return (trunc(x) * 2.0) / floor(y) + ceil(x + y) - round(x * y);
}

/* Test 11: Integer-valued functions in comparisons */
static int test_comparisons(double a, double b) {
    return (ceil(a) > floor(b)) ? 1 : 0;
}

/* Test 12: Type casts with integer-valued functions */
static int test_type_casts(double d) {
    return (int)rint(d) + (int)nearbyint(d * 2.0);
}

/* Test 13: Recursive/nested in constant expressions */
static constexpr double test_constexpr(double x) {
    return floor(ceil(x));
}

/* Test 14: Template metaprogramming (C++ style) */
#ifdef __cplusplus
template<double (*FUNC)(double)>
struct MathWrapper {
    static constexpr int apply(double x) {
        return (int)FUNC(x);
    }
};

template<typename T>
constexpr T template_test(T x) {
    return floor(ceil(x));
}
#endif

/* Test 15: Various argument types and values */
static void test_argument_variants(void) {
    /* Integer arguments */
    double r1 = floor(5);
    double r2 = trunc(2);
    
    /* Real arguments that are exact integers */
    double r3 = ceil(4.0);
    double r4 = round(8.0);
    
    /* Real arguments with fractional parts */
    double r5 = floor(4.7);
    double r6 = trunc(9.99);
    
    /* Negative values */
    double r7 = round(-2.3);
    double r8 = ceil(-3.7);
    double r9 = floor(-3.7);
    
    /* Large values */
    double r10 = trunc(1e10 + 0.5);
    double r11 = floor(1e10 - 0.5);
    
    /* Use volatile to prevent pre-evaluation */
    double r12 = trunc(vd);
    double r13 = floor(vf);
    double r14 = ceil(vd * 2.0);
}

/* Test 16: Calls with 0, 1, and 2 arguments */
/* Note: Most math functions take exactly 1 argument, but we can create
   scenarios where optional arguments might be involved through macros
   or compiler extensions */
static double test_variable_args(void) {
    /* Single argument calls */
    double a = trunc(3.14);
    double b = floor(2.71);
    
    /* In conditional with different functions */
    double c = (vi > 5) ? ceil(a) : round(b);
    
    return a + b + c;
}

/* Test 17: Loop bounds with integer-valued functions */
static int test_loop_bounds(double start, double end) {
    int count = 0;
    for (double d = ceil(start); d <= floor(end); d += 1.0) {
        count += (int)trunc(d);
    }
    return count;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    /* Test 1: Basic functions */
    checksum += ENUM_TRUNC + ENUM_FLOOR + ENUM_CEIL + ENUM_ROUND;
    
    /* Test 2: Array size verification */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    /* Test 4: Global initializers */
    checksum += (int)g1 + (int)g2 + (int)g3 + (int)g4;
    
    /* Test 5: Nested calls */
    double nested = test_nested(3.7);
    checksum += (int)nested;
    
    /* Test 6: Conditional calls */
    double cond = test_conditional(4.2, 3.8);
    checksum += (int)cond;
    
    /* Test 7: Calls as arguments */
    double args = test_args(5.3);
    checksum += (int)args;
    
    /* Test 8: Builtins */
    long long builtins = test_builtins(6.7);
    checksum += (int)builtins;
    
    /* Test 9: Complex parts */
    checksum += test_complex_real(3.0 + 4.0*I);
    checksum += test_complex_imag(5.0 + 6.0*I);
    
    /* Test 10: Mixed arithmetic */
    double mixed = test_mixed_arithmetic(2.5, 3.5);
    checksum += (int)mixed;
    
    /* Test 11: Comparisons */
    checksum += test_comparisons(4.2, 3.1);
    
    /* Test 12: Type casts */
    checksum += test_type_casts(2.3);
    
    /* Test 13: Constexpr (compile-time evaluation) */
    #ifdef __cplusplus
    constexpr double ce = test_constexpr(3.3);
    checksum += (int)ce;
    #endif
    
    /* Test 15: Argument variants */
    test_argument_variants();
    
    /* Test 16: Variable args */
    double varargs = test_variable_args();
    checksum += (int)varargs;
    
    /* Test 17: Loop bounds */
    checksum += test_loop_bounds(1.2, 5.7);
    
    /* Additional tests with volatile inputs to ensure runtime evaluation */
    checksum += (int)trunc(vd);
    checksum += (int)floor(vf);
    checksum += (int)ceil(vd + 1.0);
    checksum += (int)round(vf * 2.0f);
    checksum += (int)nearbyint(vd - 1.0);
    checksum += (int)rint(vf + 0.5f);
    
    /* Complex with volatile */
    checksum += (int)__real__ vc_d;
    checksum += (int)__imag__ vc_d;
    checksum += (int)__real__ vc_i;
    checksum += (int)__imag__ vc_i;
    
    printf("Result: %d\n", checksum);
    return 0;
}
