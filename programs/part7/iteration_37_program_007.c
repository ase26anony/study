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
    
    /* Use in array sizes (C99 VLA or C++ array) */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(9.99)];
    
    /* Use in enum (integer constant expression) */
    enum { 
        E1 = (int)trunc(100.7),
        E2 = (int)ceil(99.1)
    };
    
    /* Compute with volatile inputs to force runtime evaluation */
    double v1 = trunc(g_input);
    double v2 = floor(g_input);
    double v3 = ceil(g_input);
    double v4 = round(g_input);
    
    return (int)(s1 + s2 + s3 + s4 + v1 + v2 + v3 + v4 + 
                 sizeof(buffer1) + sizeof(buffer2) + E1 + E2);
}

/* Test 2: Nested integer-valued real function calls */
int test_nested_calls(void) {
    /* Simple nesting */
    double n1 = floor(ceil(3.7));
    double n2 = trunc(round(4.2));
    double n3 = round(trunc(5.8));
    
    /* Multiple levels of nesting */
    double n4 = floor(ceil(trunc(6.3)));
    double n5 = trunc(floor(ceil(7.9)));
    
    /* Nesting with arithmetic */
    double n6 = 2.0 * floor(ceil(1.5)) + trunc(round(2.5));
    
    /* Use volatile to prevent compile-time evaluation */
    volatile double v = 8.6;
    double n7 = ceil(floor(v));
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6 + n7);
}

/* Test 3: Conditional expressions with integer-valued real functions */
int test_conditional_calls(void) {
    volatile int flag = 1;
    
    /* Ternary operator with different integer-valued functions */
    double c1 = (flag > 0) ? trunc(10.7) : floor(10.7);
    double c2 = (flag < 0) ? ceil(9.3) : round(9.3);
    
    /* Nested conditional */
    double c3 = (flag == 1) ? 
                ((flag > 0) ? floor(8.9) : ceil(8.9)) : 
                round(8.9);
    
    /* Conditional with volatile */
    volatile double v = 7.5;
    double c4 = (v > 7.0) ? trunc(v) : floor(v);
    
    return (int)(c1 + c2 + c3 + c4);
}

/* Test 4: Builtin functions with explicit integer returns */
int test_builtin_calls(void) {
    /* __builtin_llround and __builtin_llrint return long long */
    long long ll1 = __builtin_llround(123.456);
    long long ll2 = __builtin_llrint(789.012);
    
    /* nearbyint and rint */
    double n1 = nearbyint(45.67);
    double n2 = rint(89.01);
    
    /* Mix with other operations */
    double result = (double)ll1 + (double)ll2 + n1 + n2;
    
    return (int)result;
}

/* Test 5: Complex number real/imag part extraction */
int test_complex_parts(void) {
    /* Complex integer types */
    _Complex int ci = 3 + 4 * I;
    _Complex long cl = 5L + 6L * I;
    
    /* Extract real and imaginary parts */
    double r1 = __real__ ci;
    double i1 = __imag__ ci;
    double r2 = __real__ cl;
    double i2 = __imag__ cl;
    
    /* Use in expressions with other integer-valued functions */
    double result = floor(r1) + ceil(i1) + trunc(r2) + round(i2);
    
    return (int)result;
}

/* Test 6: Multiple arguments and argument counting */
int test_multi_arg_calls(void) {
    /* Some builtins might have multiple arguments */
    /* Use fmax/fmin which take 2 arguments and return integer for integer inputs */
    double m1 = fmax(5.0, 3.0);  /* Should be 5.0 */
    double m2 = fmin(7.0, 9.0);  /* Should be 7.0 */
    
    /* Use copysign which takes 2 arguments */
    double m3 = copysign(10.0, -1.0);  /* Should be -10.0 */
    
    /* Nest multi-arg calls */
    double m4 = fmax(trunc(6.7), floor(6.7));
    double m5 = fmin(ceil(8.2), round(8.2));
    
    return (int)(m1 + m2 + m3 + m4 + m5);
}

/* Test 7: Integer-valued real functions in loop bounds */
int test_loop_bounds(void) {
    int sum = 0;
    
    /* Use integer-valued function as loop bound */
    int bound = (int)floor(15.3);
    for (int i = 0; i < bound; i++) {
        sum += i;
    }
    
    /* Nested loop with different bound calculation */
    int bound2 = (int)ceil(5.7);
    for (int i = 0; i < bound2; i++) {
        for (int j = 0; j < (int)trunc(3.8); j++) {
            sum += i * j;
        }
    }
    
    return sum;
}

/* Test 8: Type casting and conversions */
int test_type_conversions(void) {
    /* Explicit casts of integer-valued real functions */
    int i1 = (int)trunc(20.9);
    long l1 = (long)floor(30.4);
    float f1 = (float)ceil(40.1);
    double d1 = (double)round(50.6);
    
    /* Cast in expressions */
    double result = (double)((int)trunc(25.7)) + 
                    (float)((long)floor(35.2)) +
                    f1 + d1;
    
    return (int)result + i1 + (int)l1;
}

/* Test 9: Large and edge case values */
int test_edge_cases(void) {
    /* Exact integers */
    double e1 = trunc(100.0);
    double e2 = floor(200.0);
    double e3 = ceil(300.0);
    
    /* Negative values */
    double e4 = trunc(-45.6);
    double e5 = floor(-45.6);
    double e6 = ceil(-45.6);
    double e7 = round(-45.6);
    
    /* Large values */
    double e8 = trunc(1e10 + 0.7);
    double e9 = floor(1e10 + 0.3);
    
    /* Zero */
    double e10 = trunc(0.0);
    double e11 = floor(-0.0);
    
    return (int)(e1 + e2 + e3 + e4 + e5 + e6 + e7 + e8 + e9 + e10 + e11);
}

/* Test 10: Mixed expressions with arithmetic */
int test_mixed_expressions(void) {
    volatile double a = 12.34;
    volatile double b = 56.78;
    
    /* Arithmetic with integer-valued functions */
    double m1 = trunc(a) * 2.0 + floor(b) / 3.0;
    double m2 = (ceil(a) - floor(b)) * round(a + b);
    double m3 = trunc(a + b) - ceil(a - b);
    
    /* Comparison expressions */
    int cmp1 = (trunc(a) > floor(b)) ? 1 : 0;
    int cmp2 = (ceil(a) == round(b)) ? 1 : 0;
    
    /* Combined */
    double result = m1 + m2 + m3 + cmp1 + cmp2;
    
    return (int)result;
}

/* C++ specific tests (compile with C++) */
#ifdef __cplusplus
#include <type_traits>

constexpr int cpp_test_constexpr() {
    /* constexpr evaluation forces compile-time folding */
    constexpr double c1 = std::trunc(3.14);
    constexpr double c2 = std::floor(2.71);
    constexpr double c3 = std::ceil(1.41);
    constexpr double c4 = std::round(4.65);
    
    /* static_assert uses constant folding */
    static_assert(std::trunc(5.9) == 5, "trunc failed");
    static_assert(std::floor(4.7) == 4, "floor failed");
    static_assert(std::ceil(3.2) == 4, "ceil failed");
    static_assert(std::round(6.5) == 7, "round failed");
    
    return (int)(c1 + c2 + c3 + c4);
}

template<int N>
struct TestTemplate {
    static const int value = (int)std::trunc(N * 1.5);
};

constexpr int cpp_test_template() {
    return TestTemplate<10>::value + TestTemplate<20>::value;
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_calls();
    checksum += test_complex_parts();
    checksum += test_multi_arg_calls();
    checksum += test_loop_bounds();
    checksum += test_type_conversions();
    checksum += test_edge_cases();
    checksum += test_mixed_expressions();
    
#ifdef __cplusplus
    checksum += cpp_test_constexpr();
    checksum += cpp_test_template();
#endif
    
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests that don't affect runtime */
    static_assert(trunc(5.9) == 5, "Compile-time trunc test");
    static_assert(floor(4.7) == 4, "Compile-time floor test");
    static_assert(ceil(3.2) == 4, "Compile-time ceil test");
    static_assert(round(6.5) == 7, "Compile-time round test");
    
    return 0;
}
