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
    
    /* Use in array sizes (via integer constant expressions) */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(9.99)];
    
    /* Use in static assertions */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(4.7) == 4, "floor failed");
    static_assert(ceil(3.2) == 4, "ceil failed");
    static_assert(round(6.5) == 7, "round failed");
    
    /* Compute checksum */
    int sum = (int)s1 + (int)s2 + (int)s3 + (int)s4;
    sum += sizeof(buffer1) + sizeof(buffer2);
    
    return sum;
}

/* Test 2: Nested integer-valued real function calls */
int test_nested_calls(void) {
    /* Nested calls */
    double v1 = floor(ceil(3.7));      /* floor(4.0) = 4 */
    double v2 = trunc(round(2.3));     /* trunc(2.0) = 2 */
    double v3 = ceil(floor(5.9));      /* ceil(5.0) = 5 */
    double v4 = round(trunc(7.8));     /* round(7.0) = 7 */
    
    /* More complex nesting */
    double v5 = nearbyint(rint(4.4));  /* nearbyint(4.0) = 4 */
    double v6 = rint(nearbyint(6.6));  /* rint(7.0) = 7 */
    
    /* Triple nesting */
    double v7 = floor(ceil(trunc(8.9))); /* floor(ceil(8.0)) = floor(8.0) = 8 */
    
    int sum = (int)v1 + (int)v2 + (int)v3 + (int)v4;
    sum += (int)v5 + (int)v6 + (int)v7;
    
    return sum;
}

/* Test 3: Integer-valued real functions in conditional expressions */
int test_conditional_calls(void) {
    int x = 10;
    
    /* Conditional operator with integer-valued real calls */
    double v1 = (x > 5) ? trunc(9.9) : floor(9.9);
    double v2 = (x < 5) ? ceil(2.1) : round(2.1);
    
    /* Nested conditional with these calls */
    double v3 = (x == 10) ? 
                (trunc(7.7) > 6 ? floor(8.8) : ceil(8.8)) :
                round(8.8);
    
    /* Conditional as argument to another integer-valued function */
    double v4 = trunc((x > 0) ? 3.3 : 4.4);
    double v5 = floor((x % 2 == 0) ? 5.5 : 6.6);
    
    int sum = (int)v1 + (int)v2 + (int)v3 + (int)v4 + (int)v5;
    return sum;
}

/* Test 4: Builtin functions with different argument counts */
int test_builtin_variants(void) {
    /* Builtins that might have different argument handling */
    long long v1 = __builtin_llround(3.14);
    long long v2 = __builtin_llrint(2.71);
    
    /* Using volatile to prevent compile-time evaluation */
    volatile double d = g_input;
    long long v3 = __builtin_llround(d);
    long long v4 = __builtin_llrint(d * 2);
    
    /* Complex number parts extraction (integer-valued) */
    complex int ci = 3 + 4I;
    int v5 = __real__ ci;  /* Extracts real part - integer valued */
    int v6 = __imag__ ci;  /* Extracts imag part - integer valued */
    
    /* Complex double with integer parts */
    complex double cd = 5.0 + 6.0I;
    double v7 = __real__ cd;  /* Real part extraction */
    double v8 = __imag__ cd;  /* Imag part extraction */
    
    int sum = (int)v1 + (int)v2 + (int)v3 + (int)v4;
    sum += v5 + v6 + (int)v7 + (int)v8;
    
    return sum;
}

/* Test 5: Mixed expressions with arithmetic */
int test_mixed_expressions(void) {
    /* Arithmetic with integer-valued real functions */
    double v1 = trunc(5.9) * 2.0;
    double v2 = floor(4.7) / 2.0;
    double v3 = ceil(3.2) + 1.5;
    double v4 = round(6.5) - 2.0;
    
    /* Comparisons involving these functions */
    int cmp1 = (trunc(5.9) > floor(4.7));
    int cmp2 = (ceil(3.2) < round(6.5));
    int cmp3 = (nearbyint(4.4) == rint(4.0));
    
    /* Type casts */
    int v5 = (int)trunc(9.9);
    int v6 = (int)floor(8.8);
    long v7 = (long)ceil(7.7);
    
    /* Combined arithmetic */
    double v8 = (trunc(10.5) * floor(2.2)) / ceil(3.0);
    
    int sum = (int)v1 + (int)v2 + (int)v3 + (int)v4;
    sum += cmp1 + cmp2 + cmp3 + v5 + v6 + (int)v7 + (int)v8;
    
    return sum;
}

/* Test 6: Template and constexpr contexts (C++ specific) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static constexpr int value = (int)trunc(N * 1.5);
};

constexpr double constexpr_floor(double x) {
    return floor(x);
}

constexpr double constexpr_ceil(double x) {
    return ceil(x);
}

int test_constexpr_context(void) {
    /* Template arguments */
    constexpr int v1 = TestTemplate<5>::value;  /* trunc(7.5) = 7 */
    constexpr int v2 = TestTemplate<3>::value;  /* trunc(4.5) = 4 */
    
    /* Constexpr function calls */
    constexpr double v3 = constexpr_floor(9.9);
    constexpr double v4 = constexpr_ceil(8.1);
    
    /* Constexpr variables */
    constexpr double v5 = trunc(7.7);
    constexpr double v6 = round(6.6);
    
    /* Use in static assertions */
    static_assert(constexpr_floor(5.5) == 5, "");
    static_assert(constexpr_ceil(4.1) == 5, "");
    
    return v1 + v2 + (int)v3 + (int)v4 + (int)v5 + (int)v6;
}
#else
int test_constexpr_context(void) {
    return 42;  /* Default value for C */
}
#endif

/* Test 7: Edge cases and special values */
int test_edge_cases(void) {
    /* Exact integers */
    double v1 = floor(4.0);
    double v2 = ceil(4.0);
    double v3 = trunc(4.0);
    double v4 = round(4.0);
    
    /* Negative values */
    double v5 = floor(-3.7);   /* -4 */
    double v6 = ceil(-3.7);    /* -3 */
    double v7 = trunc(-3.7);   /* -3 */
    double v8 = round(-3.7);   /* -4 */
    
    /* Large values */
    double v9 = trunc(1e10 + 0.5);
    double v10 = floor(1e10 - 0.5);
    
    /* Zero */
    double v11 = trunc(0.0);
    double v12 = round(0.0);
    
    int sum = (int)v1 + (int)v2 + (int)v3 + (int)v4;
    sum += (int)v5 + (int)v6 + (int)v7 + (int)v8;
    sum += (int)v9 + (int)v10 + (int)v11 + (int)v12;
    
    return sum;
}

/* Test 8: Functions as arguments to other functions */
int test_function_arguments(void) {
    /* Integer-valued real functions as arguments */
    double v1 = trunc(floor(9.5));      /* trunc(9.0) = 9 */
    double v2 = floor(trunc(8.6));      /* floor(8.0) = 8 */
    double v3 = ceil(round(7.4));       /* ceil(7.0) = 7 */
    double v4 = round(ceil(6.1));       /* round(7.0) = 7 */
    
    /* Mixed in arithmetic expressions */
    double v5 = trunc(5.5) + floor(4.4);
    double v6 = ceil(3.3) - round(2.2);
    double v7 = nearbyint(1.1) * rint(2.0);
    
    /* In comparisons */
    int cmp1 = (trunc(floor(6.7)) == 6);
    int cmp2 = (ceil(round(5.5)) > 5);
    
    int sum = (int)v1 + (int)v2 + (int)v3 + (int)v4;
    sum += (int)v5 + (int)v6 + (int)v7 + cmp1 + cmp2;
    
    return sum;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_variants();
    checksum += test_mixed_expressions();
    checksum += test_constexpr_context();
    checksum += test_edge_cases();
    checksum += test_function_arguments();
    
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    enum { 
        E1 = (int)trunc(10.5),
        E2 = (int)floor(9.5),
        E3 = (int)ceil(8.5),
        E4 = (int)round(7.5)
    };
    
    /* Use in switch case labels */
    switch (checksum % 4) {
        case (int)trunc(1.0): break;
        case (int)floor(2.0): break;
        case (int)ceil(3.0): break;
        case (int)round(4.0): break;
    }
    
    return 0;
}
