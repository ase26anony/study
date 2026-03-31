/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force folding in initializers */
static double g1 = trunc(4.7);
static double g2 = floor(4.7);
static double g3 = ceil(4.7);
static double g4 = round(4.7);

/* Complex types for __real__ and __imag__ */
static _Complex int ci = 5 + 3 * _Complex_I;
static _Complex long long cll = 10 + 20 * _Complex_I;

/* Test 1: Basic integer-valued real functions in constant contexts */
int test_basic_folding(void) {
    /* These should be folded by fold-const.cc */
    const double d1 = trunc(vd);
    const double d2 = floor(vd);
    const double d3 = ceil(vd);
    const double d4 = round(vd);
    const double d5 = nearbyint(vd);
    const double d6 = rint(vd);
    
    /* Use in array size (C99 VLA in function scope) */
    int n = (int)trunc(10.5);
    char buffer1[n];
    
    /* Use in switch case through constant propagation */
    int result = 0;
    switch ((int)floor(3.14)) {
        case 3: result += 1; break;
        default: result += 0;
    }
    
    return result + (int)d1 + (int)d2 + (int)d3 + (int)d4;
}

/* Test 2: Nested integer-valued real function calls */
int test_nested_calls(void) {
    /* Nested calls: floor(ceil(x)) */
    const double n1 = floor(ceil(4.3));
    const double n2 = trunc(round(5.8));
    const double n3 = round(trunc(7.2));
    const double n4 = ceil(floor(6.9));
    
    /* Deeper nesting */
    const double n5 = floor(ceil(trunc(3.7)));
    const double n6 = round(nearbyint(floor(8.4)));
    
    /* Mixed with arithmetic */
    const double n7 = trunc(2.5 * floor(3.2));
    const double n8 = ceil(floor(2.1) + round(3.6));
    
    return (int)n1 + (int)n2 + (int)n3 + (int)n4 + (int)n5 + (int)n6;
}

/* Test 3: Conditional expressions with integer-valued calls */
int test_conditional_calls(void) {
    int x = vi;
    double result = 0;
    
    /* Conditional operator with integer-valued calls */
    result = (x > 0) ? trunc(4.8) : floor(4.2);
    result += (x < 100) ? ceil(5.1) : round(5.9);
    
    /* Nested conditional with calls */
    result += (x == 42) ? trunc(floor(6.7)) : ceil(round(6.3));
    
    /* Conditional as argument to another call */
    result = floor((x > 0) ? 3.14 : 2.71);
    
    return (int)result;
}

/* Test 4: Builtins with explicit integer returns */
int test_builtin_calls(void) {
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llround(2.718);
    long long ll3 = __builtin_llrint(4.5);
    long long ll4 = __builtin_llrint(5.5);
    
    /* Use in constant context */
    static long long arr1[(int)__builtin_llround(3.0)] = {0};
    
    /* Complex part extraction */
    int real_part = __real__ ci;
    int imag_part = __imag__ ci;
    long long ll_real = __real__ cll;
    long long ll_imag = __imag__ cll;
    
    return (int)(ll1 + ll2 + ll3 + ll4) + real_part + imag_part;
}

/* Test 5: Integer-valued calls in comparisons */
int test_comparison_contexts(void) {
    int result = 0;
    
    /* Direct comparisons */
    if (trunc(4.7) == 4) result += 1;
    if (floor(4.2) < 5) result += 2;
    if (ceil(3.1) > 3) result += 4;
    if (round(2.5) >= 3) result += 8;
    
    /* Complex comparison expressions */
    if (trunc(floor(5.8)) == 5) result += 16;
    if (ceil(round(3.4)) <= 4) result += 32;
    
    /* Ternary in comparison */
    result += (nearbyint(2.3) == 2) ? 64 : 0;
    
    return result;
}

/* Test 6: Mixed expressions with arithmetic */
int test_mixed_arithmetic(void) {
    /* Arithmetic with integer-valued calls */
    double a = trunc(5.7) * 2.0;
    double b = floor(4.2) + ceil(3.8);
    double c = round(2.5) / trunc(1.5);
    double d = nearbyint(3.14) - rint(2.71);
    
    /* Nested arithmetic */
    double e = floor(trunc(6.4) + 1.5);
    double f = ceil(round(3.2) * 2.0);
    
    /* Type casting */
    int g = (int)trunc(8.9);
    int h = (int)floor(7.1);
    int i = (int)ceil(6.2);
    int j = (int)round(5.5);
    
    return g + h + i + j + (int)a + (int)b;
}

/* Test 7: Edge cases and special values */
int test_edge_cases(void) {
    int result = 0;
    
    /* Exact integers */
    result += (int)trunc(5.0);
    result += (int)floor(4.0);
    result += (int)ceil(3.0);
    result += (int)round(2.0);
    
    /* Negative values */
    result += (int)trunc(-3.7);
    result += (int)floor(-3.7);
    result += (int)ceil(-3.7);
    result += (int)round(-3.7);
    
    /* Zero */
    result += (int)trunc(0.0);
    result += (int)floor(0.0);
    
    /* Large values */
    result += (int)trunc(1e6 + 0.5);
    result += (int)floor(1e6 + 0.5);
    
    return result;
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

constexpr int cpp_test_constexpr() {
    /* constexpr forces compile-time evaluation */
    constexpr double d1 = std::trunc(3.14);
    constexpr double d2 = std::floor(3.14);
    constexpr double d3 = std::ceil(3.14);
    constexpr double d4 = std::round(3.14);
    
    /* Use in static_assert */
    static_assert(std::trunc(5.9) == 5, "");
    static_assert(std::floor(5.9) == 5, "");
    static_assert(std::ceil(5.1) == 6, "");
    static_assert(std::round(5.5) == 6, "");
    
    return (int)d1 + (int)d2 + (int)d3 + (int)d4;
}

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

constexpr int cpp_test_template() {
    return TestTemplate<10>::value + TestTemplate<20>::value;
}

#endif

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_folding();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_calls();
    checksum += test_comparison_contexts();
    checksum += test_mixed_arithmetic();
    checksum += test_edge_cases();
    
    #ifdef __cplusplus
    checksum += cpp_test_constexpr();
    checksum += cpp_test_template();
    #endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    printf("Globals: %f %f %f %f\n", g1, g2, g3, g4);
    
    return checksum != 0 ? 0 : 1;
}
