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
    
    /* Use in array sizes (C99 VLA or C++ array) */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(9.99)];
    
    /* Use in enum constants */
    enum { 
        E1 = (int)trunc(100.7),
        E2 = (int)ceil(50.1)
    };
    
    /* Compute with volatile to force folding during compilation */
    double v = g_input;
    double r1 = trunc(v);
    double r2 = floor(v + 1.0);
    double r3 = ceil(v - 1.0);
    double r4 = round(v * 2.0);
    
    return (int)(r1 + r2 + r3 + r4 + s1 + s2 + s3 + s4 + E1 + E2);
}

/* Test 2: Nested integer-valued real function calls */
static int test_nested_calls(void) {
    double v = g_input;
    
    /* Nested calls */
    double n1 = floor(ceil(v));
    double n2 = trunc(round(v * 2.0));
    double n3 = nearbyint(rint(v));
    double n4 = __builtin_llround(__builtin_llrint(v));
    
    /* Multiple levels of nesting */
    double n5 = trunc(floor(ceil(round(v))));
    
    /* Nested with arithmetic */
    double n6 = floor(trunc(v) * 2.0 + ceil(v / 2.0));
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6);
}

/* Test 3: Conditional expressions with integer-valued real functions */
static int test_conditional_calls(void) {
    double v = g_input;
    double v2 = g_input2;
    
    /* Conditional operator with integer-valued calls */
    double c1 = (v > 3.0) ? trunc(v) : floor(v2);
    double c2 = (v2 < 3.0) ? ceil(v) : round(v2);
    
    /* Nested conditional with calls */
    double c3 = (v > v2) ? 
                (trunc(v) > floor(v2) ? nearbyint(v) : rint(v2)) :
                ceil(v + v2);
    
    /* Conditional with builtins */
    long long c4 = (v > 0) ? __builtin_llround(v) : __builtin_llrint(v2);
    
    return (int)(c1 + c2 + c3 + c4);
}

/* Test 4: Complex number real/imag part extraction */
static int test_complex_parts(void) {
    /* Complex integer type */
    _Complex int ci = 3 + 4 * I;
    _Complex double cd = 5.5 + 6.6 * I;
    
    /* Extract real and imaginary parts */
    double r1 = __real__ ci;  /* integer-valued real */
    double r2 = __imag__ ci;  /* integer-valued real */
    double r3 = __real__ cd;
    double r4 = __imag__ cd;
    
    /* Use in expressions with other integer-valued functions */
    double r5 = trunc(__real__ ci) + floor(__imag__ cd);
    double r6 = round(__real__ cd) * ceil(__imag__ ci);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6);
}

/* Test 5: Integer-valued calls with different argument counts */
static int test_varying_arguments(void) {
    double v = g_input;
    
    /* Functions with single argument */
    double s1 = trunc(v);
    double s2 = floor(v);
    
    /* Builtins that may have different argument forms */
    double s3 = __builtin_rint(v);
    double s4 = __builtin_round(v);
    
    /* Use in comparison expressions */
    int cmp1 = (trunc(v) == floor(v + 0.5));
    int cmp2 = (ceil(v) > round(v - 0.5));
    
    /* Arithmetic with mixed calls */
    double a1 = (trunc(v) * 2.0) / floor(v + 1.0);
    double a2 = ceil(v) + round(v) - nearbyint(v);
    
    return (int)(s1 + s2 + s3 + s4 + cmp1 + cmp2 + a1 + a2);
}

/* Test 6: Integer-valued calls in loop bounds */
static int test_loop_bounds(void) {
    double v = g_input;
    int sum = 0;
    
    /* Use integer-valued calls in loop bounds */
    int limit1 = (int)trunc(v * 2.0);
    int limit2 = (int)floor(v + 2.0);
    
    for (int i = 0; i < limit1; i++) {
        sum += i;
    }
    
    for (int j = (int)ceil(v); j < limit2; j++) {
        sum += j * 2;
    }
    
    /* While loop with condition using integer-valued call */
    int k = 0;
    while (k < (int)round(v * 3.0)) {
        sum += k * 3;
        k++;
    }
    
    return sum;
}

/* Test 7: Type casting with integer-valued real functions */
static int test_type_casts(void) {
    double v = g_input;
    
    /* Explicit casts of integer-valued real results */
    int i1 = (int)trunc(v);
    long l1 = (long)floor(v * 10.0);
    float f1 = (float)ceil(v);
    double d1 = (double)round(v);
    
    /* Cast in expressions */
    double e1 = (double)((int)trunc(v)) + floor(v);
    float e2 = (float)ceil(v) * (float)round(v);
    
    return (int)(i1 + l1 + f1 + d1 + e1 + e2);
}

/* Test 8: Large and edge case values */
static int test_edge_cases(void) {
    /* Exact integers */
    double e1 = trunc(4.0);
    double e2 = floor(5.0);
    double e3 = ceil(-3.0);
    
    /* Negative values */
    double n1 = trunc(-2.7);
    double n2 = floor(-3.2);
    double n3 = ceil(-4.8);
    double n4 = round(-5.5);
    
    /* Large values */
    double large = 1e10;
    double l1 = trunc(large + 0.7);
    double l2 = floor(large - 0.3);
    double l3 = ceil(large + 0.1);
    
    /* Zero */
    double z1 = trunc(0.0);
    double z2 = floor(0.0);
    double z3 = ceil(-0.0);
    
    return (int)(e1 + e2 + e3 + n1 + n2 + n3 + n4 + l1 + l2 + l3 + z1 + z2 + z3);
}

#ifdef __cplusplus
/* C++ specific tests with constexpr and templates */
constexpr double cpp_trunc(double x) { return trunc(x); }
constexpr double cpp_floor(double x) { return floor(x); }

template<int N>
struct TestTemplate {
    static constexpr int value = (int)trunc(N * 1.5);
};

constexpr int test_cpp_constexpr() {
    constexpr double v1 = cpp_trunc(7.8);
    constexpr double v2 = cpp_floor(9.2);
    constexpr int v3 = TestTemplate<10>::value;
    
    // Use in static_assert
    static_assert((int)cpp_trunc(5.9) == 5, "trunc failed");
    static_assert((int)cpp_floor(5.9) == 5, "floor failed");
    static_assert(TestTemplate<4>::value == 6, "template failed");
    
    return (int)(v1 + v2 + v3);
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests and accumulate checksum */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_complex_parts();
    checksum += test_varying_arguments();
    checksum += test_loop_bounds();
    checksum += test_type_casts();
    checksum += test_edge_cases();
    
#ifdef __cplusplus
    checksum += test_cpp_constexpr();
#endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    static_assert(trunc(5.9) == 5, "compile-time trunc");
    static_assert(floor(5.9) == 5, "compile-time floor");
    static_assert(ceil(5.1) == 6, "compile-time ceil");
    static_assert(round(5.5) == 6, "compile-time round");
    
    return checksum == 0 ? 1 : 0;
}
