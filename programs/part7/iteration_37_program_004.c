/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force constant folding in static initializers */
static const double cd = 5.67;
static const float cf = 8.91f;
static const int ci = 123;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum {
    ENUM_TRUNC = (int)trunc(10.7),
    ENUM_FLOOR = (int)floor(10.7),
    ENUM_CEIL = (int)ceil(10.3),
    ENUM_ROUND = (int)round(10.5)
};

/* Test 2: Array sizes using integer-valued real functions */
char buffer1[(int)trunc(20.3)];
char buffer2[(int)floor(15.8)];
char buffer3[(int)ceil(15.2)];

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Complex number real/imag parts */
static double test_complex_parts(void) {
    double _Complex z = 3.0 + 4.0 * I;
    double r = __real__ z;  /* Should be integer-valued real */
    double i = __imag__ z;  /* Should be integer-valued real */
    return r + i;
}

/* Test 5: Nested calls */
static double test_nested_calls(double x) {
    /* Nested integer-valued real function calls */
    return floor(ceil(trunc(round(x))));
}

/* Test 6: Calls in conditional expressions */
static double test_conditional_calls(double a, double b, int flag) {
    return flag ? trunc(a) : floor(b);
}

/* Test 7: Calls as function arguments */
static double test_argument_calls(double x) {
    return round(trunc(floor(ceil(x))));
}

/* Test 8: Builtins with different argument counts */
static long long test_builtin_calls(double x) {
    /* __builtin_llround has 1 argument */
    long long r1 = __builtin_llround(x);
    /* __builtin_llrint has 1 argument */
    long long r2 = __builtin_llrint(x);
    return r1 + r2;
}

/* Test 9: Mixed arithmetic with integer-valued real calls */
static double test_mixed_arithmetic(double x, double y) {
    return (trunc(x) * 2.0) / floor(y) + ceil(x + y) - round(x - y);
}

/* Test 10: In conditional context with comparisons */
static int test_comparisons(double a, double b) {
    if (ceil(a) > floor(b))
        return trunc(a + b);
    else
        return round(a - b);
}

/* Test 11: Template (C++ only) for additional folding contexts */
#ifdef __cplusplus
template<typename T>
constexpr T template_test(T x) {
    return floor(ceil(x)) + trunc(round(x));
}

/* Test 12: constexpr functions */
constexpr double constexpr_test(double x) {
    return nearbyint(rint(floor(x)));
}
#endif

/* Test 13: Different argument types */
static double test_various_args(void) {
    double result = 0.0;
    
    /* Integer arguments */
    result += floor(5);
    result += trunc(2);
    
    /* Real arguments that are exact integers */
    result += ceil(4.0);
    result += round(8.0);
    
    /* Real arguments with fractional parts */
    result += floor(4.7);
    result += trunc(-3.8);
    
    /* Negative values */
    result += round(-2.3);
    result += ceil(-5.1);
    
    return result;
}

/* Test 14: Large values */
static double test_large_values(void) {
    double large = 1e15;
    return trunc(large + 0.7) + floor(large - 0.3);
}

/* Test 15: Zero and one argument calls (simulated) */
static double test_arg_counts(void) {
    /* Most math functions take 1 argument, but we can create patterns
       that might trigger different call_expr_nargs paths */
    double x = 10.5;
    
    /* Chain of single-argument calls */
    double r1 = trunc(x);
    double r2 = floor(r1);
    double r3 = ceil(r2);
    
    return r1 + r2 + r3;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    /* Use volatile variables to prevent complete compile-time evaluation */
    double x = vd;
    float y = vf;
    int flag = vi & 1;
    
    /* Test 1: Basic functions */
    checksum += (int)trunc(x);
    checksum += (int)floor(x + 1.0);
    checksum += (int)ceil(x - 1.0);
    checksum += (int)round(x * 2.0);
    checksum += (int)nearbyint(x / 2.0);
    checksum += (int)rint(y * 3.0f);
    
    /* Test 2: Complex parts */
    checksum += (int)test_complex_parts();
    
    /* Test 3: Nested calls */
    checksum += (int)test_nested_calls(x);
    
    /* Test 4: Conditional calls */
    checksum += (int)test_conditional_calls(x, y, flag);
    
    /* Test 5: Argument calls */
    checksum += (int)test_argument_calls(x);
    
    /* Test 6: Builtin calls */
    checksum += (int)test_builtin_calls(x);
    
    /* Test 7: Mixed arithmetic */
    checksum += (int)test_mixed_arithmetic(x, y);
    
    /* Test 8: Comparisons */
    checksum += test_comparisons(x, y);
    
    /* Test 9: Various arguments */
    checksum += (int)test_various_args();
    
    /* Test 10: Large values */
    checksum += (int)test_large_values();
    
    /* Test 11: Argument counts */
    checksum += (int)test_arg_counts();
    
#ifdef __cplusplus
    /* Test 12: Template test */
    checksum += (int)template_test(x);
    
    /* Test 13: constexpr test */
    checksum += (int)constexpr_test(x);
#endif
    
    /* Additional constant context tests */
    static double static_var = trunc(cd) + floor(cf) + ceil((double)ci);
    checksum += (int)static_var;
    
    /* Array access using computed indices */
    int idx1 = (int)trunc(x) % 10;
    int idx2 = (int)floor(y) % 10;
    char local_buf[20];
    local_buf[idx1] = 'a';
    local_buf[idx2] = 'b';
    checksum += local_buf[0];
    
    printf("Result: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
