/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature constant folding */
volatile double g_input = 3.14159;
volatile double g_input2 = 2.71828;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_functions(void) {
    /* Use in static initializers */
    static const double t1 = trunc(5.9);
    static const double t2 = floor(5.9);
    static const double t3 = ceil(5.1);
    static const double t4 = round(5.5);
    
    /* Use in array sizes (C99 VLA or C++ array) */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(10.5)];
    
    /* Use in static assertions */
    _Static_assert(trunc(5.9) == 5, "trunc failed");
    _Static_assert(floor(5.9) == 5, "floor failed");
    _Static_assert(ceil(5.1) == 6, "ceil failed");
    _Static_assert(round(5.5) == 6, "round failed");
    
    return (int)(t1 + t2 + t3 + t4);
}

/* Test 2: Nested calls and complex expressions */
static int test_nested_calls(void) {
    /* Nested calls */
    double n1 = floor(ceil(4.7));
    double n2 = trunc(round(3.2));
    double n3 = nearbyint(rint(2.8));
    
    /* Calls as arguments to other calls */
    double n4 = round(trunc(7.3));
    double n5 = ceil(floor(6.9));
    
    /* Mixed arithmetic with integer-valued calls */
    double n6 = (trunc(8.7) * 2) / floor(4.3);
    double n7 = ceil(3.2) + floor(2.8) - round(1.5);
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6 + n7);
}

/* Test 3: Conditional expressions with integer-valued calls */
static int test_conditional_calls(void) {
    volatile double x = g_input;
    volatile double y = g_input2;
    
    /* Conditional operator with integer-valued calls */
    double c1 = (x > 3.0) ? trunc(x) : floor(y);
    double c2 = (y < 3.0) ? ceil(y) : round(x);
    
    /* Nested conditionals */
    double c3 = (x > y) ? trunc(ceil(x)) : floor(round(y));
    
    /* In comparison expressions */
    int cmp1 = (ceil(x) > floor(y)) ? 1 : 0;
    int cmp2 = (trunc(x) == round(y)) ? 1 : 0;
    
    return (int)(c1 + c2 + c3) + cmp1 + cmp2;
}

/* Test 4: Builtin functions with different argument counts */
static int test_builtin_functions(void) {
    /* Builtins that may have different argument handling */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llrint(2.71);
    
    /* Using __real__ and __imag__ on complex types */
    double complex z = 3.0 + 4.0 * I;
    double r1 = __real__(z);
    double i1 = __imag__(z);
    
    /* Complex integer type */
    int complex zi = 5 + 6 * I;
    double r2 = __real__(zi);
    double i2 = __imag__(zi);
    
    return (int)(ll1 + ll2 + r1 + i1 + r2 + i2);
}

/* Test 5: Various argument types and values */
static int test_various_arguments(void) {
    /* Integer arguments */
    double a1 = floor(5);
    double a2 = trunc(2);
    
    /* Exact integer real arguments */
    double a3 = ceil(4.0);
    double a4 = round(6.0);
    
    /* Fractional arguments */
    double a5 = floor(4.7);
    double a6 = trunc(9.3);
    
    /* Negative values */
    double a7 = round(-2.3);
    double a8 = ceil(-3.7);
    double a9 = floor(-3.7);
    
    /* Large values */
    double a10 = trunc(1e10 + 0.7);
    double a11 = floor(1e10 - 0.3);
    
    return (int)(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11);
}

/* Test 6: In loop bounds and control flow */
static int test_loop_bounds(void) {
    int sum = 0;
    
    /* Use integer-valued calls in loop bounds */
    int limit1 = (int)trunc(g_input * 2);
    int limit2 = (int)floor(g_input2 * 3);
    
    for (int i = (int)ceil(1.2); i < limit1; i += (int)round(1.0)) {
        sum += i;
    }
    
    for (int j = (int)floor(0.5); j < limit2; j++) {
        sum += (int)trunc(j * 1.5);
    }
    
    /* In if conditions */
    if (ceil(g_input) > 3) {
        sum += 100;
    }
    
    if (trunc(g_input2) == 2) {
        sum += 200;
    }
    
    return sum;
}

/* C++ specific tests (compile with C++) */
#ifdef __cplusplus
#include <type_traits>

constexpr double cpp_trunc(double x) {
    return trunc(x);
}

constexpr double cpp_floor(double x) {
    return floor(x);
}

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

static int test_cpp_features(void) {
    /* constexpr functions */
    constexpr double ct1 = cpp_trunc(5.9);
    constexpr double ct2 = cpp_floor(5.9);
    
    /* Template arguments */
    constexpr int tv1 = TestTemplate<10>::value;
    
    /* static_assert in C++ */
    static_assert(trunc(5.9) == 5, "C++ trunc failed");
    static_assert(floor(5.9) == 5, "C++ floor failed");
    
    /* Use in constexpr context */
    constexpr double arr_size = ceil(8.3);
    int arr[(int)arr_size];
    
    return (int)(ct1 + ct2 + tv1 + arr_size);
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests and accumulate results */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_functions();
    checksum += test_various_arguments();
    checksum += test_loop_bounds();
    
    #ifdef __cplusplus
    checksum += test_cpp_features();
    #endif
    
    /* Print final checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional verification */
    printf("Verification:\n");
    printf("  trunc(5.9) = %.0f\n", trunc(5.9));
    printf("  floor(5.9) = %.0f\n", floor(5.9));
    printf("  ceil(5.1) = %.0f\n", ceil(5.1));
    printf("  round(5.5) = %.0f\n", round(5.5));
    
    return 0;
}
