/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force constant folding in static initializers */
static double g1 = trunc(4.7);
static double g2 = floor(4.7);
static double g3 = ceil(4.7);
static double g4 = round(4.7);
static double g5 = nearbyint(4.7);
static double g6 = rint(4.7);

/* Complex types for __real__ and __imag__ */
static complex int ci = 3 + 4 * I;
static complex long cl = 5L + 6L * I;

/* Test 1: Basic integer-valued real functions */
int test_basic_functions(void) {
    double d = vd;
    float f = vf;
    
    /* Calls with 1 argument */
    double t1 = trunc(d);
    double t2 = floor(f);
    double t3 = ceil(d + 1.0);
    double t4 = round(f * 2.0f);
    double t5 = nearbyint(d - 1.0);
    double t6 = rint(f + 0.5f);
    
    /* Builtins with long long return */
    long long ll1 = __builtin_llround(d);
    long long ll2 = __builtin_llrint(d);
    
    return (int)(t1 + t2 + t3 + t4 + t5 + t6 + ll1 + ll2) % 100;
}

/* Test 2: Nested calls */
int test_nested_calls(void) {
    double d = vd;
    
    /* Multiple levels of nesting */
    double n1 = floor(ceil(d));
    double n2 = trunc(round(d * 2.0));
    double n3 = nearbyint(rint(d + 0.5));
    double n4 = __builtin_llround(floor(d));
    double n5 = ceil(trunc(d - 0.5));
    
    /* Deep nesting */
    double n6 = floor(ceil(trunc(round(d))));
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6) % 100;
}

/* Test 3: Calls in conditional expressions */
int test_conditional_calls(void) {
    double d = vd;
    int cond = vi > 40;
    
    /* Ternary operator with integer-valued calls */
    double c1 = cond ? trunc(d) : floor(d + 1.0);
    double c2 = (d > 3.0) ? ceil(d) : round(d);
    double c3 = cond ? nearbyint(d) : rint(d * 2.0);
    
    /* Nested conditionals */
    double c4 = (cond && d > 0) ? floor(ceil(d)) : trunc(round(d));
    
    return (int)(c1 + c2 + c3 + c4) % 100;
}

/* Test 4: Complex number real/imag parts */
int test_complex_parts(void) {
    /* __real__ and __imag__ on complex integer types */
    int r1 = __real__(ci);
    int i1 = __imag__(ci);
    long r2 = __real__(cl);
    long i2 = __imag__(cl);
    
    /* Combined with other integer-valued calls */
    double cr1 = trunc(__real__(ci) + 0.5);
    double cr2 = floor(__imag__(cl) / 2.0);
    
    return (r1 + i1 + r2 + i2 + (int)cr1 + (int)cr2) % 100;
}

/* Test 5: Calls as function arguments */
int test_call_arguments(void) {
    double d = vd;
    
    /* Integer-valued calls as arguments to other functions */
    double a1 = fabs(trunc(d));
    double a2 = sqrt(floor(d * d));
    double a3 = pow(ceil(d), 2.0);
    
    /* Multiple arguments */
    double a4 = fmax(trunc(d), floor(d + 1.0));
    double a5 = fmin(ceil(d), round(d * 2.0));
    
    return (int)(a1 + a2 + a3 + a4 + a5) % 100;
}

/* Test 6: Arithmetic expressions with integer-valued calls */
int test_arithmetic_expressions(void) {
    double d = vd;
    
    /* Mixed arithmetic */
    double e1 = (trunc(d) * 2.0) / floor(d + 1.0);
    double e2 = ceil(d) + round(d) - nearbyint(d);
    double e3 = rint(d) * trunc(d) / 2.0;
    
    /* With integer constants */
    double e4 = floor(d) + 5;
    double e5 = round(d) * 3;
    
    return (int)(e1 + e2 + e3 + e4 + e5) % 100;
}

/* Test 7: Comparison expressions */
int test_comparison_expressions(void) {
    double d = vd;
    
    int cmp1 = trunc(d) > floor(d);
    int cmp2 = ceil(d) <= round(d);
    int cmp3 = nearbyint(d) == rint(d);
    int cmp4 = __builtin_llround(d) < 10LL;
    
    /* In conditional contexts */
    double result = 0;
    if (trunc(d) > 3) {
        result = floor(d);
    } else {
        result = ceil(d);
    }
    
    return (cmp1 + cmp2 + cmp3 + cmp4 + (int)result) % 100;
}

/* Test 8: Type casts */
int test_type_casts(void) {
    double d = vd;
    
    int i1 = (int)trunc(d);
    long l1 = (long)floor(d);
    float f1 = (float)ceil(d);
    double d1 = (double)round(d);
    
    /* Multiple casts */
    int i2 = (int)((double)__builtin_llrint(d));
    
    return (i1 + l1 + (int)f1 + (int)d1 + i2) % 100;
}

/* Test 9: Different argument types */
int test_argument_types(void) {
    /* Integer arguments */
    double a1 = trunc(5);
    double a2 = floor(2);
    double a3 = ceil(-3);
    
    /* Exact integer real arguments */
    double a4 = round(4.0);
    double a5 = nearbyint(-2.0);
    
    /* Fractional arguments */
    double a6 = rint(4.7);
    double a7 = trunc(-3.8);
    
    /* Large values */
    double a8 = floor(1e10 + 0.5);
    double a9 = ceil(-1e10 - 0.5);
    
    return (int)(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9) % 100;
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
constexpr double cpp_trunc(double x) { return trunc(x); }
constexpr double cpp_floor(double x) { return floor(x); }

template<int N>
struct ArrayTest {
    char buffer[(int)floor(N * 1.5)];
};

int test_cpp_features(void) {
    /* constexpr functions */
    constexpr double ct1 = cpp_trunc(3.14);
    constexpr double ct2 = cpp_floor(3.99);
    
    /* Template argument */
    ArrayTest<10> arr;
    
    /* static_assert */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    
    return (int)(ct1 + ct2 + sizeof(arr.buffer)) % 100;
}
#endif

/* Main driver */
int main(void) {
    int checksum = 0;
    
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_complex_parts();
    checksum += test_call_arguments();
    checksum += test_arithmetic_expressions();
    checksum += test_comparison_expressions();
    checksum += test_type_casts();
    checksum += test_argument_types();
    
#ifdef __cplusplus
    checksum += test_cpp_features();
#endif
    
    /* Use results to prevent dead code elimination */
    printf("Global values: %f %f %f %f %f %f\n", g1, g2, g3, g4, g5, g6);
    printf("Complex parts: %d %ld\n", __real__(ci), __real__(cl));
    printf("Final checksum: %d\n", checksum % 100);
    
    return checksum % 100;
}
