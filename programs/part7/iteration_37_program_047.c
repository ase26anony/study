/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature optimization */
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
    
    /* Use in enum (C) or constexpr (C++) */
    enum { VAL1 = (int)ceil(7.1), VAL2 = (int)round(8.4) };
    
    /* Basic computations */
    double d1 = trunc(g_input);
    double d2 = floor(g_input + 1.0);
    double d3 = ceil(g_input * 2.0);
    double d4 = round(g_input / 2.0);
    
    return (int)(s1 + s2 + s3 + s4 + d1 + d2 + d3 + d4 + 
                 sizeof(buffer1) + sizeof(buffer2) + VAL1 + VAL2);
}

/* Test 2: Nested integer-valued real function calls */
int test_nested_calls(void) {
    /* Simple nesting */
    double n1 = floor(ceil(3.7));
    double n2 = trunc(round(4.2));
    double n3 = round(trunc(5.8));
    double n4 = ceil(floor(6.3));
    
    /* Multiple levels of nesting */
    double n5 = floor(ceil(trunc(7.6)));
    double n6 = round(floor(ceil(8.9)));
    
    /* Nesting with arithmetic */
    double n7 = trunc(2.0 * floor(3.5));
    double n8 = ceil(floor(4.2) + trunc(1.8));
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8);
}

/* Test 3: Conditional expressions with integer-valued real functions */
int test_conditional_calls(void) {
    volatile int cond = 1;
    double c1 = cond ? trunc(5.9) : floor(4.7);
    double c2 = (cond > 0) ? ceil(3.2) : round(6.5);
    
    /* Nested conditional */
    double c3 = (cond < 2) ? 
                (cond > 0 ? floor(8.1) : ceil(7.3)) : 
                round(9.4);
    
    /* Conditional with function arguments */
    double c4 = trunc(cond ? 10.5 : 11.6);
    double c5 = round((cond + 1) ? 12.7 : 13.8);
    
    return (int)(c1 + c2 + c3 + c4 + c5);
}

/* Test 4: Builtin functions with different argument counts */
int test_builtin_functions(void) {
    /* Builtins that might have different argument counts */
    double b1 = __builtin_llround(3.14);
    double b2 = __builtin_llrint(2.71);
    
    /* Complex number parts (integer-valued when applied to complex integer) */
    double complex z = 3.0 + 4.0 * I;
    double b3 = __real__(z);
    double b4 = __imag__(z);
    
    /* Mix with standard functions */
    double b5 = __builtin_llround(floor(2.8));
    double b6 = trunc(__builtin_llrint(3.3));
    
    return (int)(b1 + b2 + b3 + b4 + b5 + b6);
}

/* Test 5: Integer-valued real functions in arithmetic expressions */
int test_arithmetic_expressions(void) {
    double a1 = (trunc(5.9) * 2.0) / floor(2.5);
    double a2 = ceil(3.2) + floor(4.7) - round(1.5);
    double a3 = trunc(8.4) * floor(2.1) / ceil(3.0);
    
    /* With comparisons */
    int cmp1 = (ceil(4.1) > floor(3.9));
    int cmp2 = (trunc(5.5) == round(5.5));
    int cmp3 = (floor(6.2) <= ceil(6.2));
    
    /* Type casts */
    int cast1 = (int)rint(7.8);
    int cast2 = (int)trunc(8.9);
    long cast3 = (long)floor(9.1);
    
    return (int)(a1 + a2 + a3 + cmp1 + cmp2 + cmp3 + cast1 + cast2 + cast3);
}

/* Test 6: Various argument types and values */
int test_various_arguments(void) {
    /* Integer arguments */
    double v1 = floor(5);
    double v2 = trunc(2);
    
    /* Exact integer real arguments */
    double v3 = ceil(4.0);
    double v4 = round(6.0);
    
    /* Fractional arguments */
    double v5 = floor(4.7);
    double v6 = trunc(3.14159);
    
    /* Negative values */
    double v7 = round(-2.3);
    double v8 = ceil(-3.7);
    double v9 = floor(-4.2);
    double v10 = trunc(-5.8);
    
    /* Large values */
    double v11 = floor(1e10 + 0.7);
    double v12 = ceil(1e10 - 0.3);
    
    /* Zero and near-zero */
    double v13 = round(0.0);
    double v14 = trunc(0.499);
    double v15 = floor(0.501);
    
    return (int)(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                 v11 + v12 + v13 + v14 + v15);
}

/* Test 7: Template and constexpr contexts (C++ compatible) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static constexpr int value = (int)floor(N * 1.5);
};

constexpr double constexpr_floor(double x) {
    return floor(x);
}

constexpr double constexpr_round(double x) {
    return round(x);
}
#endif

int test_template_constexpr(void) {
    int result = 0;
    
#ifdef __cplusplus
    /* Template arguments */
    result += TestTemplate<5>::value;
    result += TestTemplate<10>::value;
    
    /* Constexpr functions */
    constexpr double cf1 = constexpr_floor(8.7);
    constexpr double cf2 = constexpr_round(9.2);
    result += (int)(cf1 + cf2);
    
    /* Static assert */
    static_assert(trunc(5.9) == 5, "trunc test");
    static_assert(floor(4.7) == 4, "floor test");
    static_assert(ceil(3.2) == 4, "ceil test");
    static_assert(round(6.5) == 7, "round test");
#endif
    
    /* C compatible static assertions */
    typedef char static_assert_trunc[(int)(trunc(5.9) == 5) ? 1 : -1];
    typedef char static_assert_floor[(int)(floor(4.7) == 4) ? 1 : -1];
    
    return result;
}

/* Test 8: Complex expressions mixing all patterns */
int test_complex_expressions(void) {
    /* Mix of everything */
    double x = g_input;
    double y = g_input2;
    
    double e1 = trunc(floor(x) + ceil(y));
    double e2 = round(trunc(x * 2.0) / floor(y + 1.0));
    double e3 = (x > y) ? floor(x * 3.0) : ceil(y * 2.0);
    double e4 = __builtin_llround(trunc(x)) + __builtin_llrint(floor(y));
    
    /* Nested conditionals with function calls */
    double e5 = (x < 5.0) ? 
                ((y > 2.0) ? round(x) : trunc(y)) : 
                floor(x + y);
    
    /* Arithmetic chain */
    double e6 = ceil(e1) * floor(e2) - trunc(e3) + round(e4);
    
    /* Comparison chain */
    int cmp = (trunc(e1) == floor(e2)) && 
              (ceil(e3) > round(e4)) || 
              (trunc(e5) != floor(e6));
    
    return (int)(e1 + e2 + e3 + e4 + e5 + e6 + cmp);
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_functions();
    checksum += test_arithmetic_expressions();
    checksum += test_various_arguments();
    checksum += test_template_constexpr();
    checksum += test_complex_expressions();
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional verification */
    printf("Verification:\n");
    printf("  trunc(5.9) = %.0f (expected 5)\n", trunc(5.9));
    printf("  floor(4.7) = %.0f (expected 4)\n", floor(4.7));
    printf("  ceil(3.2) = %.0f (expected 4)\n", ceil(3.2));
    printf("  round(6.5) = %.0f (expected 7)\n", round(6.5));
    
    return checksum != 0 ? 0 : 1;
}
