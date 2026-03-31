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
    
    /* Use in enum (C) or constexpr (C++) */
    enum { E1 = (int)ceil(7.1), E2 = (int)round(8.6) };
    
    /* Basic calls with different argument types */
    double d1 = trunc(g_input);
    double d2 = floor(g_input + 1.0);
    double d3 = ceil(g_input * 2.0);
    double d4 = round(g_input / 2.0);
    
    return (int)(s1 + s2 + s3 + s4 + d1 + d2 + d3 + d4 + 
                 sizeof(buffer1) + sizeof(buffer2) + E1 + E2);
}

/* Test 2: Nested integer-valued real function calls */
static int test_nested_calls(void) {
    /* Simple nesting */
    double n1 = floor(ceil(g_input));
    double n2 = trunc(round(g_input2));
    double n3 = ceil(trunc(g_input * 2.0));
    double n4 = round(floor(g_input2 / 2.0));
    
    /* Multiple levels of nesting */
    double n5 = floor(ceil(trunc(round(g_input))));
    double n6 = trunc(floor(ceil(round(g_input2))));
    
    /* Nesting with arithmetic */
    double n7 = floor(ceil(g_input) + trunc(g_input2));
    double n8 = round(trunc(g_input * 2.0) / floor(g_input2));
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8);
}

/* Test 3: Conditional expressions with integer-valued real functions */
static int test_conditional_calls(void) {
    int result = 0;
    
    /* Ternary operator with calls */
    double c1 = (g_input > 3.0) ? trunc(g_input) : floor(g_input);
    double c2 = (g_input2 < 3.0) ? ceil(g_input2) : round(g_input2);
    
    /* Nested ternary with calls */
    double c3 = (g_input > 0.0) ? 
                ((g_input2 > 0.0) ? trunc(g_input) : floor(g_input2)) :
                ceil(g_input + g_input2);
    
    /* Conditional with mixed calls */
    double c4 = (trunc(g_input) > floor(g_input2)) ? 
                round(g_input) : nearbyint(g_input2);
    
    result += (int)(c1 + c2 + c3 + c4);
    
    /* Use in if conditions */
    if (trunc(g_input) == 3) result += 10;
    if (ceil(g_input2) == 3) result += 20;
    if (floor(g_input) == 3) result += 30;
    if (round(g_input2) == 3) result += 40;
    
    return result;
}

/* Test 4: Builtin functions with integer-valued real behavior */
static int test_builtin_functions(void) {
    double b1 = __builtin_trunc(g_input);
    double b2 = __builtin_floor(g_input2);
    double b3 = __builtin_ceil(g_input);
    double b4 = __builtin_round(g_input2);
    
    /* Builtins that return long long */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llrint(2.718);
    
    /* Mix builtins with library calls */
    double b5 = __builtin_trunc(__builtin_floor(g_input));
    double b6 = __builtin_round(__builtin_ceil(g_input2));
    
    return (int)(b1 + b2 + b3 + b4 + b5 + b6) + (int)(ll1 + ll2);
}

/* Test 5: Complex number real/imag part extraction */
static int test_complex_parts(void) {
    /* Complex integer types */
    _Complex int ci = 3 + 4 * I;
    _Complex long long cll = 5 + 6 * I;
    
    /* Extract real and imaginary parts */
    double cr1 = __real__(ci);
    double cr2 = __imag__(ci);
    double cr3 = __real__(cll);
    double cr4 = __imag__(cll);
    
    /* Combine with other integer-valued functions */
    double cr5 = trunc(__real__(ci));
    double cr6 = floor(__imag__(cll));
    
    return (int)(cr1 + cr2 + cr3 + cr4 + cr5 + cr6);
}

/* Test 6: Calls with 0, 1, and 2 arguments */
static int test_varying_arguments(void) {
    /* Most math functions take 1 argument */
    double v1 = trunc(5.5);
    double v2 = floor(4.4);
    
    /* Some builtins might have optional arguments */
    /* Note: nearbyint and rint typically take 1 argument */
    double v3 = nearbyint(3.7);
    double v4 = rint(2.3);
    
    /* Create expressions that might be seen as 0 or 2 arg calls */
    /* by using function pointers or macros in real code */
    
    return (int)(v1 + v2 + v3 + v4);
}

/* Test 7: Mixed expressions and arithmetic */
static int test_mixed_expressions(void) {
    /* Arithmetic with integer-valued calls */
    double m1 = (trunc(g_input) * 2.0) / floor(g_input2);
    double m2 = ceil(g_input) + round(g_input2) - trunc(g_input);
    
    /* Comparisons */
    int cmp1 = (ceil(g_input) > floor(g_input2)) ? 1 : 0;
    int cmp2 = (trunc(g_input) == round(g_input2)) ? 1 : 0;
    int cmp3 = (nearbyint(g_input) <= rint(g_input2)) ? 1 : 0;
    
    /* Type casts */
    int cast1 = (int)trunc(g_input);
    int cast2 = (int)floor(g_input2);
    long cast3 = (long)ceil(g_input * 2.0);
    
    /* Combined expression */
    double m3 = (trunc(g_input) + floor(g_input2)) * 
                (ceil(g_input) - round(g_input2)) / 
                (nearbyint(g_input) + 1.0);
    
    return (int)(m1 + m2 + m3) + cmp1 + cmp2 + cmp3 + cast1 + cast2 + (int)cast3;
}

/* Test 8: Negative values and edge cases */
static int test_edge_cases(void) {
    /* Negative values */
    double e1 = trunc(-5.9);
    double e2 = floor(-4.7);
    double e3 = ceil(-3.2);
    double e4 = round(-6.5);
    
    /* Exact integers */
    double e5 = trunc(4.0);
    double e6 = floor(5.0);
    double e7 = ceil(6.0);
    double e8 = round(7.0);
    
    /* Large values */
    double e9 = trunc(1e10 + 0.5);
    double e10 = floor(1e10 - 0.5);
    
    /* Zero */
    double e11 = trunc(0.0);
    double e12 = floor(0.0);
    double e13 = ceil(0.0);
    double e14 = round(0.0);
    
    return (int)(e1 + e2 + e3 + e4 + e5 + e6 + e7 + e8 + 
                 e9 + e10 + e11 + e12 + e13 + e14);
}

/* Test 9: Template and constexpr contexts (C++) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static constexpr int value = (int)trunc(N * 1.5);
};

constexpr double constexpr_floor(double x) {
    return __builtin_floor(x);
}

constexpr double constexpr_ceil(double x) {
    return __builtin_ceil(x);
}

static int test_constexpr_context(void) {
    /* Template arguments */
    constexpr int t1 = TestTemplate<5>::value;
    constexpr int t2 = TestTemplate<10>::value;
    
    /* Constexpr functions */
    constexpr double c1 = constexpr_floor(4.7);
    constexpr double c2 = constexpr_ceil(3.2);
    
    /* Static assert */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(4.7) == 4, "floor failed");
    static_assert(ceil(3.2) == 4, "ceil failed");
    static_assert(round(6.5) == 7, "round failed");
    
    return t1 + t2 + (int)(c1 + c2);
}
#else
static int test_constexpr_context(void) {
    return 42; /* Default value for C */
}
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_functions();
    checksum += test_complex_parts();
    checksum += test_varying_arguments();
    checksum += test_mixed_expressions();
    checksum += test_edge_cases();
    checksum += test_constexpr_context();
    
    printf("Result: %d\n", checksum);
    
    /* Verify some results at runtime */
    if (trunc(5.9) != 5.0) checksum += 1000;
    if (floor(4.7) != 4.0) checksum += 2000;
    if (ceil(3.2) != 4.0) checksum += 3000;
    if (round(6.5) != 7.0) checksum += 4000;
    
    return checksum == 0 ? 0 : 1;
}
