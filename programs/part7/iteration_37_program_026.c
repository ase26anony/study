/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Complex types */
volatile _Complex int vci = 3 + 4 * I;
volatile _Complex double vcd = 1.5 + 2.5 * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
static const double test1_const = trunc(5.9);
static const float test1_float = floorf(4.7f);
enum { TEST1_ENUM = (int)ceil(3.2) };

/* Test 2: Nested calls */
static double test2_nested(void) {
    return floor(ceil(vd)) + trunc(round(vf));
}

/* Test 3: Calls with different argument counts */
static double test3_arg_counts(void) {
    /* Builtins with implicit arguments */
    double r1 = __builtin_llround(3.14);
    double r2 = __builtin_llrint(2.71);
    
    /* Complex part extractors (0 or 1 argument forms) */
    double real_part = __real__(vci);
    double imag_part = __imag__(vcd);
    
    return r1 + r2 + real_part + imag_part;
}

/* Test 4: Conditional expressions with integer-valued calls */
static double test4_conditional(void) {
    return (vi > 0) ? trunc(vd) : floor(vf);
}

/* Test 5: Multiple calls in arithmetic expressions */
static double test5_arithmetic(void) {
    return (rint(vd) * 2.0) / nearbyint(vf) + round(vd + vf);
}

/* Test 6: Array sizes using integer-valued calls */
static char buffer6[(int)floor(10.5)];

/* Test 7: Static assertions (compile-time checks) */
static void test7_static_asserts(void) {
    /* These should pass at compile time */
    _Static_assert(trunc(5.9) == 5, "trunc failed");
    _Static_assert(floor(4.7) == 4, "floor failed");
    _Static_assert(ceil(3.2) == 4, "ceil failed");
    _Static_assert(round(2.5) == 3, "round failed");
}

/* Test 8: Template/constexpr in C++ mode */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static const int value = N;
};

constexpr int test8_constexpr(void) {
    return static_cast<int>(trunc(9.99)) + 
           static_cast<int>(floor(8.01)) +
           static_cast<int>(ceil(7.5));
}
#endif

/* Test 9: Mixed types and precision */
static double test9_mixed(void) {
    long double ld = 123.456L;
    double d = 78.9;
    float f = 12.34f;
    
    return truncl(ld) + floor(d) + ceilf(f);
}

/* Test 10: Negative values and edge cases */
static double test10_negatives(void) {
    return round(-2.3) + floor(-3.7) + ceil(-1.2) + trunc(-4.8);
}

/* Test 11: Very large values */
static double test11_large(void) {
    double large = 1e15 + 0.7;
    return nearbyint(large) + rint(large * 0.5);
}

/* Test 12: Calls as function arguments */
static double test12_nested_args(void) {
    return round(trunc(vd)) + floor(ceil(vf));
}

/* Test 13: In loop bounds */
static double test13_loops(void) {
    double sum = 0;
    int limit = (int)floor(vd);
    for (int i = 0; i < limit; i++) {
        sum += ceil(i * 0.5);
    }
    return sum;
}

/* Test 14: Comparison expressions */
static double test14_comparisons(void) {
    if (ceil(vd) > floor(vf)) {
        return trunc(vd);
    }
    return round(vf);
}

/* Test 15: Complex expressions with multiple calls */
static double test15_complex_expr(void) {
    return (__builtin_llround(vd) % 10) + 
           (int)rint(vf) * 2 -
           (long)trunc(vd * 2) / 3;
}

/* Main driver that accumulates results */
int main(void) {
    double checksum = 0.0;
    
    /* Initialize volatile values */
    vd = 3.14159;
    vf = 2.71828f;
    vi = 42;
    
    /* Accumulate test results */
    checksum += test1_const;
    checksum += test1_float;
    checksum += TEST1_ENUM;
    checksum += test2_nested();
    checksum += test3_arg_counts();
    checksum += test4_conditional();
    checksum += test5_arithmetic();
    checksum += sizeof(buffer6);  /* Use array size */
    test7_static_asserts();       /* Compile-time only */
    
    #ifdef __cplusplus
    checksum += TestTemplate<(int)trunc(5.5)>::value;
    checksum += test8_constexpr();
    #endif
    
    checksum += test9_mixed();
    checksum += test10_negatives();
    checksum += test11_large();
    checksum += test12_nested_args();
    checksum += test13_loops();
    checksum += test14_comparisons();
    checksum += test15_complex_expr();
    
    /* Add some direct calls in main */
    checksum += __real__(vci);
    checksum += __imag__(vcd);
    checksum += nearbyint(checksum);
    
    printf("Result: %f\n", checksum);
    printf("Buffer size: %zu\n", sizeof(buffer6));
    
    return (int)trunc(checksum) % 256;
}
