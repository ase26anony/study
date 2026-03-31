/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Test 1: Basic integer-valued real functions in constant contexts */
static const double test1_const = trunc(4.7);
static const float test1_float = floorf(5.3f);
enum { ENUM_VAL = (int)ceil(9.1) };

/* Test 2: Nested calls */
static double test2_nested(void) {
    return floor(ceil(vd)) + trunc(round(vf));
}

/* Test 3: Calls in conditional expressions */
static double test3_conditional(int flag) {
    return flag ? nearbyint(vd * 2.0) : rint(vf * 3.0);
}

/* Test 4: Builtins with explicit integer return types */
static long long test4_builtins(void) {
    return __builtin_llround(vd) + __builtin_llrint(vf);
}

/* Test 5: Complex number real/imag parts */
static double test5_complex(void) {
    complex int ci = 3 + 4 * I;
    complex double cd = 5.5 + 6.6 * I;
    
    double r1 = __real__ ci;  /* integer-valued real from complex int */
    double r2 = __imag__ cd;  /* real from complex double */
    
    return floor(r1) + trunc(r2);
}

/* Test 6: Multiple arguments and argument count variations */
static double test6_multi_args(void) {
    /* Functions with different argument counts */
    double r1 = trunc(vd);
    double r2 = round(vd);  /* 1 arg */
    
    /* Some builtins might have optional second args in certain contexts */
    return r1 + r2;
}

/* Test 7: In arithmetic expressions */
static double test7_arithmetic(void) {
    return (trunc(vd) * 2.0) / floor(vf + 1.0);
}

/* Test 8: In comparisons */
static int test8_comparisons(void) {
    return ceil(vd) > floor(vf) ? 100 : 200;
}

/* Test 9: Template/constexpr context (C++ only) */
#ifdef __cplusplus
template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

constexpr int test9_constexpr(double x) {
    return (int)round(x * 2.0);
}

static int test9_cpp(void) {
    constexpr int val1 = test9_constexpr(3.14);
    const int val2 = TestTemplate<10>::value;
    return val1 + val2;
}
#else
static int test9_cpp(void) { return 0; }
#endif

/* Test 10: Array sizes and static asserts */
static void test10_array_sizes(void) {
    char buffer1[(int)floor(10.5)];
    char buffer2[(int)ceil(7.2)];
    
    /* Use arrays to prevent optimization */
    buffer1[0] = (char)trunc(vd);
    buffer2[0] = (char)round(vf);
    
#ifdef __cplusplus
    static_assert(trunc(5.9) == 5, "trunc should work at compile time");
    static_assert(floor(5.9) == 5, "floor should work at compile time");
#endif
}

/* Test 11: Mixed types and values */
static double test11_mixed(void) {
    /* Integer arguments */
    double r1 = floor(5);
    double r2 = trunc(2);
    
    /* Exact integer real arguments */
    double r3 = ceil(4.0);
    
    /* Negative values */
    double r4 = round(-2.3);
    double r5 = nearbyint(-3.7);
    
    /* Large values */
    double r6 = floor(1e10 + 0.5);
    double r7 = trunc(1e15 - 0.2);
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7;
}

/* Test 12: Recursive/nested in complex expressions */
static double test12_recursive_expr(void) {
    return trunc(round(floor(ceil(vd * 2.0) / 3.0) * 4.0) - 5.0);
}

/* Main driver that accumulates results */
int main(void) {
    int checksum = 0;
    
    /* Force evaluation in non-constant context first */
    checksum += (int)test1_const;
    checksum += (int)test1_float;
    checksum += ENUM_VAL;
    
    checksum += (int)test2_nested();
    checksum += (int)test3_conditional(vi & 1);
    checksum += (int)test4_builtins();
    checksum += (int)test5_complex();
    checksum += (int)test6_multi_args();
    checksum += (int)test7_arithmetic();
    checksum += test8_comparisons();
    checksum += test9_cpp();
    
    test10_array_sizes();
    
    checksum += (int)test11_mixed();
    checksum += (int)test12_recursive_expr();
    
    /* Additional direct calls in main */
    checksum += (int)trunc(vd * 100.0);
    checksum += (int)floor(vf * 50.0);
    checksum += (int)ceil(vd + vf);
    checksum += (int)round(vd - vf);
    checksum += (int)nearbyint(vd * vf);
    checksum += (int)rint(vf / vd);
    
    printf("Result: %d\n", checksum);
    
    /* Verify some expected values */
    if (trunc(5.9) != 5) return 1;
    if (floor(5.9) != 5) return 1;
    if (ceil(5.1) != 6) return 1;
    if (round(5.5) != 6) return 1;
    
    return 0;
}
