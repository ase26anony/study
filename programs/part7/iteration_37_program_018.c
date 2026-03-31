/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature optimization */
volatile double g_input = 3.14159;
volatile double g_input2 = 2.71828;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_functions(void) {
    /* Use in static initializers */
    static const double s1 = trunc(5.9);
    static const double s2 = floor(4.7);
    static const double s3 = ceil(3.2);
    static const double s4 = round(6.5);
    
    /* Use in array sizes (C99 VLA in function scope) */
    int n1 = (int)trunc(10.5);
    char buf1[n1];
    
    /* Use in enum (compile-time constant) */
    enum { E1 = (int)floor(7.8) };
    
    /* Complex real/imag part extraction */
    complex int ci = 3 + 4 * I;
    double real_part = __real__ ci;
    double imag_part = __imag__ ci;
    
    return (int)(s1 + s2 + s3 + s4 + E1 + real_part + imag_part);
}

/* Test 2: Nested integer-valued real function calls */
static int test_nested_calls(void) {
    double x = g_input;
    
    /* Nested calls */
    double r1 = floor(ceil(x));
    double r2 = trunc(round(x * 2));
    double r3 = nearbyint(rint(x));
    
    /* Multiple nesting levels */
    double r4 = floor(ceil(trunc(round(x))));
    
    /* Builtin variants */
    long long r5 = __builtin_llround(floor(x));
    long long r6 = __builtin_llrint(ceil(x));
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6);
}

/* Test 3: Conditional expressions with integer-valued real functions */
static int test_conditional_calls(void) {
    double a = g_input;
    double b = g_input2;
    
    /* Conditional operator with integer-valued calls */
    double r1 = (a > b) ? trunc(a) : floor(b);
    double r2 = (a < b) ? ceil(a) : round(b);
    
    /* Nested conditional with calls */
    double r3 = (a > 0) ? ((b > 0) ? trunc(a + b) : floor(a - b)) : ceil(b);
    
    /* Conditional as argument to another call */
    double r4 = trunc((a > 2.0) ? floor(a) : ceil(b));
    
    return (int)(r1 + r2 + r3 + r4);
}

/* Test 4: Integer-valued calls in arithmetic expressions */
static int test_arithmetic_expressions(void) {
    double x = g_input;
    
    /* Mixed arithmetic with integer-valued calls */
    double r1 = (trunc(x) * 2.0) / floor(x + 1.0);
    double r2 = ceil(x) + floor(x) - round(x);
    double r3 = nearbyint(x) * rint(x) / trunc(x);
    
    /* Comparisons involving integer-valued calls */
    int cmp1 = (ceil(x) > floor(x + 1.0));
    int cmp2 = (trunc(x) == round(x - 0.5));
    
    /* Type casts after integer-valued calls */
    int i1 = (int)floor(x);
    int i2 = (int)ceil(x * 2);
    
    return (int)(r1 + r2 + r3) + cmp1 + cmp2 + i1 + i2;
}

/* Test 5: Different argument types and values */
static int test_various_arguments(void) {
    /* Integer arguments */
    double r1 = floor(5);
    double r2 = trunc(2);
    
    /* Exact integer real arguments */
    double r3 = ceil(4.0);
    double r4 = round(8.0);
    
    /* Fractional arguments */
    double r5 = floor(4.7);
    double r6 = trunc(9.99);
    
    /* Negative values */
    double r7 = round(-2.3);
    double r8 = ceil(-3.7);
    
    /* Large values */
    double r9 = trunc(1e10 + 0.5);
    double r10 = floor(1e15 - 0.1);
    
    /* Zero and one */
    double r11 = nearbyint(0.0);
    double r12 = rint(1.0);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12);
}

/* Test 6: Complex number operations */
static int test_complex_operations(void) {
    /* Complex integer types */
    complex int ci1 = 5 + 6 * I;
    complex int ci2 = -3 + 4 * I;
    
    /* Real/imag part extraction */
    double r1 = __real__ ci1;
    double r2 = __imag__ ci1;
    double r3 = __real__ ci2;
    double r4 = __imag__ ci2;
    
    /* Combined with other integer-valued calls */
    double r5 = floor(__real__ ci1);
    double r6 = ceil(__imag__ ci2);
    
    /* Arithmetic with extracted parts */
    double r7 = trunc(r1 + r2) + round(r3 - r4);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7);
}

/* Test 7: Calls with different numbers of arguments */
static int test_argument_counts(void) {
    double x = g_input;
    
    /* Standard single-argument calls */
    double r1 = trunc(x);
    double r2 = floor(x);
    
    /* Some builtins might have optional arguments */
    /* Note: nearbyint and rint typically take 1 argument, but check for variants */
    double r3 = nearbyint(x);
    double r4 = rint(x);
    
    /* For functions that could have 2 arguments in some contexts */
    /* Using fmax/fmin which are integer-valued for integer inputs */
    double r5 = __builtin_fmax(floor(x), ceil(x));
    double r6 = __builtin_fmin(trunc(x), round(x));
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6);
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

constexpr int cpp_test_constexpr() {
    /* constexpr evaluation forces compile-time folding */
    constexpr double v1 = std::trunc(5.9);
    constexpr double v2 = std::floor(4.7);
    constexpr double v3 = std::ceil(3.2);
    constexpr double v4 = std::round(6.5);
    
    /* static_assert uses constant folding */
    static_assert(std::trunc(5.9) == 5, "");
    static_assert(std::floor(4.7) == 4, "");
    static_assert(std::ceil(3.2) == 4, "");
    static_assert(std::round(6.5) == 7, "");
    
    /* Template arguments require constant expressions */
    std::integral_constant<int, (int)std::floor(10.5)> ic;
    
    return (int)(v1 + v2 + v3 + v4) + ic.value;
}

template<int N>
struct ArrayTest {
    char data[N];
};

constexpr int cpp_test_templates() {
    /* Use integer-valued calls in template arguments */
    ArrayTest<(int)std::trunc(15.9)> arr1;
    ArrayTest<(int)std::ceil(8.1)> arr2;
    
    return sizeof(arr1) + sizeof(arr2);
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_arithmetic_expressions();
    checksum += test_various_arguments();
    checksum += test_complex_operations();
    checksum += test_argument_counts();
    
    #ifdef __cplusplus
    checksum += cpp_test_constexpr();
    checksum += cpp_test_templates();
    #endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
