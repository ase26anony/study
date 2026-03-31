/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature optimization */
volatile double g_input = 3.14159;
volatile double g_input2 = -2.71828;

/* Test 1: Basic integer-valued real functions in constant contexts */
int test_basic_constants(void) {
    /* Array sizes using integer-valued real functions */
    char buffer1[(int)trunc(10.7)];
    char buffer2[(int)floor(10.7)];
    char buffer3[(int)ceil(10.2)];
    char buffer4[(int)round(10.5)];
    
    /* Static assertions */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    static_assert(round(5.5) == 6, "round failed");
    
    /* Enum values */
    enum {
        VAL_TRUNC = (int)trunc(7.8),
        VAL_FLOOR = (int)floor(7.8),
        VAL_CEIL = (int)ceil(7.2),
        VAL_ROUND = (int)round(7.5)
    };
    
    return VAL_TRUNC + VAL_FLOOR + VAL_CEIL + VAL_ROUND;
}

/* Test 2: Nested integer-valued real function calls */
double test_nested_calls(double x) {
    /* Multiple levels of nesting */
    double r1 = floor(ceil(x));
    double r2 = trunc(round(r1));
    double r3 = nearbyint(rint(r2));
    double r4 = ceil(floor(r3));
    
    /* Deeper nesting */
    double r5 = round(trunc(floor(ceil(x))));
    
    return r1 + r2 + r3 + r4 + r5;
}

/* Test 3: Conditional expressions with integer-valued real functions */
double test_conditional_calls(double a, double b) {
    /* Ternary operator with integer-valued real calls */
    double r1 = (a > b) ? trunc(a) : floor(b);
    double r2 = (a < b) ? ceil(a) : round(b);
    
    /* Nested conditional */
    double r3 = (a == 0) ? nearbyint(b) : 
                (b == 0) ? rint(a) : 
                trunc(a + b);
    
    return r1 + r2 + r3;
}

/* Test 4: Builtin functions with integer arguments */
long long test_builtin_calls(void) {
    /* __builtin_llround and __builtin_llrint */
    long long r1 = __builtin_llround(123.456);
    long long r2 = __builtin_llrint(456.789);
    long long r3 = __builtin_llround(floor(789.123));
    long long r4 = __builtin_llrint(ceil(321.654));
    
    return r1 + r2 + r3 + r4;
}

/* Test 5: Complex number real/imag parts */
double test_complex_parts(void) {
    /* Complex integer type with real/imag extractors */
    _Complex int c1 = 3 + 4 * I;
    _Complex int c2 = -5 + 6 * I;
    
    double r1 = __real__ c1;  /* Should be integer-valued real */
    double r2 = __imag__ c1;  /* Should be integer-valued real */
    double r3 = __real__ c2;
    double r4 = __imag__ c2;
    
    /* Combine with other integer-valued functions */
    double r5 = trunc(__real__ c1) + floor(__imag__ c2);
    
    return r1 + r2 + r3 + r4 + r5;
}

/* Test 6: Mixed expressions and arithmetic */
double test_mixed_expressions(double x, double y) {
    /* Arithmetic with integer-valued real functions */
    double r1 = (trunc(x) * 2.0) / floor(y);
    double r2 = ceil(x) + round(y) - nearbyint(x*y);
    double r3 = rint(x/y) * trunc(x+y);
    
    /* Comparisons that might fold */
    int cmp1 = (ceil(x) > floor(y)) ? 1 : 0;
    int cmp2 = (trunc(x) == round(y)) ? 1 : 0;
    
    return r1 + r2 + r3 + cmp1 + cmp2;
}

/* Test 7: Constexpr functions (C++ style) */
#ifdef __cplusplus
constexpr double constexpr_trunc(double x) {
    return trunc(x);
}

constexpr double constexpr_nested(double x) {
    return floor(ceil(round(x)));
}

int test_constexpr_functions(void) {
    constexpr double v1 = constexpr_trunc(9.87);
    constexpr double v2 = constexpr_nested(6.54);
    
    /* Use in static assertion */
    static_assert(constexpr_trunc(5.5) == 5, "constexpr trunc failed");
    static_assert(constexpr_nested(3.3) == 4, "constexpr nested failed");
    
    return (int)(v1 + v2);
}
#endif

/* Test 8: Template metaprogramming (C++ style) */
#ifdef __cplusplus
template<double (*Func)(double)>
struct MathWrapper {
    static constexpr double apply(double x) {
        return Func(x);
    }
};

int test_template_functions(void) {
    constexpr double v1 = MathWrapper<trunc>::apply(8.9);
    constexpr double v2 = MathWrapper<floor>::apply(8.9);
    constexpr double v3 = MathWrapper<ceil>::apply(8.1);
    
    return (int)(v1 + v2 + v3);
}
#endif

/* Test 9: Zero, one, and two argument calls */
double test_various_arities(void) {
    /* Most standard math functions take 1 argument */
    double r1 = trunc(1.5);
    double r2 = floor(2.5);
    
    /* Some builtins might have different arities */
    /* Note: nearbyint and rint typically take 1 argument, but
       some implementations might have 2-argument versions */
    
    return r1 + r2;
}

/* Test 10: Large and edge case values */
double test_edge_cases(void) {
    /* Exact integers */
    double r1 = trunc(1000.0);
    double r2 = floor(1000.0);
    double r3 = ceil(1000.0);
    
    /* Negative values */
    double r4 = trunc(-1000.7);
    double r5 = floor(-1000.7);
    double r6 = ceil(-1000.2);
    double r7 = round(-1000.5);
    
    /* Large values */
    double r8 = trunc(1e10 + 0.7);
    double r9 = floor(1e10 + 0.7);
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Test 1: Basic constants */
    checksum += test_basic_constants();
    
    /* Test 2: Nested calls */
    double nested_result = test_nested_calls(g_input);
    checksum += (int)nested_result;
    
    /* Test 3: Conditional calls */
    double cond_result = test_conditional_calls(g_input, g_input2);
    checksum += (int)cond_result;
    
    /* Test 4: Builtin calls */
    long long builtin_result = test_builtin_calls();
    checksum += (int)builtin_result;
    
    /* Test 5: Complex parts */
    double complex_result = test_complex_parts();
    checksum += (int)complex_result;
    
    /* Test 6: Mixed expressions */
    double mixed_result = test_mixed_expressions(g_input, g_input2);
    checksum += (int)mixed_result;
    
    /* Test 7: Constexpr functions (C++ only) */
    #ifdef __cplusplus
    checksum += test_constexpr_functions();
    #endif
    
    /* Test 8: Template functions (C++ only) */
    #ifdef __cplusplus
    checksum += test_template_functions();
    #endif
    
    /* Test 9: Various arities */
    double arity_result = test_various_arities();
    checksum += (int)arity_result;
    
    /* Test 10: Edge cases */
    double edge_result = test_edge_cases();
    checksum += (int)edge_result;
    
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    /* These force constant folding during compilation */
    static double static_var = trunc(3.14) + floor(2.72);
    const double const_var = ceil(1.618) * round(1.414);
    
    /* Use in switch case (requires integer constant) */
    switch ((int)trunc(10.5)) {
        case 10:
            checksum += 10;
            break;
        default:
            checksum += 1;
    }
    
    return checksum % 256;
}
