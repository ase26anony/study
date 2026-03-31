/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force constant folding in static initializers */
static double g1 = trunc(4.7);
static double g2 = floor(4.7);
static double g3 = ceil(4.7);
static double g4 = round(4.7);
static double g5 = nearbyint(4.7);
static double g6 = rint(4.7);

/* Complex types for __real__ and __imag__ */
static complex int ci = 3 + 4 * I;
static complex long cl = 5L + 6L * I;

/* Test 1: Basic integer-valued real functions */
int test_basic_functions(void) {
    double d = vd;
    float f = vf;
    
    /* Calls with 1 argument */
    double t1 = trunc(d);
    double t2 = floor(d + 1.0);
    double t3 = ceil(f);
    double t4 = round(d);
    double t5 = nearbyint(d * 2.0);
    double t6 = rint(f * 3.0f);
    
    /* Builtins with long long return */
    long long ll1 = __builtin_llround(d);
    long long ll2 = __builtin_llrint(d);
    
    return (int)(t1 + t2 + t3 + t4 + t5 + t6 + ll1 + ll2) % 256;
}

/* Test 2: Nested calls */
int test_nested_calls(void) {
    double d = vd;
    
    /* Nested integer-valued real calls */
    double n1 = floor(ceil(d));
    double n2 = trunc(round(d * 2.0));
    double n3 = round(trunc(d / 2.0));
    double n4 = ceil(floor(d + 1.5));
    
    /* Multiple levels of nesting */
    double n5 = nearbyint(floor(ceil(rint(d))));
    
    return (int)(n1 + n2 + n3 + n4 + n5) % 256;
}

/* Test 3: Calls in conditional expressions */
int test_conditional_calls(void) {
    double d = vd;
    int cond = vi > 0;
    
    /* Conditional operator with integer-valued real calls */
    double c1 = cond ? trunc(d) : floor(d);
    double c2 = (d > 0) ? ceil(d) : round(-d);
    double c3 = (vi % 2) ? nearbyint(d) : rint(d * 2.0);
    
    /* Nested conditionals */
    double c4 = (cond && d > 0) ? floor(ceil(d)) : trunc(round(d));
    
    return (int)(c1 + c2 + c3 + c4) % 256;
}

/* Test 4: Complex number real/imag parts */
int test_complex_parts(void) {
    /* __real__ and __imag__ on complex integer types */
    int r1 = __real__(ci);
    int i1 = __imag__(ci);
    long r2 = __real__(cl);
    long i2 = __imag__(cl);
    
    /* Combine with other integer-valued calls */
    double cr1 = trunc(__real__(ci) + 0.5);
    double cr2 = floor(__imag__(cl) / 2.0);
    
    return (r1 + i1 + r2 + i2 + (int)cr1 + (int)cr2) % 256;
}

/* Test 5: Calls as function arguments */
int test_call_arguments(void) {
    double d = vd;
    
    /* Integer-valued calls as arguments to other functions */
    double a1 = fabs(trunc(d));
    double a2 = sqrt(floor(d * d));
    double a3 = sin(ceil(d));
    
    /* Multiple arguments */
    double a4 = pow(round(d), trunc(d/2.0));
    
    return (int)(a1 + a2 + a3 + a4) % 256;
}

/* Test 6: Arithmetic expressions with integer-valued calls */
int test_arithmetic_expressions(void) {
    double d = vd;
    
    /* Mixed arithmetic */
    double ar1 = (trunc(d) * 2.0) / floor(d + 1.0);
    double ar2 = ceil(d) + round(d) - nearbyint(d);
    double ar3 = rint(d * 3.0) * trunc(d / 2.0);
    
    /* Comparisons */
    int cmp1 = ceil(d) > floor(d + 1.0);
    int cmp2 = round(d) == trunc(d);
    int cmp3 = nearbyint(d * 2.0) <= rint(d * 3.0);
    
    return (int)(ar1 + ar2 + ar3 + cmp1 + cmp2 + cmp3) % 256;
}

/* Test 7: Type casting */
int test_type_casting(void) {
    double d = vd;
    
    /* Explicit casts */
    int i1 = (int)trunc(d);
    long l1 = (long)floor(d * 2.0);
    float f1 = (float)ceil(d);
    double d1 = (double)round(d);
    
    /* Casts in expressions */
    double cd1 = (double)((int)rint(d));
    int ci1 = (int)(trunc(d) + floor(d));
    
    return (i1 + l1 + (int)f1 + (int)d1 + (int)cd1 + ci1) % 256;
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

constexpr double const_d = 3.14159;

/* Test 8: constexpr functions */
constexpr int test_constexpr_functions() {
    constexpr double d = 3.14159;
    
    /* constexpr integer-valued calls */
    constexpr double ce1 = trunc(d);
    constexpr double ce2 = floor(d + 1.0);
    constexpr double ce3 = ceil(d);
    constexpr double ce4 = round(d);
    
    /* Nested in constexpr */
    constexpr double ce5 = floor(ceil(d));
    
    return static_cast<int>(ce1 + ce2 + ce3 + ce4 + ce5) % 256;
}

/* Test 9: Template arguments */
template<int N>
struct TestTemplate {
    static const int value = N;
};

/* Test 10: Static assertions */
void test_static_asserts() {
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    static_assert(round(5.5) == 6, "round failed");
}

/* Test 11: Array sizes */
void test_array_sizes() {
    char buffer1[(int)floor(10.5)];  // buffer1[10]
    char buffer2[(int)ceil(10.1)];   // buffer2[11]
    char buffer3[(int)trunc(10.9)];  // buffer3[10]
    
    // Use arrays to prevent warnings
    buffer1[0] = 'a';
    buffer2[0] = 'b';
    buffer3[0] = 'c';
}

/* Test 12: Enum values */
enum TestEnum {
    VAL1 = __builtin_llround(3.14),
    VAL2 = __builtin_llrint(2.718),
    VAL3 = (int)trunc(4.7)
};

#endif

/* Test 13: Mixed precision */
int test_mixed_precision(void) {
    double d = vd;
    float f = vf;
    long double ld = 1.23456789L;
    
    /* Mixed precision calls */
    double m1 = trunc(f);           // float -> double
    float m2 = floorf(d);           // double -> float
    long double m3 = ceill(ld);
    double m4 = roundl(ld * 2.0L);  // long double -> double
    
    return (int)(m1 + m2 + m3 + m4) % 256;
}

/* Test 14: Edge cases and special values */
int test_edge_cases(void) {
    /* Exact integers */
    double e1 = trunc(4.0);
    double e2 = floor(4.0);
    double e3 = ceil(4.0);
    double e4 = round(4.0);
    
    /* Negative values */
    double n1 = trunc(-3.7);
    double n2 = floor(-3.7);
    double n3 = ceil(-3.7);
    double n4 = round(-3.7);
    
    /* Large values */
    double l1 = trunc(1e10 + 0.5);
    double l2 = floor(1e10 + 0.5);
    double l3 = ceil(1e10 + 0.5);
    
    /* Zero */
    double z1 = trunc(0.0);
    double z2 = floor(0.0);
    double z3 = ceil(0.0);
    double z4 = round(0.0);
    
    return (int)(e1 + e2 + e3 + e4 + n1 + n2 + n3 + n4 + 
                 l1 + l2 + l3 + z1 + z2 + z3 + z4) % 256;
}

/* Test 15: Loop bounds with integer-valued calls */
int test_loop_bounds(void) {
    double d = vd;
    int sum = 0;
    
    /* Integer-valued calls in loop bounds */
    int limit1 = (int)trunc(d * 2.0);
    int limit2 = (int)floor(d + 2.0);
    
    for (int i = 0; i < limit1; i++) {
        sum += i;
    }
    
    for (int j = (int)ceil(d); j < limit2; j++) {
        sum += j * 2;
    }
    
    return sum % 256;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum ^= test_basic_functions();
    checksum ^= test_nested_calls();
    checksum ^= test_conditional_calls();
    checksum ^= test_complex_parts();
    checksum ^= test_call_arguments();
    checksum ^= test_arithmetic_expressions();
    checksum ^= test_type_casting();
    checksum ^= test_mixed_precision();
    checksum ^= test_edge_cases();
    checksum ^= test_loop_bounds();
    
#ifdef __cplusplus
    /* C++ specific tests */
    checksum ^= test_constexpr_functions();
    test_static_asserts();
    test_array_sizes();
    
    /* Use template */
    TestTemplate<(int)trunc(3.14)> t1;
    TestTemplate<(int)floor(3.14)> t2;
    TestTemplate<(int)ceil(3.14)> t3;
    
    checksum ^= t1.value;
    checksum ^= t2.value;
    checksum ^= t3.value;
    
    /* Use enum */
    checksum ^= VAL1;
    checksum ^= VAL2;
    checksum ^= VAL3;
#endif
    
    printf("Result: %d\n", checksum);
    
    /* Verify some results at runtime */
    if (trunc(4.7) != 4.0) return 1;
    if (floor(4.7) != 4.0) return 1;
    if (ceil(4.7) != 5.0) return 1;
    if (round(4.7) != 5.0) return 1;
    
    return 0;
}
