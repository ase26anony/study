/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature constant folding */
volatile double g_input = 3.14159;
volatile double g_input2 = 2.71828;

/* Test 1: Basic integer-valued real functions in constant contexts */
int test_basic_functions(void) {
    /* Use in static initializers */
    static const double s1 = trunc(5.9);
    static const double s2 = floor(4.7);
    static const double s3 = ceil(3.2);
    static const double s4 = round(6.5);
    
    /* Use in array sizes */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(9.99)];
    
    /* Use in static asserts */
    _Static_assert(trunc(5.9) == 5, "trunc failed");
    _Static_assert(floor(4.7) == 4, "floor failed");
    _Static_assert(ceil(3.2) == 4, "ceil failed");
    _Static_assert(round(6.5) == 7, "round failed");
    
    return (int)(s1 + s2 + s3 + s4);
}

/* Test 2: Nested calls to integer-valued real functions */
int test_nested_calls(void) {
    double x = g_input;
    
    /* Nested calls */
    double r1 = floor(ceil(x));
    double r2 = trunc(round(x * 2));
    double r3 = nearbyint(rint(x));
    double r4 = __builtin_llround(__builtin_llrint(x));
    
    /* Multiple levels of nesting */
    double r5 = trunc(floor(ceil(round(x))));
    
    return (int)(r1 + r2 + r3 + r4 + r5);
}

/* Test 3: Calls within conditional expressions */
int test_conditional_calls(void) {
    double a = g_input;
    double b = g_input2;
    
    /* Conditional operator with integer-valued calls */
    double r1 = (a > b) ? trunc(a) : floor(b);
    double r2 = (a < b) ? ceil(a) : round(b);
    
    /* Nested conditionals */
    double r3 = (trunc(a) > 3) ? nearbyint(a * 2) : rint(b * 2);
    
    /* Conditional with builtins */
    double r4 = (__builtin_llround(a) % 2) ? __builtin_llrint(a) : __builtin_llrint(b);
    
    return (int)(r1 + r2 + r3 + r4);
}

/* Test 4: Complex number real/imag part extractors */
int test_complex_extractors(void) {
    /* Complex integer types */
    complex int c1 = 3 + 4 * I;
    complex long c2 = 5 + 6 * I;
    
    /* Extract real and imaginary parts */
    double r1 = __real__ c1;
    double r2 = __imag__ c1;
    double r3 = __real__ c2;
    double r4 = __imag__ c2;
    
    /* Combine with other integer-valued functions */
    double r5 = trunc(__real__ c1);
    double r6 = floor(__imag__ c2);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6);
}

/* Test 5: Mixed expressions with arithmetic */
int test_mixed_expressions(void) {
    double x = g_input;
    double y = g_input2;
    
    /* Arithmetic with integer-valued calls */
    double r1 = (trunc(x) * 2) / floor(y);
    double r2 = ceil(x) + round(y) - nearbyint(x + y);
    
    /* Comparisons */
    int cmp1 = (ceil(x) > floor(y)) ? 1 : 0;
    int cmp2 = (trunc(x) == round(y)) ? 1 : 0;
    
    /* Type casts */
    int r3 = (int)rint(x);
    long r4 = (long)__builtin_llround(y);
    
    return (int)(r1 + r2 + r3 + r4 + cmp1 + cmp2);
}

/* Test 6: Calls with different argument counts */
int test_varying_arguments(void) {
    double x = g_input;
    
    /* Functions with 1 argument */
    double r1 = trunc(x);
    double r2 = floor(x);
    
    /* Builtins that might have different argument counts */
    double r3 = __builtin_llround(x);
    double r4 = __builtin_llrint(x);
    
    /* Note: Some math functions have 2-argument versions */
    /* but for integer-valued real functions, we focus on 1-arg versions */
    
    return (int)(r1 + r2 + r3 + r4);
}

/* Test 7: Different argument types and values */
int test_various_arguments(void) {
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
    
    /* Large values */
    double r9 = trunc(1e10 + 0.5);
    double r10 = floor(1e15 - 0.1);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10);
}

/* Test 8: In constant expressions with const variables */
int test_const_variables(void) {
    const double c1 = 7.3;
    const double c2 = -4.8;
    const int c3 = 10;
    
    /* Use const variables as arguments */
    double r1 = trunc(c1);
    double r2 = floor(c2);
    double r3 = ceil(c3);  /* Integer const */
    
    /* Mixed const and non-const */
    double x = g_input;
    double r4 = round(c1 + x);
    
    return (int)(r1 + r2 + r3 + r4);
}

/* C++ specific tests (if compiled as C++) */
#ifdef __cplusplus
#include <type_traits>

constexpr int cpp_test_constexpr() {
    /* constexpr math functions */
    constexpr double r1 = std::trunc(5.9);
    constexpr double r2 = std::floor(4.7);
    constexpr double r3 = std::ceil(3.2);
    constexpr double r4 = std::round(6.5);
    
    /* Nested in constexpr */
    constexpr double r5 = std::trunc(std::floor(8.9));
    
    return static_cast<int>(r1 + r2 + r3 + r4 + r5);
}

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

int test_cpp_features() {
    constexpr int r1 = cpp_test_constexpr();
    constexpr int r2 = TestTemplate<10>::value;
    
    /* static_assert with integer-valued calls */
    static_assert((int)trunc(5.9) == 5, "C++ trunc failed");
    static_assert((int)floor(4.7) == 4, "C++ floor failed");
    
    return r1 + r2;
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_complex_extractors();
    checksum += test_mixed_expressions();
    checksum += test_varying_arguments();
    checksum += test_various_arguments();
    checksum += test_const_variables();
    
#ifdef __cplusplus
    checksum += test_cpp_features();
#endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional verification */
    printf("Basic: %d\n", test_basic_functions());
    printf("Nested: %d\n", test_nested_calls());
    printf("Conditional: %d\n", test_conditional_calls());
    
    return 0;
}
